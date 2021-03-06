/*
    Copyright (c) 2009 Grégory Oestreicher <greg@kamago.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "davgroupwareresource.h"

#include "configdialog.h"
#include "davcollectiondeletejob.h"
#include "davcollectionsfetchjob.h"
#include "davcollectionsmultifetchjob.h"
#include "davfreebusyhandler.h"
#include "davitemcreatejob.h"
#include "davitemdeletejob.h"
#include "davitemfetchjob.h"
#include "davitemmodifyjob.h"
#include "davitemsfetchjob.h"
#include "davitemslistjob.h"
#include "davmanager.h"
#include "davprotocolattribute.h"
#include "davprotocolbase.h"
#include "settings.h"
#include "settingsadaptor.h"
#include "setupwizard.h"

#include <kcalcore/freebusy.h>
#include <kcalcore/incidence.h>
#include <kcalcore/icalformat.h>
#include <KCalCore/MemoryCalendar>
#include <kcalcore/todo.h>
#include <kdatetime.h>
#include <kjob.h>
#include <klocale.h>

#include <akonadi/attributefactory.h>
#include <akonadi/cachepolicy.h>
#include <akonadi/changerecorder.h>
#include <akonadi/collectionfetchscope.h>
#include <akonadi/entitydisplayattribute.h>
#include <akonadi/itemdeletejob.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/recursiveitemfetchjob.h>
#include <akonadi/itemmodifyjob.h>

#include <kabc/addressee.h>
#include <kabc/vcardconverter.h>
#include <kwindowsystem.h>

#include <QtCore/QSet>
#include <QtDBus/QDBusConnection>

using namespace Akonadi;

typedef QSharedPointer<KCalCore::Incidence> IncidencePtr;

DavGroupwareResource::DavGroupwareResource( const QString &id )
  : ResourceBase( id ), FreeBusyProviderBase(), mSyncErrorNotified( false )
{
  AttributeFactory::registerAttribute<EntityDisplayAttribute>();
  AttributeFactory::registerAttribute<DavProtocolAttribute>();

  setNeedsNetwork( true );

  mDavCollectionRoot.setParentCollection( Collection::root() );
  mDavCollectionRoot.setName( identifier() );
  mDavCollectionRoot.setRemoteId( identifier() );
  mDavCollectionRoot.setContentMimeTypes( QStringList() << Collection::mimeType() );
  mDavCollectionRoot.setRights( Collection::CanCreateCollection | Collection::CanDeleteCollection | Collection::CanChangeCollection );

  EntityDisplayAttribute *attribute = mDavCollectionRoot.attribute<EntityDisplayAttribute>( Collection::AddIfMissing );
  attribute->setIconName( QLatin1String( "folder-remote" ) );

  int refreshInterval = Settings::self()->refreshInterval();
  if ( refreshInterval == 0 )
    refreshInterval = -1;

  Akonadi::CachePolicy cachePolicy;
  cachePolicy.setInheritFromParent( false );
  cachePolicy.setSyncOnDemand( false );
  cachePolicy.setCacheTimeout( -1 );
  cachePolicy.setIntervalCheckTime( refreshInterval );
  cachePolicy.setLocalParts( QStringList() << QLatin1String( "ALL" ) );
  mDavCollectionRoot.setCachePolicy( cachePolicy );

  changeRecorder()->fetchCollection( true );
  changeRecorder()->collectionFetchScope().setAncestorRetrieval( Akonadi::CollectionFetchScope::All );
  changeRecorder()->itemFetchScope().fetchFullPayload( true );
  changeRecorder()->itemFetchScope().setAncestorRetrieval( ItemFetchScope::All );

  Settings::self()->setWinId( winIdForDialogs() );
  Settings::self()->setResourceIdentifier( identifier() );

  mFreeBusyHandler = new DavFreeBusyHandler( this );
  connect( mFreeBusyHandler, SIGNAL(handlesFreeBusy(QString,bool)), this, SLOT(onHandlesFreeBusy(QString,bool)) );
  connect( mFreeBusyHandler, SIGNAL(freeBusyRetrieved(QString,QString,bool,QString)), this, SLOT(onFreeBusyRetrieved(QString,QString,bool,QString)) );

  connect(this, SIGNAL(reloadConfiguration()), this, SLOT(onReloadConfig()));

  scheduleCustomTask( this, "createInitialCache", QVariant(), ResourceBase::AfterChangeReplay );
}

DavGroupwareResource::~DavGroupwareResource()
{
  delete mFreeBusyHandler;
}

void DavGroupwareResource::collectionRemoved( const Akonadi::Collection &collection )
{
  kDebug() << "Removing collection " << collection.remoteId();

  if ( !configurationIsValid() ) {
    return;
  }

  const DavUtils::DavUrl davUrl = Settings::self()->davUrlFromCollectionUrl( collection.remoteId() );

  DavCollectionDeleteJob *job = new DavCollectionDeleteJob( davUrl );
  job->setProperty( "collection", QVariant::fromValue( collection ) );
  connect( job, SIGNAL(result(KJob*)), SLOT(onCollectionRemovedFinished(KJob*)) );
  job->start();
}

void DavGroupwareResource::cleanup()
{
  Settings::self()->cleanup();
  Akonadi::AgentBase::cleanup();
}

KDateTime DavGroupwareResource::lastCacheUpdate() const
{
  return KDateTime::currentLocalDateTime();
}

void DavGroupwareResource::canHandleFreeBusy( const QString& email ) const
{
  if ( !isOnline() )
    handlesFreeBusy( email, false );
  else
    mFreeBusyHandler->canHandleFreeBusy( email );
}

void DavGroupwareResource::onHandlesFreeBusy( const QString &email, bool handles )
{
  handlesFreeBusy( email, handles );
}

void DavGroupwareResource::retrieveFreeBusy( const QString& email, const KDateTime& start, const KDateTime& end )
{
  if ( !isOnline() )
    freeBusyRetrieved( email, QString(), false, i18n( "Unable to retrieve free-busy info while offline" ) );
  else
    mFreeBusyHandler->retrieveFreeBusy( email, start, end );
}

void DavGroupwareResource::onFreeBusyRetrieved( const QString& email, const QString& freeBusy, bool success, const QString &errorText )
{
  freeBusyRetrieved( email, freeBusy, success, errorText );
}

void DavGroupwareResource::configure( WId windowId )
{
  Settings::self()->setWinId( windowId );

  // On the initial configuration we start the setup wizard
  if ( Settings::self()->configuredDavUrls().isEmpty() ) {
    SetupWizard wizard;

    if ( windowId )
      KWindowSystem::setMainWindow( &wizard, windowId );

    const int result = wizard.exec();
    if ( result == QDialog::Accepted ) {
      const SetupWizard::Url::List urls = wizard.urls();
      foreach ( const SetupWizard::Url &url, urls ) {
        Settings::UrlConfiguration *urlConfig = new Settings::UrlConfiguration();

        urlConfig->mUrl = url.url;
        urlConfig->mProtocol = url.protocol;
        urlConfig->mUser = url.userName;
        urlConfig->mPassword = wizard.field( QLatin1String("credentialsPassword") ).toString();

        Settings::self()->newUrlConfiguration( urlConfig );
      }

      if ( !urls.isEmpty() )
        Settings::self()->setDisplayName( wizard.displayName() );

      QString defaultUser = wizard.field( QLatin1String("credentialsUserName") ).toString();
      if ( !defaultUser.isEmpty() ) {
        Settings::self()->setDefaultUsername( defaultUser );
        Settings::self()->setDefaultPassword( wizard.field( QLatin1String("credentialsPassword") ).toString() );
      }
    }
  }

  // continue with the normal config dialog
  ConfigDialog dialog;

  if ( windowId )
    KWindowSystem::setMainWindow( &dialog, windowId );

  if ( !Settings::self()->defaultUsername().isEmpty() )
    dialog.setPassword( Settings::self()->defaultPassword() );

  const int result = dialog.exec();

  if ( result == QDialog::Accepted ) {
    Settings::self()->setSettingsVersion( 3 );
    Settings::self()->writeConfig();
    synchronize();
    emit configurationDialogAccepted();
  } else {
    emit configurationDialogRejected();
  }
}

void DavGroupwareResource::retrieveCollections()
{
  kDebug() << "Retrieving collections list";
  mSyncErrorNotified = false;

  if ( !configurationIsValid() ) {
    return;
  }

  emit status( Running, i18n( "Fetching collections" ) );

  DavCollectionsMultiFetchJob *job = new DavCollectionsMultiFetchJob( Settings::self()->configuredDavUrls() );
  connect( job, SIGNAL(result(KJob*)), SLOT(onRetrieveCollectionsFinished(KJob*)) );
  connect( job, SIGNAL(collectionDiscovered(int,QString,QString)),
           SLOT(onCollectionDiscovered(int,QString,QString)) );
  job->start();
}

void DavGroupwareResource::retrieveItems( const Akonadi::Collection &collection )
{
  kDebug() << "Retrieving items for collection " << collection.remoteId();

  if ( !configurationIsValid() ) {
    return;
  }

  // As the resource root collection contains mime types for items we must
  // work around the fact that Akonadi will rightfully try to retrieve items
  // from it. So just return an empty list
  if ( collection.remoteId() == identifier() ) {
    itemsRetrievalDone();
    return;
  }

  const DavUtils::DavUrl davUrl = Settings::self()->davUrlFromCollectionUrl( collection.remoteId() );

  if ( !davUrl.url().isValid() ) {
    kError() << "Can't find a configured URL, collection.remoteId() is " << collection.remoteId();
    cancelTask( i18n( "Asked to retrieve items for an unknown collection: %1", collection.remoteId() ) );
    //Q_ASSERT_X( false, "DavGroupwareResource::retrieveItems", "Url is invalid" );
    return;
  }

  DavItemsListJob *job = new DavItemsListJob( davUrl );
  job->setProperty( "collection", QVariant::fromValue( collection ) );
  job->setContentMimeTypes( collection.contentMimeTypes() );
  connect( job, SIGNAL(result(KJob*)), SLOT(onRetrieveItemsFinished(KJob*)) );
  job->start();
}

bool DavGroupwareResource::retrieveItem( const Akonadi::Item &item, const QSet<QByteArray>& )
{
  kDebug() << "Retrieving single item. Remote id = " << item.remoteId();

  if ( !configurationIsValid() ) {
    return false;
  }

  const DavUtils::DavUrl davUrl = Settings::self()->davUrlFromCollectionUrl( item.parentCollection().remoteId(), item.remoteId() );
  if ( !davUrl.url().isValid() ) {
    cancelTask();
    return false;
  }

  DavItem davItem;
  davItem.setUrl( item.remoteId() );
  davItem.setContentType( QLatin1String("text/calendar") );
  davItem.setEtag( item.remoteRevision() );

  DavItemFetchJob *job = new DavItemFetchJob( davUrl, davItem );
  job->setProperty( "item", QVariant::fromValue( item ) );
  connect( job, SIGNAL(result(KJob*)), SLOT(onRetrieveItemFinished(KJob*)) );
  job->start();

  return true;
}

void DavGroupwareResource::itemAdded( const Akonadi::Item &item, const Akonadi::Collection &collection )
{
  kDebug() << "Received notification for added item. Local id = "
      << item.id() << ". Remote id = " << item.remoteId()
      << ". Collection remote id = " << collection.remoteId();

  if ( !configurationIsValid() ) {
    return;
  }

  if ( collection.remoteId().isEmpty() ) {
    kError() << "Invalid remote id for collection " << collection.id() << " = " << collection.remoteId();
    cancelTask( i18n( "Invalid collection for item %1.", item.id() ) );
    return;
  }

  DavItem davItem = DavUtils::createDavItem( item, collection );
  if ( davItem.data().isEmpty() ) {
    kError() << "Item " << item.id() << " doesn't has a valid payload";
    cancelTask();
    return;
  }

  QString urlStr = davItem.url();
  const DavUtils::DavUrl davUrl = Settings::self()->davUrlFromCollectionUrl( collection.remoteId(), urlStr );
  kDebug() << "Item " << item.id() << " will be put to " << urlStr;

  DavItemCreateJob *job = new DavItemCreateJob( davUrl, davItem );
  job->setProperty( "collection", QVariant::fromValue( collection ) );
  job->setProperty( "item", QVariant::fromValue( item ) );
  connect( job, SIGNAL(result(KJob*)), SLOT(onItemAddedFinished(KJob*)) );
  job->start();
}

void DavGroupwareResource::itemChanged( const Akonadi::Item &item, const QSet<QByteArray>& )
{
  kDebug() << "Received notification for changed item. Local id = " << item.id()
      << ". Remote id = " << item.remoteId();

  if ( !configurationIsValid() ) {
    return;
  }

  QString ridBase = item.remoteId();
  if ( ridBase.contains( QChar( '#' ) ) )
    ridBase.truncate( ridBase.indexOf( QChar( '#' ) ) );

  Akonadi::Item::List extraItems;
  foreach ( const QString &rid, mEtagCache.etags() ) {
    if ( rid.startsWith( ridBase ) && rid != item.remoteId() ) {
      Akonadi::Item extraItem;
      extraItem.setRemoteId( rid );
      extraItems << extraItem;
    }
  }

  if ( extraItems.isEmpty() ) {
    doItemChange( item );
  }
  else {
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( extraItems );
    job->setCollection( item.parentCollection() );
    job->fetchScope().fetchFullPayload();
    job->setProperty( "item", QVariant::fromValue( item ) );
    connect( job, SIGNAL(result(KJob*)), this, SLOT(onItemChangePrepared(KJob*)) );
  }
}

void DavGroupwareResource::onItemChangePrepared( KJob *job )
{
  Akonadi::ItemFetchJob *fetchJob = qobject_cast<Akonadi::ItemFetchJob*>( job );
  Akonadi::Item item = job->property( "item" ).value<Akonadi::Item>();
  doItemChange( item, fetchJob->items() );
}

void DavGroupwareResource::doItemChange( const Akonadi::Item &item, const Akonadi::Item::List &dependentItems )
{
  DavItem davItem = DavUtils::createDavItem( item, item.parentCollection(), dependentItems );
  if ( davItem.data().isEmpty() ) {
    cancelTask();
    return;
  }

  QString url = item.remoteId();
  if ( url.contains( QChar( '#' ) ) )
    url.truncate( url.indexOf( QChar( '#' ) ) );
  const DavUtils::DavUrl davUrl = Settings::self()->davUrlFromCollectionUrl( item.parentCollection().remoteId(), url );

  // We have to re-set the URL as it's not necessarily valid after createDavItem()
  davItem.setUrl( url );
  davItem.setEtag( item.remoteRevision() );

  DavItemModifyJob *modJob = new DavItemModifyJob( davUrl, davItem );
  modJob->setProperty( "item", QVariant::fromValue( item ) );
  modJob->setProperty( "dependentItems", QVariant::fromValue( dependentItems ) );
  connect( modJob, SIGNAL(result(KJob*)), SLOT(onItemChangedFinished(KJob*)) );
  modJob->start();
}

void DavGroupwareResource::itemRemoved( const Akonadi::Item &item )
{
  if ( !configurationIsValid() ) {
    return;
  }

  QString ridBase = item.remoteId();
  if ( ridBase.contains( QChar( '#' ) ) ) {
    // A bit tricky: we must remove an incidence contained in a resource
    // containing multiple ones.
    ridBase.truncate( ridBase.indexOf( QChar( '#' ) ) );

    Akonadi::Item::List extraItems;
    foreach ( const QString &rid, mEtagCache.etags() ) {
      if ( rid.startsWith( ridBase ) && rid != item.remoteId() ) {
        Akonadi::Item extraItem;
        extraItem.setRemoteId( rid );
        extraItems << extraItem;
      }
    }
  
    if ( extraItems.isEmpty() ) {
      // Urrrr?
      // Well, just delete the item.
      doItemRemoval( item );
    }
    else {
      Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( extraItems );
      job->setCollection( item.parentCollection() );
      job->fetchScope().fetchFullPayload();
      job->setProperty( "item", QVariant::fromValue( item ) );
      connect( job, SIGNAL(result(KJob*)), this, SLOT(onItemRemovalPrepared(KJob*)) );
    }
  }
  else {
    // easy as pie: just remove everything at the URL.
    doItemRemoval( item );
  }
}

void DavGroupwareResource::onItemRemovalPrepared( KJob *job )
{
  Akonadi::ItemFetchJob *fetchJob = qobject_cast<Akonadi::ItemFetchJob*>( job );
  Akonadi::Item item = job->property( "item" ).value<Akonadi::Item>();
  Akonadi::Item::List keptItems = fetchJob->items();

  if ( keptItems.isEmpty() ) {
    // Urrrr? Not again!
    doItemRemoval( item );
  }
  else {
    Akonadi::Item mainItem;
    Akonadi::Item::List extraItems;
    QString ridBase = item.remoteId();
    ridBase.truncate( ridBase.indexOf( QChar( '#' ) ) );

    foreach ( const Akonadi::Item &kept, keptItems ) {
      if ( kept.remoteId() == ridBase && extraItems.isEmpty() )
        mainItem = kept;
      else
        extraItems << kept;
    }

    if ( !mainItem.hasPayload() )
      mainItem = extraItems.takeFirst();

    const DavUtils::DavUrl davUrl = Settings::self()->davUrlFromCollectionUrl( item.parentCollection().remoteId(), ridBase );

    DavItem davItem = DavUtils::createDavItem( mainItem, mainItem.parentCollection(), extraItems );
    davItem.setUrl( ridBase );
    davItem.setEtag( item.remoteRevision() );

    DavItemModifyJob *modJob = new DavItemModifyJob( davUrl, davItem );
    modJob->setProperty( "item", QVariant::fromValue( mainItem ) );
    modJob->setProperty( "dependentItems", QVariant::fromValue( extraItems ) );
    modJob->setProperty( "isRemoval", QVariant::fromValue( true ) );
    modJob->setProperty( "removedItem", QVariant::fromValue( item ) );
    connect( modJob, SIGNAL(result(KJob*)), SLOT(onItemChangedFinished(KJob*)) );
    modJob->start();
  }
}

void DavGroupwareResource::doItemRemoval( const Akonadi::Item &item )
{
  const DavUtils::DavUrl davUrl = Settings::self()->davUrlFromCollectionUrl( item.parentCollection().remoteId(), item.remoteId() );

  DavItem davItem;
  davItem.setUrl( item.remoteId() );
  davItem.setEtag( item.remoteRevision() );

  DavItemDeleteJob *job = new DavItemDeleteJob( davUrl, davItem );
  job->setProperty( "item", QVariant::fromValue( item ) );
  job->setProperty( "collection", QVariant::fromValue( item.parentCollection() ) );
  connect( job, SIGNAL(result(KJob*)), SLOT(onItemRemovedFinished(KJob*)) );
  job->start();
}

void DavGroupwareResource::doSetOnline( bool online )
{
  kDebug() << "Resource changed online status to" << online;

  if ( online ) {
    synchronize();
  }

  ResourceBase::doSetOnline( online );
}

void DavGroupwareResource::createInitialCache()
{
  // Get all the items fetched by this resource
  Akonadi::RecursiveItemFetchJob *job = new Akonadi::RecursiveItemFetchJob( mDavCollectionRoot, QStringList() );
  job->fetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );
  connect( job, SIGNAL(result(KJob*)), this, SLOT(onCreateInitialCacheReady(KJob*)) );
  job->start();
}

void DavGroupwareResource::onCreateInitialCacheReady( KJob *job )
{
  Akonadi::RecursiveItemFetchJob *fetchJob = qobject_cast<Akonadi::RecursiveItemFetchJob*>( job );

  foreach ( const Akonadi::Item &item, fetchJob->items() ) {
    const QString rid = item.remoteId();
    if ( rid.isEmpty() )
      continue;

    const QString colRid = item.parentCollection().remoteId();
    if ( colRid.isEmpty() )
      continue;

    const QString etag = item.remoteRevision();
    if ( etag.isEmpty() )
      continue;

    mEtagCache.setEtag( rid, etag );

    if ( !mItemsRidCache.contains( colRid ) )
      mItemsRidCache.insert( colRid, QSet<QString>() );

    mItemsRidCache[colRid].insert( rid );
  }

  taskDone();
}

void DavGroupwareResource::onReloadConfig()
{
    Settings::self()->reloadConfig();
    synchronize();
}

void DavGroupwareResource::onCollectionRemovedFinished( KJob *job )
{
  if ( job->error() ) {
    cancelTask( i18n( "Unable to remove collection: %1", job->errorText() ) );
    return;
  }

  Akonadi::Collection collection = job->property( "collection" ).value<Akonadi::Collection>();
  if ( mItemsRidCache.contains( collection.remoteId() ) ) {
    foreach ( const QString &rid, mItemsRidCache.value( collection.remoteId() ) ) {
      mEtagCache.removeEtag( rid );
    }
    mItemsRidCache.remove( collection.remoteId() );
  }
  changeProcessed();
}

void DavGroupwareResource::onRetrieveCollectionsFinished( KJob *job )
{
  const DavCollectionsMultiFetchJob *fetchJob = qobject_cast<DavCollectionsMultiFetchJob*>( job );

  if ( job->error() ) {
    kWarning() << "Unable to fetch collections" << job->error() << job->errorText();
    cancelTask( i18n( "Unable to retrieve collections: %1", job->errorText() ) );
    mSyncErrorNotified = true;
    return;
  }

  Akonadi::Collection::List collections;
  collections << mDavCollectionRoot;
  QSet<QString> seenCollectionsUrls;

  const DavCollection::List davCollections = fetchJob->collections();

  foreach ( const DavCollection &davCollection, davCollections ) {
    if ( seenCollectionsUrls.contains( davCollection.url() ) )
      continue;
    else
      seenCollectionsUrls.insert( davCollection.url() );

    if ( !mItemsRidCache.contains( davCollection.url() ) )
      mItemsRidCache.insert( davCollection.url(), QSet<QString>() );

    Akonadi::Collection collection;
    collection.setParentCollection( mDavCollectionRoot );
    collection.setRemoteId( davCollection.url() );
    collection.setName( collection.remoteId() );

    if ( !davCollection.displayName().isEmpty() ) {
      EntityDisplayAttribute *attr = collection.attribute<EntityDisplayAttribute>( Collection::AddIfMissing );
      attr->setDisplayName( davCollection.displayName() );
    }

    QStringList mimeTypes;
    mimeTypes << Collection::mimeType();

    const DavCollection::ContentTypes contentTypes = davCollection.contentTypes();
    if ( contentTypes & DavCollection::Calendar )
      mimeTypes << QLatin1String( "text/calendar" );

    if ( contentTypes & DavCollection::Events )
      mimeTypes << KCalCore::Event::eventMimeType();

    if ( contentTypes & DavCollection::Todos )
      mimeTypes << KCalCore::Todo::todoMimeType();

    if ( contentTypes & DavCollection::Contacts )
      mimeTypes << KABC::Addressee::mimeType();

    if ( contentTypes & DavCollection::FreeBusy )
      mimeTypes << KCalCore::FreeBusy::freeBusyMimeType();

    if ( contentTypes & DavCollection::Journal )
      mimeTypes << KCalCore::Journal::journalMimeType();

    collection.setContentMimeTypes( mimeTypes );
    setCollectionIcon( collection /*by-ref*/ );

    DavProtocolAttribute *protoAttr = collection.attribute<DavProtocolAttribute>( Collection::AddIfMissing );
    protoAttr->setDavProtocol( davCollection.protocol() );

    DavUtils::Privileges privileges = davCollection.privileges();
    Akonadi::Collection::Rights rights;

    if ( privileges & DavUtils::All || privileges & DavUtils::Write )
      rights |= Akonadi::Collection::AllRights;

    if ( privileges & DavUtils::WriteContent )
      rights |= Akonadi::Collection::CanChangeItem;

    if ( privileges & DavUtils::Bind )
      rights |= Akonadi::Collection::CanCreateItem;

    if ( privileges & DavUtils::Unbind )
      rights |= Akonadi::Collection::CanDeleteItem;

    if ( privileges == DavUtils::Read )
      rights |= Akonadi::Collection::ReadOnly;


    collection.setRights( rights );
    mEtagCache.sync( collection );
    collections << collection;
  }

  foreach ( const QString &rid, mItemsRidCache.keys() ) {
    if ( !seenCollectionsUrls.contains( rid ) ) {
      foreach ( const QString &itemRid, mItemsRidCache[rid] ) {
        mEtagCache.removeEtag( itemRid );
      }
      mItemsRidCache.remove( rid );
    }
  }

  collectionsRetrieved( collections );
}

