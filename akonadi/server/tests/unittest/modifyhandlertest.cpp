/*
    Copyright (c) 2014 Christian Mollekopf <mollekopf@kolabsys.com>

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
#include <handler/modify.h>
#include <imapstreamparser.h>
#include <response.h>
#include <storage/entity.h>

#include "fakeakonadiserver.h"
#include "aktest.h"
#include "akdebug.h"
#include "entities.h"

#include <QtTest/QTest>

using namespace Akonadi;
using namespace Akonadi::Server;

Q_DECLARE_METATYPE(QList<Akonadi::NotificationMessageV3>)

class ModifyHandlerTest : public QObject
{
    Q_OBJECT

public:
    ModifyHandlerTest()
    {
        qRegisterMetaType<Akonadi::Server::Response>();

        try {
            FakeAkonadiServer::instance()->init();
        } catch (const FakeAkonadiServerException &e) {
            akError() << "Server exception: " << e.what();
            akFatal() << "Fake Akonadi Server failed to start up, aborting test";
        }
    }

    ~ModifyHandlerTest()
    {
        FakeAkonadiServer::instance()->quit();
    }


private Q_SLOTS:
    void testModify_data()
    {
        QTest::addColumn<QList<QByteArray> >("scenario");
        QTest::addColumn<QList<Akonadi::NotificationMessageV3> >("expectedNotifications");
        QTest::addColumn<QVariant>("newValue");

        Akonadi::NotificationMessageV3 notificationTemplate;
        notificationTemplate.setType(NotificationMessageV2::Collections);
        notificationTemplate.setOperation(NotificationMessageV2::Modify);
        notificationTemplate.addEntity(5, QLatin1String("ColD"), QLatin1String(""));
        notificationTemplate.setParentCollection(4);
        notificationTemplate.setResource("akonadi_fake_resource_0");
        notificationTemplate.setSessionId(FakeAkonadiServer::instanceName().toLatin1());

        {
            QList<QByteArray> scenario;
            scenario << FakeAkonadiServer::defaultScenario()
                    << "C: 2 MODIFY 5 NAME \"New Name\""
                    << "S: 2 OK MODIFY done";

            Akonadi::NotificationMessageV3 notification = notificationTemplate;
            notification.setItemParts(QSet<QByteArray>() << "NAME");

            QTest::newRow("modify collection") << scenario << (QList<Akonadi::NotificationMessageV3>() << notification) << QVariant::fromValue(QString::fromLatin1("New Name"));
        }
        {
            QList<QByteArray> scenario;
            scenario << FakeAkonadiServer::defaultScenario()
                    << "C: 2 MODIFY 5 ENABLED FALSE SYNC DEFAULT DISPLAY DEFAULT INDEX DEFAULT"
                    << "S: 2 OK MODIFY done";

            Akonadi::NotificationMessageV3 notification = notificationTemplate;
            notification.setItemParts(QSet<QByteArray>() << "ENABLED");
            Akonadi::NotificationMessageV3 unsubscribeNotification = notificationTemplate;
            unsubscribeNotification.setOperation(NotificationMessageV2::Unsubscribe);

            QTest::newRow("disable collection") << scenario <<  (QList<Akonadi::NotificationMessageV3>() << notification << unsubscribeNotification) << QVariant::fromValue(false);
        }
        {
            QList<QByteArray> scenario;
            scenario << FakeAkonadiServer::defaultScenario()
                    << "C: 2 MODIFY 5 ENABLED TRUE SYNC DEFAULT DISPLAY DEFAULT INDEX DEFAULT"
                    << "S: 2 OK MODIFY done";

            Akonadi::NotificationMessageV3 notification = notificationTemplate;
            notification.setItemParts(QSet<QByteArray>() << "ENABLED");
            Akonadi::NotificationMessageV3 subscribeNotification = notificationTemplate;
            subscribeNotification.setOperation(NotificationMessageV2::Subscribe);

            QTest::newRow("enable collection") << scenario << (QList<Akonadi::NotificationMessageV3>() << notification << subscribeNotification) << QVariant::fromValue(true);
        }
        {
            QList<QByteArray> scenario;
            scenario << FakeAkonadiServer::defaultScenario()
                    << "C: 2 MODIFY 5 ENABLED FALSE SYNC TRUE DISPLAY TRUE INDEX TRUE"
                    << "S: 2 OK MODIFY done";

            Akonadi::NotificationMessageV3 notification = notificationTemplate;
            notification.setItemParts(QSet<QByteArray>() << "ENABLED" << "SYNC" << "DISPLAY" << "INDEX");
            Akonadi::NotificationMessageV3 unsubscribeNotification = notificationTemplate;
            unsubscribeNotification.setOperation(NotificationMessageV2::Unsubscribe);

            QTest::newRow("local override enable") << scenario << (QList<Akonadi::NotificationMessageV3>() << notification << unsubscribeNotification) << QVariant::fromValue(true);
        }
    }

    void testModify()
    {
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(QList<NotificationMessageV3>, expectedNotifications);
        QFETCH(QVariant, newValue);

        FakeAkonadiServer::instance()->setScenario(scenario);
        FakeAkonadiServer::instance()->runTest();

        QSignalSpy *notificationSpy = FakeAkonadiServer::instance()->notificationSpy();
        if (expectedNotifications.isEmpty()) {
            QVERIFY(notificationSpy->isEmpty() || notificationSpy->takeFirst().first().value<NotificationMessageV3::List>().isEmpty());
            return;
        }
        QCOMPARE(notificationSpy->count(), 1);
        //Only one notify call
        QCOMPARE(notificationSpy->first().count(), 1);
        const NotificationMessageV3::List receivedNotifications = notificationSpy->first().first().value<NotificationMessageV3::List>();
        QCOMPARE(receivedNotifications.size(), expectedNotifications.count());

        for (int i = 0; i < expectedNotifications.size(); i++) {
            QCOMPARE(receivedNotifications.at(i), expectedNotifications.at(i));
            NotificationMessageV3 notification = receivedNotifications.at(i);
            Q_FOREACH (const NotificationMessageV2::Entity &entity, notification.entities()) {
                if (notification.itemParts().contains("NAME")) {
                    Collection col = Collection::retrieveById( entity.id );
                    QCOMPARE(col.name(), newValue.toString());
                }
                if (notification.itemParts().contains("ENABLED") || notification.itemParts().contains("SYNC") || notification.itemParts().contains("DISPLAY") || notification.itemParts().contains("INDEX")) {
                    Collection col = Collection::retrieveById( entity.id );
                    const bool sync = col.syncPref() == Tristate::Undefined ? col.enabled() : col.syncPref() == Tristate::True;
                    QCOMPARE(sync, newValue.toBool());
                    const bool display = col.displayPref() == Tristate::Undefined ? col.enabled() : col.displayPref() == Tristate::True;
                    QCOMPARE(display, newValue.toBool());
                    const bool index = col.indexPref() == Tristate::Undefined ? col.enabled() : col.indexPref() == Tristate::True;
                    QCOMPARE(index, newValue.toBool());
                }
            }
        }
    }

};

AKTEST_FAKESERVER_MAIN(ModifyHandlerTest)

#include "modifyhandlertest.moc"
