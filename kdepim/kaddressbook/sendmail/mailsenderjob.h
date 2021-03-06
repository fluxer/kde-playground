/*
  This file is part of KAddressBook.

  Copyright (c) 2014 Laurent Montel <montel@kde.org>
  based on code from Copyright (c) 2014 Clément Vannier <clement.vannier@free.fr>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef MAILSENDERJOB_H
#define MAILSENDERJOB_H

#include "kaddressbook_export.h"
#include <Akonadi/Item>
#include <QObject>
#include <QStringList>

#include <QItemSelectionModel>
class KJob;
namespace KABMailSender {
class KADDRESSBOOK_EXPORT MailSenderJob : public QObject
{
    Q_OBJECT

public:
    explicit MailSenderJob(const Akonadi::Item::List &listItem, QObject* parent = 0);
    ~MailSenderJob();

    void start();

Q_SIGNALS:
    void sendMails(const QStringList &emails);

private Q_SLOTS:
    void fetchJobFinished(KJob* job);

private:
    void finishJob();
    void fetchItem(const Akonadi::Item &item);
    void fetchNextItem();
    Akonadi::Item::List mListItem;
    Akonadi::Item::List mItemToFetch;
    QStringList mEmailAddresses;
    int mFetchJobCount;
};
}

#endif
