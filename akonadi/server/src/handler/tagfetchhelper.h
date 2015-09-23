/*
    Copyright (c) 2014 Daniel Vrátil <dvratil@redhat.com>

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

#ifndef AKONADI_TAGFETCHHELPER_H
#define AKONADI_TAGFETCHHELPER_H

#include <QtCore/QObject>
#include <QtSql/QSqlQuery>

#include "libs/imapset_p.h"

namespace Akonadi {

class ImapSet;

namespace Server {

class ImapStreamParser;

class Connection;
class Response;

class TagFetchHelper : public QObject
{
  Q_OBJECT

  public:
    TagFetchHelper( Connection *connection, const ImapSet &set );

    bool fetchTags( const QByteArray &responseIdentifier );

    static QList<QByteArray> fetchTagAttributes( qint64 tagId );
    static QByteArray tagToByteArray( qint64 tagId, const QByteArray &gid, qint64 parentId, const QByteArray &type, const QByteArray &remoteId, const QList<QByteArray> &tagAttributes );

  Q_SIGNALS:
    void responseAvailable( const Akonadi::Server::Response &response );

  private:
    QSqlQuery buildTagQuery();
    QSqlQuery buildAttributeQuery() const;
    static QSqlQuery buildAttributeQuery( qint64 id );

  private:
    ImapStreamParser *mStreamParser;

    Connection *mConnection;
    ImapSet mSet;
};

} // namespace Server
} // namespace Akonadi

#endif // AKONADI_TAGFETCHHELPER_H