void DavGroupwareResource::onRetrieveItemsFinished( KJob *job )
{
  if ( job->error() ) {
    if ( mSyncErrorNotified ) {
      cancelTask();
    }
    else {
      cancelTask( i18n( "Unable to retrieve items: %1", job->errorText() ) );
      mSyncErrorNotified = true;
    }
    return;
  }

  const Collection collection = job->property( "collection" ).value<Collection>();
  const DavUtils::DavUrl davUrl = Settings::self()->davUrlFromCollectionUrl( collection.remoteId() );
  const bool protocolSupportsMultiget = DavManager::self()->davProtocol( davUrl.protocol() )->useMultiget();

  const DavItemsListJob *listJob = qobject_cast<DavItemsListJob*>( job );

  Akonadi::Item::List changedItems;
  QSet<QString> seenRids;
  QStringList changedRids;

  const DavItem::List davItems = listJob->items();
  foreach ( const DavItem &davItem, davItems ) {
    seenRids.insert( davItem.url() );

    Akonadi::Item item;
    item.setParentCollection( collection );
    item.setRemoteId( davItem.url() );
    item.setMimeType( davItem.contentType() );
    item.setRemoteRevision( davItem.etag() );

    if ( mEtagCache.etagChanged( item.remoteId(), davItem.etag() ) ) {
      mEtagCache.markAsChanged( item.remoteId() );
      changedRids << item.remoteId();
      changedItems << item;

      // Only clear the payload (and therefor trigger a refetch from the backend) if we
      // do not use multiget, because in this case we fetch the complete payload
      // some lines below already.
      if ( !protocolSupportsMultiget ) {
        kDebug() << "Outdated item " << item.remoteId() << " (etag = " << davItem.etag() << ")";
        item.clearPayload();
      }
    }
  }

  QSet<QString> removedRids = mItemsRidCache[collection.remoteId()];
  mItemsRidCache[collection.remoteId()] = seenRids;
  removedRids.subtract( seenRids );
  foreach ( const QString &rmd, removedRids ) {
    Akonadi::Item item;
    item.setParentCollection( collection );
    item.setRemoteId( rmd );
    mEtagCache.removeEtag( rmd );

    // Use a job to delete items as itemsRetrievedIncremental seem to choke
    // when many items are given with just their RID.
    Akonadi::ItemDeleteJob *deleteJob = new Akonadi::ItemDeleteJob( item );
    deleteJob->start();
  }


  // If the protocol supports multiget then deviate from the expected behavior
  // and fetch all items with payload now instead of waiting for Akonadi to
  // request it item by item in retrieveItem().
  // This allows the resource to use the multiget query and let it be nice
  // to the remote server : only one request for n items instead of n requests.
  if ( protocolSupportsMultiget && !changedRids.isEmpty() ) {
    DavItemsFetchJob *fetchJob = new DavItemsFetchJob( davUrl, changedRids );
    connect( fetchJob, SIGNAL(result(KJob*)), this, SLOT(onMultigetFinished(KJob*)) );
    fetchJob->setProperty( "items", QVariant::fromValue( changedItems ) );
    fetchJob->start();
    // delay the call of itemsRetrieved() to onMultigetFinished()
  } else {
    itemsRetrievedIncremental( changedItems, Akonadi::Item::List() );
  }
}

