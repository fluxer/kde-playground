/*
    Copyright (c) 2013 Christian Mollekopf <mollekopf@kolabsys.com>

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

#include <QObject>

#include "test_utils.h"

#include <KStandardDirs>
#include <KConfigGroup>
#include <akonadi/entitytreemodel.h>
#include <akonadi/control.h>
#include <akonadi/entitytreemodel_p.h>
#include <akonadi/monitor_p.h>
#include <akonadi/changerecorder_p.h>
#include <akonadi/qtest_akonadi.h>
#include <akonadi/collectioncreatejob.h>
#include <akonadi/itemcreatejob.h>
#include <favoritecollectionsmodel.h>
#include <QtGui/QItemSelectionRange>
#include <QSortFilterProxyModel>

using namespace Akonadi;

// Taken from Qt 5:
#if QT_VERSION < 0x050000

// Will try to wait for the expression to become true while allowing event processing
#define QTRY_VERIFY(__expr) \
do { \
  const int __step = 50; \
  const int __timeout = 5000; \
  if ( !( __expr ) ) { \
    QTest::qWait( 0 ); \
  } \
  for ( int __i = 0; __i < __timeout && !( __expr ); __i += __step ) { \
    QTest::qWait( __step ); \
  } \
  QVERIFY( __expr ); \
} while ( 0 )

// Will try to wait for the comparison to become successful while allowing event processing
#define QTRY_COMPARE(__expr, __expected) \
do { \
  const int __step = 50; \
  const int __timeout = 5000; \
  if ( ( __expr ) != ( __expected ) ) { \
    QTest::qWait( 0 ); \
  } \
  for ( int __i = 0; __i < __timeout && ( ( __expr ) != ( __expected ) ); __i += __step ) { \
    QTest::qWait( __step ); \
  } \
  QCOMPARE( __expr, __expected ); \
} while ( 0 )

#endif



class InspectableETM: public EntityTreeModel
{
public:
  explicit InspectableETM(ChangeRecorder* monitor, QObject* parent = 0)
  :EntityTreeModel(monitor, parent) {}
  EntityTreeModelPrivate *etmPrivate() { return d_ptr; };
  void reset() { EntityTreeModel::reset(); }
};

class FavoriteProxyTest : public QObject
{
  Q_OBJECT

private slots:
  void initTestCase();
  void testItemAdded();
  void testLoadConfig();
  void testInsertAfterModelCreation();
private:
  InspectableETM *createETM();
};

void FavoriteProxyTest::initTestCase()
{
  AkonadiTest::checkTestIsIsolated();
  Akonadi::Control::start();
  AkonadiTest::setAllResourcesOffline();
}


QModelIndex getIndex(const QString &string, EntityTreeModel *model)
{
  QModelIndexList list = model->match( model->index( 0, 0 ), Qt::DisplayRole, string, 1, Qt::MatchRecursive );
  if ( list.isEmpty() ) {
    return QModelIndex();
  }
  return list.first();
}

/**
 * Since we have no sensible way to figure out if the model is fully populated,
 * we use the brute force approach.
 */
bool waitForPopulation(const QModelIndex &idx, EntityTreeModel *model, int count)
{
  for (int i = 0; i < 500; i++) {
    if (model->rowCount(idx) >= count) {
      return true;
    }
    QTest::qWait(10);
  }
  return false;
}

InspectableETM *FavoriteProxyTest::createETM()
{
  ChangeRecorder *changeRecorder = new ChangeRecorder(this);
  changeRecorder->setCollectionMonitored(Collection::root());
  InspectableETM *model = new InspectableETM( changeRecorder, this );
  model->setItemPopulationStrategy(Akonadi::EntityTreeModel::LazyPopulation);
  return model;
}

/**
 * Tests that the item is being referenced when added to the favorite proxy, and dereferenced when removed.
 */
