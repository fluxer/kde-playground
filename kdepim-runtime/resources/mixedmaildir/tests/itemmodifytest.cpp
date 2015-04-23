/*  This file is part of the KDE project
    Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
    Author: Kevin Krammer, krake@kdab.com
    Copyright (C) 2011 Kevin Krammer, kevin.krammer@gmx.at

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "mixedmaildirstore.h"

#include "testdatautil.h"

#include "filestore/itemfetchjob.h"
#include "filestore/itemmodifyjob.h"
#include "filestore/storecompactjob.h"

#include "libmaildir/maildir.h"

#include <akonadi/kmime/messageparts.h>
#include <akonadi/itemfetchscope.h>

#include <kmbox/mbox.h>
#include <kmime/kmime_message.h>

#include <KRandom>
#include <KTempDir>

#include <qtest_kde.h>

#include <QCryptographicHash>

using namespace Akonadi;
using namespace KMBox;

static bool fullEntryCompare( const MBoxEntry &a, const MBoxEntry &b )
{
  return a.messageOffset() == b.messageOffset() &&
         a.separatorSize() == b.separatorSize() &&
         a.messageSize() == b.messageSize();
}

class ItemModifyTest : public QObject
{
  Q_OBJECT

  public:
    ItemModifyTest()
      : QObject(), mStore( 0 ), mDir( 0 ) {}

    ~ItemModifyTest() {
      delete mStore;
      delete mDir;
    }

  private:
    MixedMaildirStore *mStore;
    KTempDir *mDir;

  private Q_SLOTS:
    void init();
    void cleanup();
    void testExpectedFail();
    void testIgnorePayload();
    void testModifyPayload();
    void testModifyFlags();
    void testModifyFlagsAndPayload();
};

void ItemModifyTest::init()
{
  mStore = new MixedMaildirStore;

  mDir = new KTempDir;
  QVERIFY( mDir->exists() );
}

void ItemModifyTest::cleanup()
{
  delete mStore;
  mStore = 0;
  delete mDir;
  mDir = 0;
}

void ItemModifyTest::testExpectedFail()
{
  QDir topDir( mDir->name() );

  QVERIFY( TestDataUtil::installFolder( QLatin1String( "maildir" ), topDir.path(), QLatin1String( "collection1" ) ) );
  QVERIFY( TestDataUtil::installFolder( QLatin1String( "mbox" ), topDir.path(), QLatin1String( "collection2" ) ) );

  KPIM::Maildir topLevelMd( topDir.path(), true );

  KPIM::Maildir md1 = topLevelMd.subFolder( QLatin1String( "collection1" ) );
  QSet<QString> entrySet1 = QSet<QString>::fromList( md1.entryList() );
  QCOMPARE( (int)entrySet1.count(), 4 );

  QFileInfo fileInfo2( topDir.path(), QLatin1String( "collection2" ) );
  MBox mbox2;
  QVERIFY( mbox2.load( fileInfo2.absoluteFilePath() ) );
  MBoxEntry::List entryList2 = mbox2.entries();
  QCOMPARE( (int)entryList2.count(), 4 );

  QSet<qint64> entrySet2;
  Q_FOREACH( const MBoxEntry &entry, entryList2 ) {
    entrySet2 << entry.messageOffset();
  }

  mStore->setPath( topDir.path() );

  // common variables
  FileStore::ItemModifyJob *job = 0;

  // test failure of modifying a non-existant maildir entry
  Collection collection1;
  collection1.setName( QLatin1String( "collection1" ) );
  collection1.setRemoteId( QLatin1String( "collection1" ) );
  collection1.setParentCollection( mStore->topLevelCollection() );

  QString remoteId1;
  do {
    remoteId1 = KRandom::randomString( 20 );
  } while ( entrySet1.contains( remoteId1 ) );

  KMime::Message::Ptr msgPtr( new KMime::Message );

  Item item1;
  item1.setMimeType( KMime::Message::mimeType() );
  item1.setRemoteId( remoteId1 );
  item1.setParentCollection( collection1 );
  item1.setPayload<KMime::Message::Ptr>( msgPtr );

  job = mStore->modifyItem( item1 );

  QVERIFY( !job->exec() );
  QCOMPARE( job->error(), (int)FileStore::Job::InvalidJobContext );

  QSet<QString> entrySet = QSet<QString>::fromList( md1.entryList() );
  QCOMPARE( entrySet, entrySet1 );

  // test failure of modifying a non-existant mbox entry
  Collection collection2;
  collection2.setName( QLatin1String( "collection2" ) );
  collection2.setRemoteId( QLatin1String( "collection2" ) );
  collection2.setParentCollection( mStore->topLevelCollection() );

  qint64 remoteId2;
  do {
    remoteId2 = KRandom::random();
  } while ( entrySet2.contains( remoteId2 ) );

  Item item2;
  item2.setMimeType( KMime::Message::mimeType() );
  item2.setRemoteId( QString::number( remoteId2 ) );
  item2.setParentCollection( collection2 );
  item2.setPayload<KMime::Message::Ptr>( msgPtr );

  job = mStore->modifyItem( item2 );

  QVERIFY( !job->exec() );
  QCOMPARE( job->error(), (int)FileStore::Job::InvalidJobContext );

  QVERIFY( mbox2.load( mbox2.fileName() ) );
  MBoxEntry::List entryList = mbox2.entries();
  QVERIFY( std::equal( entryList.begin(), entryList.end(), entryList2.begin(), fullEntryCompare ) );
}

void ItemModifyTest::testIgnorePayload()
{
  QDir topDir( mDir->name() );

  QVERIFY( TestDataUtil::installFolder( QLatin1String( "maildir" ), topDir.path(), QLatin1String( "collection1" ) ) );
  QVERIFY( TestDataUtil::installFolder( QLatin1String( "mbox" ), topDir.path(), QLatin1String( "collection2" ) ) );

  KPIM::Maildir topLevelMd( topDir.path(), true );

  KPIM::Maildir md1 = topLevelMd.subFolder( QLatin1String( "collection1" ) );
  QStringList entryList1 = md1.entryList();
  QCOMPARE( (int)entryList1.count(), 4 );

  QFileInfo fileInfo2( topDir.path(), QLatin1String( "collection2" ) );
  MBox mbox2;
  QVERIFY( mbox2.load( fileInfo2.absoluteFilePath() ) );
  MBoxEntry::List entryList2 = mbox2.entries();
  QCOMPARE( (int)entryList2.count(), 4 );

  mStore->setPath( topDir.path() );

  // common variables
  FileStore::ItemModifyJob *job = 0;

  // test failure of modifying a non-existant maildir entry
  Collection collection1;
  collection1.setName( QLatin1String( "collection1" ) );
  collection1.setRemoteId( QLatin1String( "collection1" ) );
  collection1.setParentCollection( mStore->topLevelCollection() );

  const QByteArray data1 = md1.readEntry( entryList1.first() );

  KMime::Message::Ptr msgPtr( new KMime::Message );
  msgPtr->subject()->from7BitString( "Modify Test" );
  msgPtr->assemble();

  Item item1;
  item1.setMimeType( KMime::Message::mimeType() );
  item1.setRemoteId( entryList1.first() );
  item1.setParentCollection( collection1 );
  item1.setPayload<KMime::Message::Ptr>( msgPtr );

  job = mStore->modifyItem( item1 );
  job->setIgnorePayload( true );

  QVERIFY( job->exec() );
  QCOMPARE( job->error(), 0 );

  QCOMPARE( md1.entryList(), entryList1 );
  QCOMPARE( md1.readEntry( entryList1.first() ), data1 );

  // test failure of modifying a non-existant mbox entry
  Collection collection2;
  collection2.setName( QLatin1String( "collection2" ) );
  collection2.setRemoteId( QLatin1String( "collection2" ) );
  collection2.setParentCollection( mStore->topLevelCollection() );

  const QByteArray data2 = mbox2.readRawMessage( MBoxEntry( 0 ) );

  Item item2;
  item2.setMimeType( KMime::Message::mimeType() );
  item2.setRemoteId( QLatin1String( "0" ) );
  item2.setParentCollection( collection2 );
  item2.setPayload<KMime::Message::Ptr>( msgPtr );

  job = mStore->modifyItem( item2 );
  job->setIgnorePayload( true );

  QVERIFY( job->exec() );
  QCOMPARE( job->error(), 0 );

  QVERIFY( mbox2.load( mbox2.fileName() ) );
  QCOMPARE( mbox2.entries(), entryList2 );
  QCOMPARE( mbox2.readRawMessage( MBoxEntry( 0 ) ), data2 );
}

void ItemModifyTest::testModifyPayload()
{
  QDir topDir( mDir->name() );

  QVERIFY( TestDataUtil::installFolder( QLatin1String( "maildir" ), topDir.path(), QLatin1String( "collection1" ) ) );
  QVERIFY( TestDataUtil::installFolder( QLatin1String( "mbox" ), topDir.path(), QLatin1String( "collection2" ) ) );

  KPIM::Maildir topLevelMd( topDir.path(), true );

  KPIM::Maildir md1 = topLevelMd.subFolder( QLatin1String( "collection1" ) );
  QStringList entryList1 = md1.entryList();
  QCOMPARE( (int)entryList1.count(), 4 );

  QFileInfo fileInfo2( topDir.path(), QLatin1String( "collection2" ) );
  MBox mbox2;
  QVERIFY( mbox2.load( fileInfo2.absoluteFilePath() ) );
  MBoxEntry::List entryList2 = mbox2.entries();
  QCOMPARE( (int)entryList2.count(), 4 );

  mStore->setPath( topDir.path() );

  // common variables
  FileStore::ItemModifyJob *job = 0;

  const QVariant colListVar = QVariant::fromValue<Collection::List>( Collection::List() );
  QVariant var;
  Collection::List collections;

  // test modifying a maildir entry's header
  Collection collection1;
  collection1.setName( QLatin1String( "collection1" ) );
  collection1.setRemoteId( QLatin1String( "collection1" ) );
  collection1.setParentCollection( mStore->topLevelCollection() );

  const QByteArray data1 = md1.readEntry( entryList1.first() );

  KMime::Message::Ptr msgPtr( new KMime::Message );
  msgPtr->setContent( KMime::CRLFtoLF( data1 ) );
  msgPtr->subject()->from7BitString( "Modify Test" );
  msgPtr->assemble();

  Item item1;
  item1.setMimeType( KMime::Message::mimeType() );
  item1.setRemoteId( entryList1.first() );
  item1.setParentCollection( collection1 );
  item1.setPayload<KMime::Message::Ptr>( msgPtr );

  job = mStore->modifyItem( item1 );
  // changing subject, so indicate a header change
  job->setParts( QSet<QByteArray>() << QByteArray( "PLD:" ) + MessagePart::Header );

  QVERIFY( job->exec() );
  QCOMPARE( job->error(), 0 );

  QCOMPARE( md1.entryList(), entryList1 );

  QCOMPARE( md1.readEntry( entryList1.first() ).size(),
            msgPtr->encodedContent().size() );
  QCOMPARE( md1.readEntry( entryList1.first() ), msgPtr->encodedContent() );

  // check for index preservation
  var = job->property( "onDiskIndexInvalidated" );
  QVERIFY( var.isValid() );
  QCOMPARE( var.userType(), colListVar.userType() );

  collections = var.value<Collection::List>();
  QCOMPARE( (int)collections.count(), 1 );
  QCOMPARE( collections.first(), collection1 );

  // test modifying an mbox entry's header
  Collection collection2;
  collection2.setName( QLatin1String( "collection2" ) );
  collection2.setRemoteId( QLatin1String( "collection2" ) );
  collection2.setParentCollection( mStore->topLevelCollection() );

  const QByteArray data2 = mbox2.readRawMessage( MBoxEntry( 0 ) );

  msgPtr->setContent( KMime::CRLFtoLF( data2 ) );
  msgPtr->subject()->from7BitString( "Modify Test" );
  msgPtr->assemble();

  Item item2;
  item2.setMimeType( KMime::Message::mimeType() );
  item2.setRemoteId( QLatin1String( "0" ) );
  item2.setParentCollection( collection2 );
  item2.setPayload<KMime::Message::Ptr>( msgPtr );

  job = mStore->modifyItem( item2 );
  // changing subject, so indicate a header change
  job->setParts( QSet<QByteArray>() << QByteArray( "PLD:" ) + MessagePart::Header );

  QVERIFY( job->exec() );
  QCOMPARE( job->error(), 0 );

  Item item = job->item();

  QVERIFY( mbox2.load( mbox2.fileName() ) );
  MBoxEntry::List entryList = mbox2.entries();
  QCOMPARE( (int)entryList.count(), 5 ); // mbox file not purged yet
  QCOMPARE( entryList.last().messageOffset(), item.remoteId().toULongLong() );

  var = job->property( "compactStore" );
  QVERIFY( var.isValid() );
  QCOMPARE( var.type(), QVariant::Bool );
  QCOMPARE( var.toBool(), true );

  // check for index preservation
  var = job->property( "onDiskIndexInvalidated" );
  QVERIFY( var.isValid() );
  QCOMPARE( var.userType(), colListVar.userType() );

  collections = var.value<Collection::List>();
  QCOMPARE( (int)collections.count(), 1 );
  QCOMPARE( collections.first(), collection2 );

  FileStore::ItemFetchJob *itemFetch = mStore->fetchItem( item2 );
  QVERIFY( !itemFetch->exec() ); // item at old offset gone

  FileStore::StoreCompactJob *storeCompact = mStore->compactStore();
  QVERIFY( storeCompact->exec() );

  QVERIFY( mbox2.load( mbox2.fileName() ) );
  entryList = mbox2.entries();
  QCOMPARE( (int)entryList.count(), 4 );

  QCOMPARE( mbox2.readRawMessage( entryList.last() ), msgPtr->encodedContent() );
}

void ItemModifyTest::testModifyFlags()
{
  QDir topDir( mDir->name() );

  QVERIFY( TestDataUtil::installFolder( QLatin1String( "maildir" ), topDir.path(), QLatin1String( "collection1" ) ) );
  QVERIFY( TestDataUtil::installFolder( QLatin1String( "mbox" ), topDir.path(), QLatin1String( "collection2" ) ) );

  KPIM::Maildir topLevelMd( topDir.path(), true );

  KPIM::Maildir md1 = topLevelMd.subFolder( QLatin1String( "collection1" ) );
  QStringList entryList1 = md1.entryList();
  QCOMPARE( (int)entryList1.count(), 4 );

  QFileInfo fileInfo2( topDir.path(), QLatin1String( "collection2" ) );
  MBox mbox2;
  QVERIFY( mbox2.load( fileInfo2.absoluteFilePath() ) );
  MBoxEntry::List entryList2 = mbox2.entries();
  QCOMPARE( (int)entryList2.count(), 4 );
  
  QCryptographicHash cryptoHash( QCryptographicHash::Sha1 );
  
  QFile file2( fileInfo2.absoluteFilePath() );
  QVERIFY( file2.open( QIODevice::ReadOnly ) );
  cryptoHash.addData( file2.readAll() );
  const QByteArray mbox2Sha1 = cryptoHash.result();
  
  file2.close();
  cryptoHash.reset();

  mStore->setPath( topDir.path() );

  // common variables
  FileStore::ItemModifyJob *job = 0;

  const QVariant colListVar = QVariant::fromValue<Collection::List>( Collection::List() );
  QVariant var;
  Collection::List collections;
  KMime::Message::Ptr msgPtr( new KMime::Message );
  
  // test modifying a flag of a maildir items changes the entry name but not the
  // message contents
  Collection collection1;
  collection1.setName( QLatin1String( "collection1" ) );
  collection1.setRemoteId( QLatin1String( "collection1" ) );
  collection1.setParentCollection( mStore->topLevelCollection() );

  // check that the \SEEN flag is not set yet
  QVERIFY( !md1.readEntryFlags( entryList1.first() ).contains( "\\SEEN" ) );
  
  const QByteArray data1 = md1.readEntry( entryList1.first() );

  msgPtr->setContent( KMime::CRLFtoLF( data1 ) );
  msgPtr->subject()->from7BitString( "Modify Test" );
  msgPtr->assemble();

  Item item1;
  item1.setMimeType( KMime::Message::mimeType() );
  item1.setRemoteId( entryList1.first() );
  item1.setParentCollection( collection1 );
  item1.setPayload<KMime::Message::Ptr>( msgPtr );
  item1.setFlag( "\\SEEN" );

  job = mStore->modifyItem( item1 );
  // setting \SEEN, so indicate a flags change
  job->setParts( QSet<QByteArray>() << "FLAGS" );
    
  QVERIFY( job->exec() );
  QCOMPARE( job->error(), 0 );

  Item item = job->item();
  
  // returned item should contain the change
  QVERIFY( item.flags().contains( "\\SEEN" ) );
  
  // remote ID has changed
  QVERIFY( item.remoteId() != entryList1.first() );
  QVERIFY( !md1.entryList().contains( entryList1.first() ) );
  
  // no change in number of entries, one difference
  QStringList entryList3 = md1.entryList();
  QCOMPARE( entryList3.count(), entryList1.count() );
  Q_FOREACH( const QString &oldEntry, entryList1 ) {
    entryList3.removeAll( oldEntry );
  }
  QCOMPARE( entryList3.count(), 1 );
  
  // no change to data
  QCOMPARE( md1.readEntry( entryList3.first() ), data1 );

  var = job->property( "onDiskIndexInvalidated" );
  QVERIFY( var.isValid() );
  QCOMPARE( var.userType(), colListVar.userType() );

  collections = var.value<Collection::List>();
  QCOMPARE( (int)collections.count(), 1 );
  QCOMPARE( collections.first(), collection1 );
  
  // fetch new item, check flag
  item1 = Item();
  item1.setMimeType( KMime::Message::mimeType() );
  item1.setRemoteId( entryList3.first() );
  item1.setParentCollection( collection1 );
  
  FileStore::ItemFetchJob *itemFetch = mStore->fetchItem( item1 );
  
  QVERIFY( itemFetch->exec() );
  QCOMPARE( itemFetch->error(), 0 );
  
  QCOMPARE( itemFetch->items().count(), 1 );
  QEXPECT_FAIL( "", "ItemFetch handling needs to be fixed to also fetch flags", Continue );
  QVERIFY( itemFetch->items()[ 0 ].flags().contains( "\\SEEN" ) );
  
  // test modifying flags of an mbox item "succeeds" (no error) but does not change
  // anything in store or on disk
  Collection collection2;
  collection2.setName( QLatin1String( "collection2" ) );
  collection2.setRemoteId( QLatin1String( "collection2" ) );
  collection2.setParentCollection( mStore->topLevelCollection() );

  const QByteArray data2 = mbox2.readRawMessage( MBoxEntry( 0 ) );

  msgPtr->setContent( KMime::CRLFtoLF( data2 ) );
  msgPtr->subject()->from7BitString( "Modify Test" );
  msgPtr->assemble();

  Item item2;
  item2.setMimeType( KMime::Message::mimeType() );
  item2.setRemoteId( QLatin1String( "0" ) );
  item2.setParentCollection( collection2 );
  item2.setPayload<KMime::Message::Ptr>( msgPtr );
  item2.setFlag( "\\SEEN" );

  job = mStore->modifyItem( item2 );
  // setting \SEEN, so indicate a flags change
  job->setParts( QSet<QByteArray>() << "FLAGS" );

  QVERIFY( job->exec() );
  QCOMPARE( job->error(), 0 );

  item = job->item();
  
  // returned item should contain the change
  QVERIFY( item.flags().contains( "\\SEEN" ) );
  
  // mbox not changed
  QVERIFY( mbox2.load( mbox2.fileName() ) );
  MBoxEntry::List entryList = mbox2.entries();
  QCOMPARE( (int)entryList.count(), 4 );

  var = job->property( "compactStore" );
  QVERIFY( !var.isValid() );
  
  // file not modified
  QVERIFY( file2.open( QIODevice::ReadOnly ) );
  cryptoHash.addData( file2.readAll() );
  QCOMPARE( cryptoHash.result(), mbox2Sha1 );
  
  file2.close();
  cryptoHash.reset();
  
  // check index preservation is not triggered
  var = job->property( "onDiskIndexInvalidated" );
  QVERIFY( !var.isValid() );
  
}


void ItemModifyTest::testModifyFlagsAndPayload()
{
  QDir topDir( mDir->name() );

  QVERIFY( TestDataUtil::installFolder( QLatin1String( "maildir" ), topDir.path(), QLatin1String( "collection1" ) ) );
  QVERIFY( TestDataUtil::installFolder( QLatin1String( "mbox" ), topDir.path(), QLatin1String( "collection2" ) ) );

  KPIM::Maildir topLevelMd( topDir.path(), true );

  KPIM::Maildir md1 = topLevelMd.subFolder( QLatin1String( "collection1" ) );
  QStringList entryList1 = md1.entryList();
  QCOMPARE( (int)entryList1.count(), 4 );

  QFileInfo fileInfo2( topDir.path(), QLatin1String( "collection2" ) );
  MBox mbox2;
  QVERIFY( mbox2.load( fileInfo2.absoluteFilePath() ) );
  MBoxEntry::List entryList2 = mbox2.entries();
  QCOMPARE( (int)entryList2.count(), 4 );
    
  mStore->setPath( topDir.path() );

  // common variables
  FileStore::ItemModifyJob *job = 0;

  const QVariant colListVar = QVariant::fromValue<Collection::List>( Collection::List() );
  QVariant var;
  Collection::List collections;
  KMime::Message::Ptr msgPtr( new KMime::Message );
  
  // test modifying a flag of a maildir items changes the entry name but not the
  // message contents
  Collection collection1;
  collection1.setName( QLatin1String( "collection1" ) );
  collection1.setRemoteId( QLatin1String( "collection1" ) );
  collection1.setParentCollection( mStore->topLevelCollection() );

  // check that the \SEEN flag is not set yet
  QVERIFY( !md1.readEntryFlags( entryList1.first() ).contains( "\\SEEN" ) );
  
  const QByteArray data1 = md1.readEntry( entryList1.first() );

  msgPtr->setContent( KMime::CRLFtoLF( data1 ) );
  msgPtr->subject()->from7BitString( "Modify Test" );
  msgPtr->assemble();

  Item item1;
  item1.setMimeType( KMime::Message::mimeType() );
  item1.setRemoteId( entryList1.first() );
  item1.setParentCollection( collection1 );
  item1.setPayload<KMime::Message::Ptr>( msgPtr );
  item1.setFlag( "\\SEEN" );

  job = mStore->modifyItem( item1 );
  // setting \SEEN so indicate a flags change and
  // setting new subject so indicate a payload change
  job->setParts( QSet<QByteArray>() << "FLAGS"
                                    << QByteArray( "PLD:" ) + MessagePart::Header );
    
  QVERIFY( job->exec() );
  QCOMPARE( job->error(), 0 );

  Item item = job->item();
  
  // returned item should contain the change
  QVERIFY( item.flags().contains( "\\SEEN" ) );
  
  // remote ID has changed
  QVERIFY( item.remoteId() != entryList1.first() );
  QVERIFY( !md1.entryList().contains( entryList1.first() ) );
  
  // no change in number of entries, one difference
  QStringList entryList3 = md1.entryList();
  QCOMPARE( entryList3.count(), entryList1.count() );
  Q_FOREACH( const QString &oldEntry, entryList1 ) {
    entryList3.removeAll( oldEntry );
  }
  QCOMPARE( entryList3.count(), 1 );
  
  // data changed
  QCOMPARE( md1.readEntry( entryList3.first() ), msgPtr->encodedContent() );

  var = job->property( "onDiskIndexInvalidated" );
  QVERIFY( var.isValid() );
  QCOMPARE( var.userType(), colListVar.userType() );

  collections = var.value<Collection::List>();
  QCOMPARE( (int)collections.count(), 1 );
  QCOMPARE( collections.first(), collection1 );
  
  // fetch new item, check flag
  item1 = Item();
  item1.setMimeType( KMime::Message::mimeType() );
  item1.setRemoteId( entryList3.first() );
  item1.setParentCollection( collection1 );
  
  FileStore::ItemFetchJob *itemFetch = mStore->fetchItem( item1 );
  itemFetch->fetchScope().fetchFullPayload();
  
  QVERIFY( itemFetch->exec() );
  QCOMPARE( itemFetch->error(), 0 );
  
  QCOMPARE( itemFetch->items().count(), 1 );
  Item fetchedItem = itemFetch->items().first();
  QEXPECT_FAIL( "", "ItemFetch handling needs to be fixed to also fetch flags", Continue );
  QVERIFY( fetchedItem.flags().contains( "\\SEEN" ) );
  
  QVERIFY( fetchedItem.hasPayload<KMime::Message::Ptr>() );
  KMime::Message::Ptr fetchedMsgPtr = fetchedItem.payload<KMime::Message::Ptr>();
  QCOMPARE( msgPtr->encodedContent(), fetchedMsgPtr->encodedContent() );
  
  
  // TODO test for mbox.
}

QTEST_KDEMAIN( ItemModifyTest, NoGUI )

#include "itemmodifytest.moc"

// kate: space-indent on; indent-width 2; replace-tabs on;