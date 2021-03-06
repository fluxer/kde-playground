/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "sendlatermanager.h"
#include "sendlaterinfo.h"
#include "sendlaterutil.h"
#include "sendlaterjob.h"

#include "messagecomposer/sender/akonadisender.h"
#include "messagecomposer/utils/util.h"

#include <KSharedConfig>
#include <KConfigGroup>
#include <KGlobal>
#include <KMessageBox>
#include <KLocale>

#include <QStringList>
#include <QTimer>

SendLaterManager::SendLaterManager(QObject *parent)
    : QObject(parent),
      mCurrentInfo(0),
      mCurrentJob(0),
      mSender(new MessageComposer::AkonadiSender)
{
    mConfig = KGlobal::config();
    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(slotCreateJob()));
}

SendLaterManager::~SendLaterManager()
{
    stopAll();
    delete mSender;
}

void SendLaterManager::stopAll()
{
    stopTimer();
    qDeleteAll(mListSendLaterInfo);
    mListSendLaterInfo.clear();
    mCurrentJob = 0;
}

void SendLaterManager::load(bool forcereload)
{
    stopAll();
    if (forcereload)
        mConfig->reparseConfiguration();

    const QStringList itemList = mConfig->groupList().filter( QRegExp( QLatin1String("SendLaterItem \\d+") ) );
    const int numberOfItems = itemList.count();
    for (int i = 0 ; i < numberOfItems; ++i) {
        KConfigGroup group = mConfig->group(itemList.at(i));
        SendLater::SendLaterInfo *info = new SendLater::SendLaterInfo(group);
        if (info->isValid()) {
            mListSendLaterInfo.append(info);
        } else {
            delete info;
        }
    }
    createSendInfoList();
}

void SendLaterManager::createSendInfoList()
{
    mCurrentInfo = 0;
    qSort(mListSendLaterInfo.begin(), mListSendLaterInfo.end(), SendLater::SendLaterUtil::compareSendLaterInfo);

    //Look at QQueue
    if (mSendLaterQueue.isEmpty()) {
        if (!mListSendLaterInfo.isEmpty()) {
            mCurrentInfo = mListSendLaterInfo.first();
            const QDateTime now = QDateTime::currentDateTime();
            const int seconds = now.secsTo(mCurrentInfo->dateTime());
            if (seconds > 0) {
                //qDebug()<<" seconds"<<seconds;
                mTimer->start(seconds*1000);
            } else {
                //Create job when seconds <0
                slotCreateJob();
            }
        } else {
            mTimer->stop();
            qDebug()<<" list is empty";
        }
    } else {
        SendLater::SendLaterInfo *info = searchInfo(mSendLaterQueue.dequeue());
        if (info) {
            mCurrentInfo = info;
            slotCreateJob();
        } else { //If removed.
            createSendInfoList();
        }
    }
}

void SendLaterManager::stopTimer()
{
    if (mTimer->isActive())
        mTimer->stop();
}

SendLater::SendLaterInfo *SendLaterManager::searchInfo(Akonadi::Item::Id id)
{
    Q_FOREACH(SendLater::SendLaterInfo *info, mListSendLaterInfo) {
        if (info->itemId() == id) {
            return info;
        }
    }
    return 0;
}

void SendLaterManager::sendNow(Akonadi::Item::Id id)
{
    if (!mCurrentJob) {
        SendLater::SendLaterInfo *info = searchInfo(id);
        if (info) {
            mCurrentInfo = info;
            slotCreateJob();
        } else {
            kDebug()<<" can't find info about current id: "<<id;
            itemRemoved(id);
        }
    } else {
        //Add to QQueue
        mSendLaterQueue.enqueue(id);
    }
}

void SendLaterManager::slotCreateJob()
{
    if (mCurrentJob) {
        kDebug()<<" Problem we have already a job"<<mCurrentJob;
        return;
    }
    mCurrentJob = new SendLaterJob(this, mCurrentInfo);
    mCurrentJob->start();
}

void SendLaterManager::itemRemoved(Akonadi::Item::Id id)
{
    if (mConfig->hasGroup(SendLater::SendLaterUtil::sendLaterPattern.arg(id))) {
        removeInfo(id);
        mConfig->reparseConfiguration();
        Q_EMIT needUpdateConfigDialogBox();
    }
}

void SendLaterManager::removeInfo(Akonadi::Item::Id id)
{
    KConfigGroup group = mConfig->group(SendLater::SendLaterUtil::sendLaterPattern.arg(id));
    group.deleteGroup();
    group.sync();
}

void SendLaterManager::sendError(SendLater::SendLaterInfo *info, ErrorType type)
{
    if (info) {
        switch(type) {
        case UnknownError:
        case ItemNotFound:
            //Don't try to resend it. Remove it.
            removeLaterInfo(info);
            break;
        case MailDispatchDoesntWork:
            //Force to make online maildispatcher
            //Don't remove it.
            MessageComposer::Util::sendMailDispatcherIsOnline( 0 );
            //Remove item which create error ?
            if (!info->isRecurrence()) {
                removeLaterInfo(info);
            }
            break;
        default:
            if (KMessageBox::No == KMessageBox::questionYesNo(0, i18n("An error was found. Do you want to resend it?"), i18n("Error found"))) {
                removeLaterInfo(info);
            }
            break;
        }
    }
    recreateSendList();
}

void SendLaterManager::recreateSendList()
{
    mCurrentJob = 0;
    Q_EMIT needUpdateConfigDialogBox();
    QTimer::singleShot(1000*60, this, SLOT(createSendInfoList()));
}

void SendLaterManager::sendDone(SendLater::SendLaterInfo *info)
{
    if (info) {
        if (info->isRecurrence()) {
            SendLater::SendLaterUtil::changeRecurrentDate(info);
        } else {
            removeLaterInfo(info);
        }
    }
    recreateSendList();
}

void SendLaterManager::removeLaterInfo(SendLater::SendLaterInfo *info)
{
    mListSendLaterInfo.removeAll(mCurrentInfo);
    removeInfo(info->itemId());
}

QString SendLaterManager::printDebugInfo()
{
    QString infoStr;
    if (mListSendLaterInfo.isEmpty()) {
        infoStr = QLatin1String("No mail");
    } else {
        Q_FOREACH (SendLater::SendLaterInfo *info, mListSendLaterInfo) {
            if (!infoStr.isEmpty())
                infoStr += QLatin1Char('\n');
            infoStr += infoToStr(info);
        }
    }
    return infoStr;
}

QString SendLaterManager::infoToStr(SendLater::SendLaterInfo *info)
{
    QString infoStr = QLatin1String("Recusive ") + (info->isRecurrence() ? QLatin1String("true") : QLatin1String("false"));
    infoStr += QLatin1String("Item id :") + QString::number(info->itemId());
    infoStr += QLatin1String("Send date:") + info->dateTime().toString();
    infoStr += QLatin1String("Last saved date: ") + info->lastDateTimeSend().toString();
    infoStr += QLatin1String("Mail subject :") + info->subject();
    infoStr += QLatin1String("To: ") + info->to();
    return infoStr;
}

MessageComposer::AkonadiSender *SendLaterManager::sender() const
{
    return mSender;
}