void FavoriteProxyTest::testItemAdded()
{
  Collection res3 = Collection( collectionIdFromPath( "res3" ) );

  InspectableETM *model = createETM();

  KConfigGroup configGroup( KGlobal::config(), "favoritecollectionsmodeltest" );

  FavoriteCollectionsModel *favoriteModel = new FavoriteCollectionsModel(model, configGroup, this);

  const int numberOfRootCollections = 4;
  //Wait for initial listing to complete
  QVERIFY(waitForPopulation(QModelIndex(), model, numberOfRootCollections));

  const QModelIndex res3Index = getIndex("res3", model);
  QVERIFY(res3Index.isValid());

  const Akonadi::Collection favoriteCollection = res3Index.data(EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
  QVERIFY(favoriteCollection.isValid());

  QVERIFY(!model->etmPrivate()->isMonitored(favoriteCollection.id()));

  //Ensure the collection is reference counted after being added to the favorite model
  {
    favoriteModel->addCollection(favoriteCollection);
    //the collection is in the favorites model
    QTRY_COMPARE(favoriteModel->rowCount(QModelIndex()), 1);
    QTRY_COMPARE(favoriteModel->data(favoriteModel->index(0, 0, QModelIndex()), EntityTreeModel::CollectionIdRole).value<Akonadi::Collection::Id>(), favoriteCollection.id());
    //the collection got referenced
    QTRY_VERIFY(model->etmPrivate()->isMonitored(favoriteCollection.id()));
    //the collection is not yet buffered though
    QTRY_VERIFY(!model->etmPrivate()->isBuffered(favoriteCollection.id()));
  }

  //Survive a reset
  {
    QSignalSpy resetSpy(model, SIGNAL(modelReset()));
    model->reset();
    QTRY_COMPARE(resetSpy.count(), 1);
    //the collection is in the favorites model
    QTRY_COMPARE(favoriteModel->rowCount(QModelIndex()), 1);
    QTRY_COMPARE(favoriteModel->data(favoriteModel->index(0, 0, QModelIndex()), EntityTreeModel::CollectionIdRole).value<Akonadi::Collection::Id>(), favoriteCollection.id());
    //the collection got referenced
    QTRY_VERIFY(model->etmPrivate()->isMonitored(favoriteCollection.id()));
    //the collection is not yet buffered though
    QTRY_VERIFY(!model->etmPrivate()->isBuffered(favoriteCollection.id()));
  }

  //Ensure the collection is no longer reference counted after being added to the favorite model, and moved to the buffer
  {
    favoriteModel->removeCollection(favoriteCollection);
    //moved from being reference counted to being buffered
    QTRY_VERIFY(model->etmPrivate()->isBuffered(favoriteCollection.id()));
    QTRY_COMPARE(favoriteModel->rowCount(QModelIndex()), 0);
  }
}

void FavoriteProxyTest::testLoadConfig()
{
  InspectableETM *model = createETM();

  const int numberOfRootCollections = 4;
  //Wait for initial listing to complete
  QVERIFY(waitForPopulation(QModelIndex(), model, numberOfRootCollections));
  const QModelIndex res3Index = getIndex("res3", model);
  QVERIFY(res3Index.isValid());
  const Akonadi::Collection favoriteCollection = res3Index.data(EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
  QVERIFY(favoriteCollection.isValid());

  KConfigGroup configGroup( KGlobal::config(), "favoritecollectionsmodeltest" );
  configGroup.writeEntry( "FavoriteCollectionIds", QList<Akonadi::Collection::Id>() << favoriteCollection.id() );
  configGroup.writeEntry( "FavoriteCollectionLabels", QStringList() << "label1" );

  FavoriteCollectionsModel *favoriteModel = new FavoriteCollectionsModel(model, configGroup, this);

  {
    QTRY_COMPARE(favoriteModel->rowCount(QModelIndex()), 1);
    QTRY_COMPARE(favoriteModel->data(favoriteModel->index(0, 0, QModelIndex()), EntityTreeModel::CollectionIdRole).value<Akonadi::Collection::Id>(), favoriteCollection.id());
    //the collection got referenced
    QTRY_VERIFY(model->etmPrivate()->isMonitored(favoriteCollection.id()));
  }
}

class Filter: public QSortFilterProxyModel
{
public:
  virtual bool filterAcceptsRow(int, const QModelIndex &) { return accepts; }
  bool accepts;
};

void FavoriteProxyTest::testInsertAfterModelCreation()
{
  InspectableETM *model = createETM();
  Filter filter;
  filter.accepts = false;
  filter.setSourceModel(model);

  const int numberOfRootCollections = 4;
  //Wait for initial listing to complete
  QVERIFY(waitForPopulation(QModelIndex(), model, numberOfRootCollections));
  const QModelIndex res3Index = getIndex("res3", model);
  QVERIFY(res3Index.isValid());
  const Akonadi::Collection favoriteCollection = res3Index.data(EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
  QVERIFY(favoriteCollection.isValid());

  KConfigGroup configGroup( KGlobal::config(), "favoritecollectionsmodeltest2" );

  FavoriteCollectionsModel *favoriteModel = new FavoriteCollectionsModel(&filter, configGroup, this);
  //The collection is not in the model yet
  favoriteModel->addCollection(favoriteCollection);
  filter.accepts = true;
  filter.invalidate();

  {
    QTRY_COMPARE(favoriteModel->rowCount(QModelIndex()), 1);
    QTRY_COMPARE(favoriteModel->data(favoriteModel->index(0, 0, QModelIndex()), EntityTreeModel::CollectionIdRole).value<Akonadi::Collection::Id>(), favoriteCollection.id());
    //the collection got referenced
    QTRY_VERIFY(model->etmPrivate()->isMonitored(favoriteCollection.id()));
  }
}

#include "favoriteproxytest.moc"

QTEST_AKONADIMAIN(FavoriteProxyTest, NoGUI)
