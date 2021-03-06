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

#ifndef FOLLOWUPREMINDERMANAGER_H
#define FOLLOWUPREMINDERMANAGER_H

#include <QObject>
#include <KSharedConfig>
#include <Akonadi/Item>
#include <QPointer>
namespace FollowUpReminder {
class FollowUpReminderInfo;
}
class FollowUpReminderNoAnswerDialog;
class FollowUpReminderManager : public QObject
{
    Q_OBJECT
public:
    explicit FollowUpReminderManager(QObject *parent = 0);
    ~FollowUpReminderManager();

    void load(bool forceReloadConfig = false);
    void checkFollowUp(const Akonadi::Item &item, const Akonadi::Collection &col);

    QString printDebugInfo();
private slots:
    void slotCheckFollowUpFinished(const QString &messageId, Akonadi::Item::Id id);

    void slotFinishTaskDone();
    void slotFinishTaskFailed();
private:
    void answerReceived(const QString &from);
    QString infoToStr(FollowUpReminder::FollowUpReminderInfo *info);

    KSharedConfig::Ptr mConfig;
    QList<FollowUpReminder::FollowUpReminderInfo*> mFollowUpReminderInfoList;
    QPointer<FollowUpReminderNoAnswerDialog> mNoAnswerDialog;
    bool mInitialize;
};

#endif // FOLLOWUPREMINDERMANAGER_H
