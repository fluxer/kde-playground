/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "searchpotentialduplicatecontactjobtest.h"
#include "../searchpotentialduplicatecontactjob.h"

#include <Akonadi/Item>
#include <kabc/addressee.h>
#include <QList>
#include <qtest_kde.h>

using namespace KABMergeContacts;
SearchPotentialDuplicateContactJobTest::SearchPotentialDuplicateContactJobTest()
{
    qRegisterMetaType<QList<Akonadi::Item> >();
    qRegisterMetaType<QList<Akonadi::Item::List> >();
}

void SearchPotentialDuplicateContactJobTest::shouldReturnEmptyListWhenNoItem()
{
    Akonadi::Item::List lst;
    SearchPotentialDuplicateContactJob job(lst);
    QSignalSpy spy(&job, SIGNAL(finished(QList<Akonadi::Item::List>)));
    job.start();
    QCOMPARE(spy.count(), 1);
    QList<Akonadi::Item::List> lstResult = spy.at(0).at(0).value< QList<Akonadi::Item::List> >();
    QCOMPARE(lstResult.count(), 0);
}

void SearchPotentialDuplicateContactJobTest::shouldReturnEmptyListWhenOneItem()
{
    Akonadi::Item::List lst;
    lst << Akonadi::Item(42);
    SearchPotentialDuplicateContactJob job(lst);
    QSignalSpy spy(&job, SIGNAL(finished(QList<Akonadi::Item::List>)));
    job.start();
    QCOMPARE(spy.count(), 1);
    QList<Akonadi::Item::List> lstResult = spy.at(0).at(0).value< QList<Akonadi::Item::List> >();
    QCOMPARE(lstResult.count(), 0);
}

void SearchPotentialDuplicateContactJobTest::shouldReturnListWhenTwoItemsAreDuplicated()
{
    Akonadi::Item::List lst;
    Akonadi::Item itemA;
    KABC::Addressee address;
    address.setName(QLatin1String("foo1"));
    itemA.setPayload<KABC::Addressee>( address );
    itemA.setMimeType( KABC::Addressee::mimeType() );

    lst << itemA << itemA;

    SearchPotentialDuplicateContactJob job(lst);
    QSignalSpy spy(&job, SIGNAL(finished(QList<Akonadi::Item::List>)));
    job.start();
    QCOMPARE(spy.count(), 1);
    QList<Akonadi::Item::List> lstResult = spy.at(0).at(0).value< QList<Akonadi::Item::List> >();
    QCOMPARE(lstResult.count(), 1);
}

QTEST_KDEMAIN(SearchPotentialDuplicateContactJobTest, NoGUI)
