/*
    Copyright (c) 2007 - 2012 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "dbupdater.h"
#include "dbtype.h"
#include "entities.h"
#include "akonadischema.h"
#include "akdebug.h"
#include "akdbus.h"
#include "querybuilder.h"
#include "selectquerybuilder.h"
#include "datastore.h"
#include "dbconfig.h"
#include "dbintrospector.h"
#include "dbinitializer_p.h"


#include <QCoreApplication>
#include <QMetaMethod>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QThread>

#include <QDomDocument>
#include <QFile>
#include <QTime>
#include <QtSql/qsqlresult.h>

using namespace Akonadi::Server;

DbUpdater::DbUpdater( const QSqlDatabase &database, const QString &filename )
  : m_database( database )
  , m_filename( filename )
{
}

bool DbUpdater::run()
{
  Q_ASSERT( QThread::currentThread() == QCoreApplication::instance()->thread() );

  // TODO error handling
  SchemaVersion currentVersion = SchemaVersion::retrieveAll().first();

  UpdateSet::Map updates;

  if ( !parseUpdateSets( currentVersion.version(), updates ) ) {
    return false;
  }

  if ( updates.isEmpty() ) {
    return true;
  }

  // indicate clients this might take a while
  // we can ignore unregistration in error cases, that'll kill the server anyway
  if ( !QDBusConnection::sessionBus().registerService( AkDBus::serviceName( AkDBus::UpgradeIndicator ) ) ) {
    akFatal() << "Unable to connect to dbus service: " << QDBusConnection::sessionBus().lastError().message();
  }

  // QMap is sorted, so we should be replaying the changes in correct order
  for ( QMap<int, UpdateSet>::ConstIterator it = updates.constBegin(); it != updates.constEnd(); ++it ) {
    Q_ASSERT( it.key() > currentVersion.version() );
    akDebug() << "DbUpdater: update to version:" << it.key() << " mandatory:" << it.value().abortOnFailure;

    bool success = false;
    bool hasTransaction = false;
    if ( it.value().complex ) { // complex update
      const QString methodName = QString::fromLatin1( "complexUpdate_%1()" ).arg( it.value().version );
      const int index = metaObject()->indexOfMethod( methodName.toLatin1().constData() );
      if ( index == -1 ) {
        success = false;
        akError() << "Update to version" << it.value().version << "marked as complex, but no implementation is available";
      } else {
        const QMetaMethod method = metaObject()->method( index );
        method.invoke( this, Q_RETURN_ARG(bool, success) );
        if ( !success ) {
          akError() << "Update failed";
        }
      }
    } else { // regular update
      success = m_database.transaction();
      if ( success ) {
        hasTransaction = true;
        Q_FOREACH ( const QString &statement, it.value().statements ) {
          QSqlQuery query( m_database );
          success = query.exec( statement );
          if ( !success ) {
            akError() << "DBUpdater: query error:" << query.lastError().text() << m_database.lastError().text();
            akError() << "Query was: " << statement;
            akError() << "Target version was: " << it.key();
            akError() << "Mandatory: " << it.value().abortOnFailure;
          }
        }
      }
    }

    if ( success ) {
      currentVersion.setVersion( it.key() );
      success = currentVersion.update();
    }

    if ( !success || ( hasTransaction && !m_database.commit() ) ) {
      akError() << "Failed to commit transaction for database update";
      if ( hasTransaction) {
        m_database.rollback();
      }
      if ( it.value().abortOnFailure ) {
        return false;
      }
    }
  }

  QDBusConnection::sessionBus().unregisterService( AkDBus::serviceName( AkDBus::UpgradeIndicator ) );
  return true;
}

bool DbUpdater::parseUpdateSets( int currentVersion, UpdateSet::Map &updates ) const
{
  QFile file( m_filename );
  if ( !file.open( QIODevice::ReadOnly ) ) {
    akError() << "Unable to open update description file" << m_filename;
    return false;
  }

  QDomDocument document;

  QString errorMsg;
  int line, column;
  if ( !document.setContent( &file, &errorMsg, &line, &column ) ) {
    akError() << "Unable to parse update description file" << m_filename << ":"
              << errorMsg << "at line" << line << "column" << column;
    return false;
  }

  const QDomElement documentElement = document.documentElement();
  if ( documentElement.tagName() != QLatin1String( "updates" ) ) {
    akError() << "Invalid update description file formant";
    return false;
  }

  // iterate over the xml document and extract update information into an UpdateSet
  QDomElement updateElement = documentElement.firstChildElement();
  while ( !updateElement.isNull() ) {
    if ( updateElement.tagName() == QLatin1String( "update" ) ) {
      const int version = updateElement.attribute( QLatin1String( "version" ), QLatin1String( "-1" ) ).toInt();
      if ( version <= 0 ) {
        akError() << "Invalid version attribute in database update description";
        return false;
      }

      if ( updates.contains( version ) ) {
        akError() << "Duplicate version attribute in database update description";
        return false;
      }

      if ( version <= currentVersion ) {
        akDebug() << "skipping update" << version;
      } else {
        UpdateSet updateSet;
        updateSet.version = version;
        updateSet.abortOnFailure = ( updateElement.attribute( QLatin1String( "abortOnFailure" ) ) == QLatin1String( "true" ) );

        QDomElement childElement = updateElement.firstChildElement();
        while ( !childElement.isNull() ) {
          if ( childElement.tagName() == QLatin1String( "raw-sql" ) ) {
            if ( updateApplicable( childElement.attribute( QLatin1String( "backends" ) ) ) ) {
              updateSet.statements << buildRawSqlStatement( childElement );
            }
          } else if ( childElement.tagName() == QLatin1String( "complex-update" ) ) {
            if ( updateApplicable( childElement.attribute( QLatin1String( "backends" ) ) ) ) {
              updateSet.complex = true;
            }
          }
          //TODO: check for generic tags here in the future

          childElement = childElement.nextSiblingElement();
        }

        updates.insert( version, updateSet );
      }
    }
    updateElement = updateElement.nextSiblingElement();
  }

  return true;
}

bool DbUpdater::updateApplicable( const QString &backends ) const
{
  const QStringList matchingBackends = backends.split( QLatin1Char( ',' ) );

  QString currentBackend;
  switch ( DbType::type( m_database ) ) {
  case DbType::MySQL:
    currentBackend = QLatin1String( "mysql" );
    break;
  case DbType::PostgreSQL:
    currentBackend = QLatin1String( "psql" );
    break;
  case DbType::Sqlite:
    currentBackend = QLatin1String( "sqlite" );
    break;
  case DbType::Unknown:
    return false;
  }

  return matchingBackends.contains( currentBackend );
}

QString DbUpdater::buildRawSqlStatement( const QDomElement &element ) const
{
  return element.text().trimmed();
}

bool DbUpdater::complexUpdate_25()
{
  akDebug() << "Starting database update to version 25";

  DbType::Type dbType = DbType::type( DataStore::self()->database() );

  QTime ttotal;
  ttotal.start();

  // Recover from possibly failed or interrupted update
  {
    // We don't care if this fails, it just means that there was no failed update
    QSqlQuery query( DataStore::self()->database() );
    query.exec( QLatin1String( "ALTER TABLE PartTable_old RENAME TO PartTable" ) );
  }

  {
    QSqlQuery query( DataStore::self()->database() );
    query.exec( QLatin1String( "DROP TABLE IF EXISTS PartTable_new" ) );
  }

  {
    // Make sure the table is empty, otherwise we get duplicate key error
    QSqlQuery query( DataStore::self()->database() );
    if ( dbType == DbType::Sqlite ) {
        query.exec( QLatin1String( "DELETE FROM PartTypeTable" ) );
    } else { // MySQL, PostgreSQL
      query.exec( QLatin1String( "TRUNCATE TABLE PartTypeTable" ) );
    }
  }

  {
    // It appears that more users than expected have the invalid "GID" part in their
    // PartTable, which breaks the migration below (see BKO#331867), so we apply this
    // wanna-be fix to remove the invalid part before we start the actual migration.
    QueryBuilder qb( QLatin1String( "PartTable" ), QueryBuilder::Delete );
    qb.addValueCondition( QLatin1String( "PartTable.name" ), Query::Equals, QLatin1String( "GID" ) );
    qb.exec();
  }

  akDebug() << "Creating a PartTable_new";
  {
    TableDescription description;
    description.name = QLatin1String( "PartTable_new" );

    ColumnDescription idColumn;
    idColumn.name = QLatin1String( "id" );
    idColumn.type = QLatin1String( "qint64" );
    idColumn.isAutoIncrement = true;
    idColumn.isPrimaryKey = true;
    description.columns << idColumn;

    ColumnDescription pimItemIdColumn;
    pimItemIdColumn.name = QLatin1String( "pimItemId" );
    pimItemIdColumn.type = QLatin1String( "qint64" );
    pimItemIdColumn.allowNull = false;
    description.columns << pimItemIdColumn;

    ColumnDescription partTypeIdColumn;
    partTypeIdColumn.name = QLatin1String( "partTypeId" );
    partTypeIdColumn.type = QLatin1String( "qint64" );
    partTypeIdColumn.allowNull = false;
    description.columns << partTypeIdColumn;

    ColumnDescription dataColumn;
    dataColumn.name = QLatin1String( "data" );
    dataColumn.type = QLatin1String( "QByteArray" );
    description.columns << dataColumn;

    ColumnDescription dataSizeColumn;
    dataSizeColumn.name = QLatin1String( "datasize" );
    dataSizeColumn.type = QLatin1String( "qint64" );
    dataSizeColumn.allowNull = false;
    description.columns << dataSizeColumn;

    ColumnDescription versionColumn;
    versionColumn.name = QLatin1String( "version" );
    versionColumn.type = QLatin1String( "int" );
    versionColumn.defaultValue = QLatin1String( "0" );
    description.columns << versionColumn;

    ColumnDescription externalColumn;
    externalColumn.name = QLatin1String( "external" );
    externalColumn.type = QLatin1String( "bool" );
    externalColumn.defaultValue = QLatin1String( "false" );
    description.columns << externalColumn;

    DbInitializer::Ptr initializer = DbInitializer::createInstance( DataStore::self()->database() );
    const QString queryString = initializer->buildCreateTableStatement( description );

    QSqlQuery query( DataStore::self()->database() );
    if ( !query.exec( queryString ) ) {
      akError() << query.lastError().text();
      return false;
    }
  }

  akDebug() << "Migrating part types";
  {
    // Get list of all part names
    QueryBuilder qb( QLatin1String( "PartTable" ), QueryBuilder::Select );
    qb.setDistinct( true );
    qb.addColumn( QLatin1String( "PartTable.name" ) );

    if ( !qb.exec() ) {
      akError() << qb.query().lastError().text();
      return false;
    }

    // Process them one by one
    QSqlQuery query = qb.query();
    while ( query.next() ) {
      // Split the part name to namespace and name and insert it to PartTypeTable
      const QString partName = query.value( 0 ).toString();
      const QString ns = partName.left( 3 );
      const QString name = partName.mid( 4 );

      {
        QueryBuilder qb( QLatin1String( "PartTypeTable" ), QueryBuilder::Insert );
        qb.setColumnValue( QLatin1String( "ns" ), ns );
        qb.setColumnValue( QLatin1String( "name" ), name );
        if ( !qb.exec() ) {
          akError() << qb.query().lastError().text();
          return false;
        }
      }
      akDebug() << "\t Moved part type" << partName << "to PartTypeTable";
    }
  }

  akDebug() << "Migrating data from PartTable to PartTable_new";
  {
    QSqlQuery query( DataStore::self()->database() );
    QString queryString;
    if ( dbType == DbType::PostgreSQL ) {
      queryString = QLatin1String( "INSERT INTO PartTable_new (id, pimItemId, partTypeId, data, datasize, version, external) "
                                   "SELECT PartTable.id, PartTable.pimItemId, PartTypeTable.id, PartTable.data, "
                                   "       PartTable.datasize, PartTable.version, PartTable.external "
                                   "FROM PartTable "
                                   "LEFT JOIN PartTypeTable ON "
                                   "          PartTable.name = CONCAT(PartTypeTable.ns, ':', PartTypeTable.name)"  );
    } else if ( dbType == DbType::MySQL ) {
      queryString = QLatin1String( "INSERT INTO PartTable_new (id, pimItemId, partTypeId, data, datasize, version, external) "
                                   "SELECT PartTable.id, PartTable.pimItemId, PartTypeTable.id, PartTable.data, "
                                   "PartTable.datasize, PartTable.version, PartTable.external "
                                   "FROM PartTable "
                                   "LEFT JOIN PartTypeTable ON PartTable.name = CONCAT(PartTypeTable.ns, ':', PartTypeTable.name)" );
    } else if ( dbType == DbType::Sqlite ) {
      queryString = QLatin1String( "INSERT INTO PartTable_new (id, pimItemId, partTypeId, data, datasize, version, external) "
                                   "SELECT PartTable.id, PartTable.pimItemId, PartTypeTable.id, PartTable.data, "
                                   "PartTable.datasize, PartTable.version, PartTable.external "
                                   "FROM PartTable "
                                   "LEFT JOIN PartTypeTable ON PartTable.name = PartTypeTable.ns || ':' || PartTypeTable.name" );
    }

    if ( !query.exec( queryString ) ) {
      akError() << query.lastError().text();
      return false;
    }
  }

  akDebug() << "Swapping PartTable_new for PartTable";
  {
    // Does an atomic swap

    QSqlQuery query( DataStore::self()->database() );

    if ( dbType == DbType::PostgreSQL  || dbType == DbType::Sqlite ) {
      if ( dbType == DbType::PostgreSQL ) {
        DataStore::self()->beginTransaction();
      }

      if ( !query.exec( QLatin1String( "ALTER TABLE PartTable RENAME TO PartTable_old" ) ) ) {
        akError() << query.lastError().text();
        DataStore::self()->rollbackTransaction();
        return false;
      }

      // If this fails in SQLite (i.e. without transaction), we can still recover on next start)
      if ( !query.exec( QLatin1String( "ALTER TABLE PartTable_new RENAME TO PartTable" ) ) ) {
        akError() << query.lastError().text();
        if ( DataStore::self()->inTransaction() ) {
          DataStore::self()->rollbackTransaction();
        }
        return false;
      }

      if ( dbType == DbType::PostgreSQL ) {
        DataStore::self()->commitTransaction();
      }
    } else { // MySQL cannot do rename in transaction, but supports atomic renames
      if ( !query.exec( QLatin1String( "RENAME TABLE PartTable TO PartTable_old,"
                                       "             PartTable_new TO PartTable" ) ) ) {
        akError() << query.lastError().text();
        return false;
      }
    }
  }

  akDebug() << "Removing PartTable_old";
  {
    QSqlQuery query( DataStore::self()->database() );
    if ( !query.exec( QLatin1String( "DROP TABLE PartTable_old;" ) ) ) {
      // It does not matter when this fails, we are successfully migrated
      akDebug() << query.lastError().text();
      akDebug() << "Not a fatal problem, continuing...";
    }
  }

  // Fine tuning for PostgreSQL
  akDebug() << "Final tuning of new PartTable";
  {
    QSqlQuery query( DataStore::self()->database() );
    if ( dbType == DbType::PostgreSQL ) {
      query.exec( QLatin1String( "ALTER TABLE PartTable RENAME CONSTRAINT parttable_new_pkey TO parttable_pkey" ) );
      query.exec( QLatin1String( "ALTER SEQUENCE parttable_new_id_seq RENAME TO parttable_id_seq" ) );
      query.exec( QLatin1String( "SELECT setval('parttable_id_seq', MAX(id) + 1) FROM PartTable") );
    } else if ( dbType == DbType::MySQL ) {
      // 0 will automatically reset AUTO_INCREMENT to SELECT MAX(id) + 1 FROM PartTable
      query.exec( QLatin1String( "ALTER TABLE PartTable AUTO_INCREMENT = 0" ) );
    }
  }

  akDebug() << "Update done in" << ttotal.elapsed() << "ms";

  // Foreign keys and constraints will be reconstructed automatically once
  // all updates are done

  return true;
}