void DavGroupwareResource::onMultigetFinished( KJob *job )
{
  if ( job->error() ) {
    if ( mSyncErrorNotified ) {
      cancelTask();
    }
    else {
      cancelTask( i18n( "Unable to retrieve items: %1", job->errorText() ) );
      mSyncErrorNotified = true;
    }
    return;
  }

  const Akonadi::Item::List origItems = job->property( "items" ).value<Akonadi::Item::List>();
  const DavItemsFetchJob *davJob = qobject_cast<DavItemsFetchJob*>( job );

  Akonadi::Item::List items;
  foreach ( Akonadi::Item item, origItems ) { //krazy:exclude=foreach non-const is intended here
    const DavItem davItem = davJob->item( item.remoteId() );

    // No data was retrieved for this item, maybe because it is not out of date
    if ( davItem.data().isEmpty() ) {
      if ( !mEtagCache.isOutOfDate( item.remoteId() ) )
        items << item;
      continue;
    }

    Akonadi::Item::List extraItems;
    if ( !DavUtils::parseDavData( davItem, item, extraItems ) )
      continue;

    // update etag
    item.setRemoteRevision( davItem.etag() );
    mEtagCache.setEtag( item.remoteId(), davItem.etag() );
    items << item;
    foreach ( const Akonadi::Item &extraItem, extraItems ) {
      mEtagCache.setEtag( extraItem.remoteId(), davItem.etag() );
      items << extraItem;
    }
  }

  itemsRetrievedIncremental( items, Akonadi::Item::List() );
}

