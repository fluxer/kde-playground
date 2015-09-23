/*
 * Copyright (C) 2014  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef COMMANDCONTEXT_H
#define COMMANDCONTEXT_H

#include "entities.h"

namespace Akonadi {
namespace Server {

class ImapStreamParser;

class CommandContext
{
  public:
    CommandContext();
    ~CommandContext();

    void setResource( const Resource &resource );
    Resource resource() const;

    void setCollection( const Collection &collection );
    qint64 collectionId() const;
    Collection collection() const;

    void setTag( qint64 tagId );
    qint64 tagId() const;
    Tag tag() const;

    bool isEmpty() const;

    void parseContext( ImapStreamParser *parser );

  private:
    Resource mResource;
    Collection mCollection;
    qint64 mTagId;
};

}

}

#endif // COMMANDCONTEXT_H
