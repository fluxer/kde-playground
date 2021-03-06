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


#include "followupreminderconfigtest.h"
#include "../followupreminderutil.h"
#include "../followupreminderinfo.h"
#include <qtest_kde.h>
#include <KSharedConfig>


FollowUpReminderConfigTest::FollowUpReminderConfigTest(QObject *parent)
    : QObject(parent)
{

}

FollowUpReminderConfigTest::~FollowUpReminderConfigTest()
{

}

void FollowUpReminderConfigTest::init()
{
    mConfig = KSharedConfig::openConfig(QLatin1String("test-followupreminder.rc"));
    mFollowupRegExpFilter = QRegExp( QLatin1String("FollowupReminderItem \\d+") );
    cleanup();
}

void FollowUpReminderConfigTest::cleanup()
{
    const QStringList filterGroups = mConfig->groupList();
    foreach ( const QString &group, filterGroups ) {
        mConfig->deleteGroup( group );
    }
    mConfig->sync();
    mConfig->reparseConfiguration();
}

void FollowUpReminderConfigTest::cleanupTestCase()
{
    //Make sure to clean config
    cleanup();
}

void FollowUpReminderConfigTest::shouldConfigBeEmpty()
{
    const QStringList filterGroups = mConfig->groupList();
    QCOMPARE(filterGroups.isEmpty(), true);
}

void FollowUpReminderConfigTest::shouldAddAnItem()
{
    FollowUpReminder::FollowUpReminderInfo info;
    info.setMessageId(QLatin1String("foo"));
    const QDate date(2014,1,1);
    info.setFollowUpReminderDate(QDate(date));
    const QString to = QLatin1String("kde.org");
    info.setTo(to);
    FollowUpReminder::FollowUpReminderUtil::writeFollowupReminderInfo(mConfig, &info, false);
    const QStringList itemList = mConfig->groupList().filter( mFollowupRegExpFilter );

    QCOMPARE(itemList.isEmpty(), false);
    QCOMPARE(itemList.count(), 1);
    QCOMPARE(mConfig->hasGroup(QLatin1String("General")), true);
}

void FollowUpReminderConfigTest::shouldNotAddAnInvalidItem()
{
    FollowUpReminder::FollowUpReminderInfo info;
    FollowUpReminder::FollowUpReminderUtil::writeFollowupReminderInfo(mConfig, &info, false);
    const QStringList itemList = mConfig->groupList().filter( mFollowupRegExpFilter );
    QCOMPARE(itemList.isEmpty(), true);
}

void FollowUpReminderConfigTest::shouldReplaceItem()
{
    FollowUpReminder::FollowUpReminderInfo info;
    info.setMessageId(QLatin1String("foo"));
    const QDate date(2014,1,1);
    info.setFollowUpReminderDate(QDate(date));
    const QString to = QLatin1String("kde.org");
    info.setTo(to);
    qint32 uniq = 42;
    info.setUniqueIdentifier(uniq);
    FollowUpReminder::FollowUpReminderUtil::writeFollowupReminderInfo(mConfig, &info, false);
    QStringList itemList = mConfig->groupList().filter( mFollowupRegExpFilter );

    QCOMPARE(itemList.count(), 1);

    info.setTo(QLatin1String("kmail.org"));
    FollowUpReminder::FollowUpReminderUtil::writeFollowupReminderInfo(mConfig, &info, false);
    itemList = mConfig->groupList().filter( mFollowupRegExpFilter );
    QCOMPARE(itemList.count(), 1);
}

void FollowUpReminderConfigTest::shouldAddSeveralItem()
{
    FollowUpReminder::FollowUpReminderInfo info;
    info.setMessageId(QLatin1String("foo"));
    const QDate date(2014,1,1);
    info.setFollowUpReminderDate(QDate(date));
    const QString to = QLatin1String("kde.org");
    info.setTo(to);
    qint32 uniq = 42;
    info.setUniqueIdentifier(uniq);
    FollowUpReminder::FollowUpReminderUtil::writeFollowupReminderInfo(mConfig, &info, false);
    QStringList itemList = mConfig->groupList().filter( mFollowupRegExpFilter );

    QCOMPARE(itemList.count(), 1);

    info.setTo(QLatin1String("kmail.org"));
    uniq = 43;
    info.setUniqueIdentifier(uniq);
    FollowUpReminder::FollowUpReminderUtil::writeFollowupReminderInfo(mConfig, &info, false);
    itemList = mConfig->groupList().filter( mFollowupRegExpFilter );
    QCOMPARE(itemList.count(), 2);

    uniq = 44;
    info.setUniqueIdentifier(uniq);
    FollowUpReminder::FollowUpReminderUtil::writeFollowupReminderInfo(mConfig, &info, false);
    itemList = mConfig->groupList().filter( mFollowupRegExpFilter );
    QCOMPARE(itemList.count(), 3);

    //Replace It

    info.setUniqueIdentifier(uniq);
    info.setTo(QLatin1String("kontact.org"));
    FollowUpReminder::FollowUpReminderUtil::writeFollowupReminderInfo(mConfig, &info, false);
    itemList = mConfig->groupList().filter( mFollowupRegExpFilter );
    QCOMPARE(itemList.count(), 3);

    // Add item without uniqIdentifier
    FollowUpReminder::FollowUpReminderInfo infoNotHaveUniq;
    infoNotHaveUniq.setMessageId(QLatin1String("foo"));
    infoNotHaveUniq.setFollowUpReminderDate(QDate(date));
    infoNotHaveUniq.setTo(to);

    FollowUpReminder::FollowUpReminderUtil::writeFollowupReminderInfo(mConfig, &infoNotHaveUniq, false);
    itemList = mConfig->groupList().filter( mFollowupRegExpFilter );
    QCOMPARE(itemList.count(), 4);
    QCOMPARE(infoNotHaveUniq.uniqueIdentifier(), 4);

}

