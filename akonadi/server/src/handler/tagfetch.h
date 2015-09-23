/***************************************************************************
 *   Copyright (C) 2014 by Daniel Vrátil <dvratil@redhat.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef AKONADIFETCHTAG_H
#define AKONADIFETCHTAG_H

#include "handler.h"
#include "scope.h"

namespace Akonadi {
namespace Server {

/**
  @ingroup akonadi_server_handler

  Handler for the FETCHTAG command.
 */
class TagFetch : public Handler
{
  Q_OBJECT
public:
    TagFetch( Scope::SelectionScope scope );
    ~TagFetch();

    bool parseStream();

  private:
    Scope mScope;
};

} // namespace Server
} // namespace Akonadi

#endif
