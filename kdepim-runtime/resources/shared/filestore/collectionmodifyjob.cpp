/*  This file is part of the KDE project
    Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
    Author: Kevin Krammer, krake@kdab.com

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

#include "collectionmodifyjob.h"

#include "session_p.h"

using namespace Akonadi;

class FileStore::CollectionModifyJob::Private
{
  public:
    explicit Private( FileStore::CollectionModifyJob *parent )
      : mParent( parent )
    {
    }

  public:
    Collection mCollection;

  private:
    FileStore::CollectionModifyJob *mParent;
};

FileStore::CollectionModifyJob::CollectionModifyJob( const Collection &collection, FileStore::AbstractJobSession *session )
  : FileStore::Job( session ), d( new Private( this ) )
{
  Q_ASSERT( session != 0 );

  d->mCollection = collection;

  session->addJob( this );
}

FileStore::CollectionModifyJob::~CollectionModifyJob()
{
  delete d;
}

Collection FileStore::CollectionModifyJob::collection() const
{
  return d->mCollection;
}

bool FileStore::CollectionModifyJob::accept( FileStore::Job::Visitor *visitor )
{
  return visitor->visit( this );
}

void FileStore::CollectionModifyJob::handleCollectionModified( const Collection &collection )
{
  d->mCollection = collection;
}


// kate: space-indent on; indent-width 2; replace-tabs on;
