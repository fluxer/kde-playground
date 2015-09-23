/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "storagedebugger.h"
#include "storagedebuggeradaptor.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlError>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMetaType>

Akonadi::Server::StorageDebugger *Akonadi::Server::StorageDebugger::mSelf = 0;
QMutex Akonadi::Server::StorageDebugger::mMutex;

using namespace Akonadi::Server;

Q_DECLARE_METATYPE( QList< QList<QVariant> > )

StorageDebugger *StorageDebugger::instance()
{
  mMutex.lock();
  if ( mSelf == 0 ) {
    mSelf = new StorageDebugger();
  }
  mMutex.unlock();

  return mSelf;
}

StorageDebugger::StorageDebugger()
  : mEnabled( false )
  , mSequence( 0 )
{
  qDBusRegisterMetaType<QList< QList<QVariant> > >();
  new StorageDebuggerAdaptor( this );
  QDBusConnection::sessionBus().registerObject( QLatin1String( "/storageDebug" ),
                                                this, QDBusConnection::ExportAdaptors );
}

StorageDebugger::~StorageDebugger()
{
}

void StorageDebugger::enableSQLDebugging( bool enable )
{
  mEnabled = enable;
}

void StorageDebugger::queryExecuted( const QSqlQuery &query, int duration )
{
  const qint64 seq = mSequence.fetchAndAddOrdered(1);

  if ( !mEnabled ) {
    return;
  }

  if ( query.lastError().isValid() ) {
    Q_EMIT queryExecuted( seq, duration, query.executedQuery(), query.boundValues(),
                          0, QList< QList<QVariant> >(), query.lastError().text() );
    return;
  }

  QSqlQuery q( query );
  QList< QVariantList > result;

  if ( q.first() ) {
    const QSqlRecord record = q.record();
    QVariantList row;
    for ( int i = 0; i < record.count(); ++i ) {
      row << record.fieldName( i );
    }
    result << row;

    int cnt = 0;
    do {
      const QSqlRecord record = q.record();
      QVariantList row;
      for ( int i = 0; i < record.count(); ++i ) {
        row << record.value( i );
      }
      result << row;
    } while ( q.next() && ++cnt < 1000 );
  }

  int querySize;
  if ( query.isSelect() ) {
    querySize = query.size();
  } else {
    querySize = query.numRowsAffected();
  }

  Q_EMIT queryExecuted( seq, duration, query.executedQuery(),
                        query.boundValues(), querySize, result, QString() );

  // Reset the query
  q.seek( -1, false );
}