void FollowUpReminderConfigTest::shouldRemoveItems()
{
    FollowUpReminder::FollowUpReminderInfo info;
    info.setMessageId(QLatin1String("foo"));
    const QDate date(2014,1,1);
    info.setFollowUpReminderDate(QDate(date));
    const QString to = QLatin1String("kde.org");
    info.setTo(to);
    qint32 uniq = 42;
    info.setUniqueIdentifier(uniq);
    FollowUpReminder::FollowUpReminderUtil::writeFollowupReminderInfo(mConfig, &info, false);
    QStringList itemList = mConfig->groupList().filter( mFollowupRegExpFilter );
    QCOMPARE(itemList.count(), 1);

    info.setTo(QLatin1String("kmail.org"));
    uniq = 43;
    info.setUniqueIdentifier(uniq);
    FollowUpReminder::FollowUpReminderUtil::writeFollowupReminderInfo(mConfig, &info, false);
    itemList = mConfig->groupList().filter( mFollowupRegExpFilter );

    uniq = 44;
    info.setUniqueIdentifier(uniq);
    FollowUpReminder::FollowUpReminderUtil::writeFollowupReminderInfo(mConfig, &info, false);
    itemList = mConfig->groupList().filter( mFollowupRegExpFilter );

    // Add item without uniqIdentifier
    FollowUpReminder::FollowUpReminderInfo infoNotHaveUniq;
    infoNotHaveUniq.setMessageId(QLatin1String("foo"));
    infoNotHaveUniq.setFollowUpReminderDate(QDate(date));
    infoNotHaveUniq.setTo(to);

    FollowUpReminder::FollowUpReminderUtil::writeFollowupReminderInfo(mConfig, &infoNotHaveUniq, false);
    itemList = mConfig->groupList().filter( mFollowupRegExpFilter );
    QCOMPARE(itemList.count(), 4);
    QCOMPARE(infoNotHaveUniq.uniqueIdentifier(), 3);

    QList<qint32> listRemove;
    listRemove<<43<<42;

    const bool elementRemoved = FollowUpReminder::FollowUpReminderUtil::removeFollowupReminderInfo(mConfig, listRemove);
    itemList = mConfig->groupList().filter( mFollowupRegExpFilter );
    QCOMPARE(itemList.count(), 2);
    QVERIFY(elementRemoved);
}

void FollowUpReminderConfigTest::shouldNotRemoveItemWhenListIsEmpty()
{
    QList<qint32> listRemove;
    const bool elementRemoved = FollowUpReminder::FollowUpReminderUtil::removeFollowupReminderInfo(mConfig, listRemove);
    QVERIFY(!elementRemoved);
}

void FollowUpReminderConfigTest::shouldNotRemoveItemWhenItemDoesntExist()
{
    FollowUpReminder::FollowUpReminderInfo info;
    info.setMessageId(QLatin1String("foo"));
    const QDate date(2014,1,1);
    info.setFollowUpReminderDate(QDate(date));
    const QString to = QLatin1String("kde.org");
    info.setTo(to);
    qint32 uniq = 42;
    info.setUniqueIdentifier(uniq);
    FollowUpReminder::FollowUpReminderUtil::writeFollowupReminderInfo(mConfig, &info, false);
    QStringList itemList = mConfig->groupList().filter( mFollowupRegExpFilter );

    info.setTo(QLatin1String("kmail.org"));
    uniq = 43;
    info.setUniqueIdentifier(uniq);
    FollowUpReminder::FollowUpReminderUtil::writeFollowupReminderInfo(mConfig, &info, false);
    itemList = mConfig->groupList().filter( mFollowupRegExpFilter );

    uniq = 44;
    info.setUniqueIdentifier(uniq);
    FollowUpReminder::FollowUpReminderUtil::writeFollowupReminderInfo(mConfig, &info, false);


    QList<qint32> listRemove;
    listRemove << 55 << 75;
    const bool elementRemoved = FollowUpReminder::FollowUpReminderUtil::removeFollowupReminderInfo(mConfig, listRemove);
    QVERIFY(!elementRemoved);
}



QTEST_KDEMAIN(FollowUpReminderConfigTest, NoGUI)
