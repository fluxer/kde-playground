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


#include "searchpotentialduplicatecontactjob.h"

#include <KABC/Addressee>

using namespace KABMergeContacts;

SearchPotentialDuplicateContactJob::SearchPotentialDuplicateContactJob(const Akonadi::Item::List &list, QObject *parent)
    : QObject(parent),
      mListItem(list)
{
}

SearchPotentialDuplicateContactJob::~SearchPotentialDuplicateContactJob()
{

}

void SearchPotentialDuplicateContactJob::start()
{
    mListDuplicate.clear();
    Akonadi::Item::List result = mListItem;
    while(!result.isEmpty()) {
        result = checkList(result);
    }
    Q_EMIT finished(mListDuplicate);
    deleteLater();
}

QList<Akonadi::Item::List > SearchPotentialDuplicateContactJob::potentialDuplicateContacts() const
{
    return mListDuplicate;
}

Akonadi::Item::List SearchPotentialDuplicateContactJob::checkList(const Akonadi::Item::List &lstItem)
{
    QList<Akonadi::Item> notDuplicate;
    QList<Akonadi::Item> lst;
    if (!lstItem.isEmpty()) {
        Akonadi::Item firstItem = lstItem.at(0);
        const int numberOfItems(lstItem.count());
        for (int j = 1; j < numberOfItems; ++j) {
            if (isDuplicate(firstItem, mListItem.at(j))) {
                lst.append(lstItem.at(j));
            } else {
                notDuplicate.append(lstItem.at(j));
            }
        }
        if (!lst.isEmpty()) {
            lst.append(firstItem);
            mListDuplicate.append(lst);
        }
        qDebug()<<" duplicate number "<<lst.count();
    }
    return notDuplicate;
}

bool SearchPotentialDuplicateContactJob::isDuplicate(const Akonadi::Item &itemA, const Akonadi::Item &itemB)
{
    if (!itemA.hasPayload<KABC::Addressee>() || !itemB.hasPayload<KABC::Addressee>()) {
        return false;
    }

    KABC::Addressee addressA = itemA.payload<KABC::Addressee>();
    KABC::Addressee addressB = itemB.payload<KABC::Addressee>();
    //
    if (!addressA.name().isEmpty() && !addressB.name().isEmpty()) {
        //qDebug()<<" addressB"<<addressB.name()<<" addressA.name()"<<addressA.name();
        if (addressA.name() == addressB.name()) {
            //qDebug()<<" return true;";
            return true;
        }
    }
    if (!addressA.nickName().isEmpty() && !addressB.nickName().isEmpty()) {
        if (addressA.nickName() == addressB.nickName()) {
            return true;
        }
    }
    if (!addressA.emails().isEmpty() && !addressB.emails().isEmpty()) {
        Q_FOREACH (const QString &email, addressA.emails()) {
            if (addressB.emails().contains(email)) {
                return true;
            }
        }
    }
    return false;
}