void DavGroupwareResource::onRetrieveItemFinished( KJob *job )
{
  onItemFetched( job, ItemUpdateAdd );
}

void DavGroupwareResource::onItemRefreshed( KJob* job )
{
  ItemFetchUpdateType update = ItemUpdateChange;
  if ( job->property( "isRemoval" ).isValid() && job->property( "isRemoval" ).toBool() )
    update = ItemUpdateNone;

  onItemFetched( job, update );
}

void DavGroupwareResource::onItemFetched( KJob* job, ItemFetchUpdateType updateType )
{
  if ( job->error() ) {
    if ( mSyncErrorNotified ) {
      cancelTask();
    }
    else {
      cancelTask( i18n( "Unable to retrieve item: %1", job->errorText() ) );
      mSyncErrorNotified = true;
    }
    return;
  }

  const DavItemFetchJob *fetchJob = qobject_cast<DavItemFetchJob*>( job );
  const DavItem davItem = fetchJob->item();
  Akonadi::Item item = fetchJob->property( "item" ).value<Akonadi::Item>();

  Akonadi::Item::List extraItems;
  if ( !DavUtils::parseDavData( davItem, item, extraItems ) )
    return;

  // update etag
  item.setRemoteRevision( davItem.etag() );
  mEtagCache.setEtag( item.remoteId(), davItem.etag() );

  if ( !extraItems.isEmpty() ) {
    for ( int i = 0; i < extraItems.size(); ++i )
      mEtagCache.setEtag( extraItems.at( i ).remoteId(), davItem.etag() );

    Akonadi::ItemModifyJob *j = new Akonadi::ItemModifyJob( extraItems );
    j->setIgnorePayload( true );
  }

  if ( updateType == ItemUpdateChange )
    changeCommitted( item );
  else if ( updateType == ItemUpdateAdd )
    itemRetrieved( item );
}

