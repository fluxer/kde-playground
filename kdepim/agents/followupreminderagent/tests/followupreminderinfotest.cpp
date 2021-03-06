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

#include "followupreminderinfotest.h"
#include "../followupreminderinfo.h"
#include <KConfigGroup>
#include <qtest_kde.h>

FollowUpReminderInfoTest::FollowUpReminderInfoTest(QObject *parent)
    : QObject(parent)
{
}

void FollowUpReminderInfoTest::shouldHaveDefaultValue()
{
    FollowUpReminder::FollowUpReminderInfo info;
    QCOMPARE(info.originalMessageItemId(), Akonadi::Item::Id(-1));
    QCOMPARE(info.messageId(), QString());
    QCOMPARE(info.isValid(), false);
    QCOMPARE(info.to(), QString());
    QCOMPARE(info.subject(), QString());
    QCOMPARE(info.uniqueIdentifier(), -1);
}

void FollowUpReminderInfoTest::shoudBeNotValid()
{
    FollowUpReminder::FollowUpReminderInfo info;
    //We need a messageId not empty and a valid date and a "To" not empty
    info.setMessageId(QLatin1String("foo"));
    QCOMPARE(info.isValid(), false);

    const QDate date(2014,1,1);
    info.setFollowUpReminderDate(QDate(date));
    QCOMPARE(info.isValid(), false);

    const QString to = QLatin1String("kde.org");
    info.setTo(to);
    QCOMPARE(info.isValid(), true);

    info.setOriginalMessageItemId(Akonadi::Item::Id(42));
    QCOMPARE(info.isValid(), true);
}

void FollowUpReminderInfoTest::shoudBeValidEvenIfSubjectIsEmpty()
{
    FollowUpReminder::FollowUpReminderInfo info;
    //We need a Akonadi::Id valid and a messageId not empty and a valid date and a "To" not empty
    info.setMessageId(QLatin1String("foo"));
    const QDate date(2014,1,1);
    info.setFollowUpReminderDate(QDate(date));
    const QString to = QLatin1String("kde.org");
    info.setTo(to);
    info.setOriginalMessageItemId(Akonadi::Item::Id(42));
    QCOMPARE(info.isValid(), true);
}

void FollowUpReminderInfoTest::shouldRestoreFromSettings()
{
    FollowUpReminder::FollowUpReminderInfo info;
    info.setMessageId(QLatin1String("foo"));
    const QDate date(2014,1,1);
    info.setFollowUpReminderDate(QDate(date));
    const QString to = QLatin1String("kde.org");
    info.setTo(to);
    info.setOriginalMessageItemId(Akonadi::Item::Id(42));
    info.setSubject(QLatin1String("Subject"));
    info.setUniqueIdentifier(42);
    info.setTodoId(52);

    KConfigGroup grp(KGlobal::config(), "testsettings");
    info.writeConfig(grp, info.uniqueIdentifier());

    FollowUpReminder::FollowUpReminderInfo restoreInfo(grp);
    QCOMPARE(info, restoreInfo);
}

void FollowUpReminderInfoTest::shouldCopyReminderInfo()
{
    FollowUpReminder::FollowUpReminderInfo info;
    info.setMessageId(QLatin1String("foo"));
    const QDate date(2014,1,1);
    info.setFollowUpReminderDate(QDate(date));
    const QString to = QLatin1String("kde.org");
    info.setTo(to);
    info.setOriginalMessageItemId(Akonadi::Item::Id(42));
    info.setSubject(QLatin1String("Subject"));
    info.setUniqueIdentifier(42);
    info.setTodoId(52);

    FollowUpReminder::FollowUpReminderInfo copyInfo(info);
    QCOMPARE(info, copyInfo);
}



QTEST_KDEMAIN(FollowUpReminderInfoTest, NoGUI)
