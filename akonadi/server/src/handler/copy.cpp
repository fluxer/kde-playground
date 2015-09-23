/*
    Copyright (c) 2008 Volker Krause <vkrause@kde.org>

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

#include "copy.h"

#include "connection.h"
#include "handlerhelper.h"
#include "cachecleaner.h"

#include "storage/datastore.h"
#include "storage/itemqueryhelper.h"
#include "storage/itemretriever.h"
#include "storage/selectquerybuilder.h"
#include "storage/transaction.h"
#include "storage/parthelper.h"

#include "libs/imapset_p.h"
#include "imapstreamparser.h"

using namespace Akonadi;
using namespace Akonadi::Server;

bool Copy::copyItem( const PimItem &item, const Collection &target )
{
//  akDebug() << "Copy::copyItem";

  PimItem newItem = item;
  newItem.setId( -1 );
  newItem.setRev( 0 );
  newItem.setDatetime( QDateTime::currentDateTime() );
  newItem.setAtime( QDateTime::currentDateTime() );
  newItem.setRemoteId( QString() );
  newItem.setRemoteRevision( QString() );
  newItem.setCollectionId( target.id() );
  Part::List parts;
  Q_FOREACH ( const Part &part, item.parts() ) {
    Part newPart( part );
    newPart.setData( PartHelper::translateData( newPart.data(), part.external() ) );
    newPart.setPimItemId( -1 );
    parts << newPart;
  }

  DataStore *store = connection()->storageBackend();
  if ( !store->appendPimItem( parts, item.mimeType(), target, QDateTime::currentDateTime(), QString(), QString(), item.gid(), newItem ) ) {
    return false;
  }
  Q_FOREACH ( const Flag &flag, item.flags() ) {
    if ( !newItem.addFlag( flag ) ) {
      return false;
    }
  }
  return true;
}

bool Copy::parseStream()
{
  ImapSet set = m_streamParser->readSequenceSet();
  if ( set.isEmpty() ) {
    return failureResponse( "No items specified" );
  }

  CacheCleanerInhibitor inhibitor;

  ItemRetriever retriever( connection() );
  retriever.setItemSet( set );
  retriever.setRetrieveFullPayload( true );
  if ( !retriever.exec() ) {
    return failureResponse( retriever.lastError() );
  }

  const QByteArray tmp = m_streamParser->readString();
  const Collection targetCollection = HandlerHelper::collectionFromIdOrName( tmp );
  if ( !targetCollection.isValid() ) {
    return failureResponse( "No valid target specified" );
  }
  if ( targetCollection.isVirtual() ) {
    return failureResponse( "Copying items into virtual collections is not allowed" );
  }

  SelectQueryBuilder<PimItem> qb;
  ItemQueryHelper::itemSetToQuery( set, qb );
  if ( !qb.exec() ) {
    return failureResponse( "Unable to retrieve items" );
  }
  PimItem::List items = qb.result();
  qb.query().finish();

  DataStore *store = connection()->storageBackend();
  Transaction transaction( store );

  Q_FOREACH ( const PimItem &item, items ) {
    if ( !copyItem( item, targetCollection ) ) {
      return failureResponse( "Unable to copy item" );
    }
  }

  if ( !transaction.commit() ) {
    return failureResponse( "Cannot commit transaction." );
  }

  return successResponse( "COPY complete" );
}
