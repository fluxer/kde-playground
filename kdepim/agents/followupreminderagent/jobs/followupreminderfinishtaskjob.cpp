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


#include "followupreminderfinishtaskjob.h"
#include "../followupreminderinfo.h"
#include <Akonadi/Item>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemModifyJob>
#include <QDebug>
#include <KCalCore/Todo>


FollowUpReminderFinishTaskJob::FollowUpReminderFinishTaskJob(Akonadi::Item::Id id, QObject *parent)
    : QObject(parent),
      mTodoId(id)
{
}


FollowUpReminderFinishTaskJob::~FollowUpReminderFinishTaskJob()
{

}

void FollowUpReminderFinishTaskJob::start()
{
    if (mTodoId != -1) {
        closeTodo();
    } else {
        Q_EMIT finishTaskFailed();
        deleteLater();
    }
}

void FollowUpReminderFinishTaskJob::closeTodo()
{
    Akonadi::Item item(mTodoId);
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item, this );
    connect( job, SIGNAL(result(KJob*)), SLOT(slotItemFetchJobDone(KJob*)) );
}

void FollowUpReminderFinishTaskJob::slotItemFetchJobDone(KJob *job)
{
    if ( job->error() ) {
        qWarning() << job->errorString();
        Q_EMIT finishTaskFailed();
        deleteLater();
        return;
    }

    const Akonadi::Item::List lst = qobject_cast<Akonadi::ItemFetchJob*>( job )->items();
    if (lst.count() == 1) {
        const Akonadi::Item item = lst.first();
        if (!item.hasPayload<KCalCore::Todo::Ptr>()) {
            qDebug()<<" item is not a todo.";
            Q_EMIT finishTaskFailed();
            deleteLater();
            return;
        }
        KCalCore::Todo::Ptr todo = item.payload<KCalCore::Todo::Ptr>();
        todo->setCompleted(true);
        Akonadi::Item updateItem = item;
        updateItem.setPayload<KCalCore::Todo::Ptr>( todo );

        Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob( updateItem );
        connect( job, SIGNAL(result(KJob*)), SLOT(slotItemModifiedResult(KJob*)) );
    } else {
        qWarning()<<" Found item different from 1: "<<lst.count();
        Q_EMIT finishTaskFailed();
        deleteLater();
        return;
    }
}

void FollowUpReminderFinishTaskJob::slotItemModifiedResult(KJob *job)
{
    if ( job->error() ) {
        qWarning() << job->errorString();
        Q_EMIT finishTaskFailed();
    } else {
        Q_EMIT finishTaskDone();
    }
    deleteLater();
}