void DavGroupwareResource::onItemAddedFinished( KJob *job )
{
  const DavItemCreateJob *createJob = qobject_cast<DavItemCreateJob*>( job );
  const DavItem davItem = createJob->item();
  Akonadi::Item item = createJob->property( "item" ).value<Akonadi::Item>();
  item.setRemoteId( davItem.url() );

  if ( createJob->error() ) {
    kError() << "Error when uploading item:" << createJob->error() << createJob->errorString();
    if ( createJob->canRetryLater() ) {
      retryAfterFailure(createJob->errorString());
    }
    else {
      cancelTask( i18n( "Unable to add item: %1", createJob->errorString() ) );
    }
    return;
  }

  Akonadi::Collection collection = createJob->property( "collection" ).value<Akonadi::Collection>();
  mItemsRidCache[collection.remoteId()].insert( davItem.url() );

  if ( davItem.etag().isEmpty() ) {
    const DavUtils::DavUrl davUrl = Settings::self()->davUrlFromCollectionUrl( item.parentCollection().remoteId(), item.remoteId() );
    DavItemFetchJob *fetchJob = new DavItemFetchJob( davUrl, davItem );
    fetchJob->setProperty( "item", QVariant::fromValue( item ) );
    connect( fetchJob, SIGNAL(result(KJob*)), SLOT(onItemRefreshed(KJob*)) );
    fetchJob->start();
  } else {
    item.setRemoteRevision( davItem.etag() );
    mEtagCache.setEtag( davItem.url(), davItem.etag() );
    changeCommitted( item );
  }
}

