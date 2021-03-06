/*
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2009 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef BIRTHDAYSRESOURCE_H
#define BIRTHDAYSRESOURCE_H

#include <kcalcore/event.h>

#include <akonadi/resourcebase.h>

#include <QHash>

#include <QDate>

class BirthdaysResource : public Akonadi::ResourceBase, public Akonadi::AgentBase::Observer
{
  Q_OBJECT

  public:
    BirthdaysResource( const QString &id );
    ~BirthdaysResource();

  public Q_SLOTS:
    virtual void configure( WId windowId );

  protected:
    void retrieveCollections();
    void retrieveItems( const Akonadi::Collection &collection );
    bool retrieveItem( const Akonadi::Item &item, const QSet<QByteArray> &parts );

  private:
    void addPendingEvent( const KCalCore::Event::Ptr &event, const QString &remoteId );

    KCalCore::Event::Ptr createBirthday( const Akonadi::Item &contactItem );
    KCalCore::Event::Ptr createAnniversary( const Akonadi::Item &contactItem );
    KCalCore::Event::Ptr createEvent( const QDate &date );

  private slots:
    void doFullSearch();
    void listContacts( const Akonadi::Collection::List &cols );
    void createEvents( const Akonadi::Item::List &items );

    void contactChanged( const Akonadi::Item &item );
    void contactRemoved( const Akonadi::Item &item );

    void contactRetrieved( KJob *job );
  private:
    QHash<QString, Akonadi::Item> mPendingItems;
    QHash<QString, Akonadi::Item> mDeletedItems;
};

#endif