void DavGroupwareResource::onItemChangedFinished( KJob *job )
{
  const DavItemModifyJob *modifyJob = qobject_cast<DavItemModifyJob*>( job );
  const DavItem davItem = modifyJob->item();
  Akonadi::Item item = modifyJob->property( "item" ).value<Akonadi::Item>();
  Akonadi::Item::List dependentItems = modifyJob->property( "dependentItems" ).value<Akonadi::Item::List>();
  bool isRemoval = modifyJob->property( "isRemoval" ).isValid() && modifyJob->property( "isRemoval" ).toBool();

  if ( modifyJob->error() ) {
    kError() << "Error when uploading item:" << modifyJob->error() << modifyJob->errorString();
    if ( modifyJob->canRetryLater() ) {
      retryAfterFailure(modifyJob->errorString());
    }
    else {
      cancelTask( i18n( "Unable to change item: %1", modifyJob->errorString() ) );
    }
    return;
  }

  if ( isRemoval ) {
    Akonadi::Item removedItem = job->property( "removedItem" ).value<Akonadi::Item>();
    mEtagCache.removeEtag( removedItem.remoteId() );
    mItemsRidCache[removedItem.parentCollection().remoteId()].remove( removedItem.remoteId() );
    changeProcessed();
  }

  if ( davItem.etag().isEmpty() ) {
    const DavUtils::DavUrl davUrl = Settings::self()->davUrlFromCollectionUrl( item.parentCollection().remoteId(), item.remoteId() );
    DavItemFetchJob *fetchJob = new DavItemFetchJob( davUrl, davItem );
    fetchJob->setProperty( "item", QVariant::fromValue( item ) );
    fetchJob->setProperty( "dependentItems", QVariant::fromValue( dependentItems ) );
    fetchJob->setProperty( "isRemoval", QVariant::fromValue( isRemoval ) );
    connect( fetchJob, SIGNAL(result(KJob*)), SLOT(onItemRefreshed(KJob*)) );
    fetchJob->start();
  } else {
    if ( !isRemoval ) {
      item.setRemoteRevision( davItem.etag() );
      mEtagCache.setEtag( davItem.url(), davItem.etag() );
      changeCommitted( item );
    }

    if ( !dependentItems.isEmpty() ) {
      for ( int i = 0; i < dependentItems.size(); ++i ) {
        dependentItems[i].setRemoteRevision( davItem.etag() );
        mEtagCache.setEtag( dependentItems.at( i ).remoteId(), davItem.etag() );
      }
      
      Akonadi::ItemModifyJob *j = new Akonadi::ItemModifyJob( dependentItems );
      j->setIgnorePayload( true );
    }
  }
}

void DavGroupwareResource::onItemRemovedFinished( KJob *job )
{
  if ( job->error() ) {
    const DavItemDeleteJob *deleteJob = qobject_cast<DavItemDeleteJob*>( job );

    if ( deleteJob->canRetryLater() ) {
      retryAfterFailure(job->errorString());
    }
    else {
      cancelTask( i18n( "Unable to remove item: %1", job->errorString() ) );
    }
  }
  else {
    Akonadi::Item item = job->property( "item" ).value<Akonadi::Item>();
    Akonadi::Collection collection = job->property( "collection" ).value<Akonadi::Collection>();
    mItemsRidCache[collection.remoteId()].remove( item.remoteId() );
    mEtagCache.removeEtag( item.remoteId() );
    changeProcessed();
  }
}

void DavGroupwareResource::onCollectionDiscovered( int protocol, const QString &collection, const QString &config )
{
  Settings::self()->addCollectionUrlMapping( DavUtils::Protocol( protocol ), collection, config );
}

void DavGroupwareResource::onEtagChanged(const QString& itemUrl, const QString& etag)
{
  mEtagCache.setEtag( itemUrl, etag );
}

bool DavGroupwareResource::configurationIsValid()
{
  if ( Settings::self()->configuredDavUrls().empty() ) {
    emit status( NotConfigured, i18n( "The resource is not configured yet" ) );
    cancelTask( i18n( "The resource is not configured yet" ) );
    return false;
  }

  int newICT = Settings::self()->refreshInterval();
  if ( newICT == 0 )
    newICT = -1;

  if ( newICT != mDavCollectionRoot.cachePolicy().intervalCheckTime() ) {
    Akonadi::CachePolicy cachePolicy = mDavCollectionRoot.cachePolicy();
    cachePolicy.setIntervalCheckTime( newICT );

    mDavCollectionRoot.setCachePolicy( cachePolicy );
  }

  if ( !Settings::self()->displayName().isEmpty() ) {
    EntityDisplayAttribute *attribute = mDavCollectionRoot.attribute<EntityDisplayAttribute>( Collection::AddIfMissing );
    attribute->setDisplayName( Settings::self()->displayName() );
    setName( Settings::self()->displayName() );
  }

  return true;
}

void DavGroupwareResource::retryAfterFailure(const QString &errorMessage)
{
  emit status(Broken, errorMessage);
  deferTask();
  setTemporaryOffline(Settings::self()->refreshInterval() <= 0 ? 300 : Settings::self()->refreshInterval() * 60);
}

/*static*/
void DavGroupwareResource::setCollectionIcon( Akonadi::Collection &collection )
{
  const QStringList mimeTypes = collection.contentMimeTypes();
  if ( mimeTypes.count() == 1 ) {
    QHash<QString,QString> mapping;
    mapping.insert( KCalCore::Event::eventMimeType(), QLatin1String( "view-calendar" ) );
    mapping.insert( KCalCore::Todo::todoMimeType(), QLatin1String( "view-calendar-tasks" ) );
    mapping.insert( KCalCore::Journal::journalMimeType(), QLatin1String( "view-pim-journal" ) );
    mapping.insert( KABC::Addressee::mimeType(), QLatin1String( "view-pim-contacts" ) );

    if ( mapping.contains( mimeTypes.first() ) ) {
      EntityDisplayAttribute *attribute = collection.attribute<EntityDisplayAttribute>( Collection::AddIfMissing );
      attribute->setIconName( mapping.value( mimeTypes.first() ) );
    }
  }
}

AKONADI_RESOURCE_MAIN( DavGroupwareResource )

