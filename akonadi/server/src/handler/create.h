/***************************************************************************
 *   Copyright (C) 2006 by Ingo Kloecker <kloecker@kde.org>                *
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
#ifndef AKONADICREATE_H
#define AKONADICREATE_H

#include <handler.h>
#include "scope.h"

namespace Akonadi {
namespace Server {


/**
  @ingroup akonadi_server_handler

  Handler for the CREATE command. CREATE is backward-compatible with RFC 3051,
  except recursive collection creation.

  <h4>Syntax</h4>

  Request:
  @verbatim
  tag [" RID"] " CREATE " collection-name " " parent-collection " (" attribute-list ")"
  @endverbatim

  @c attribute-list is the same as defined in AkList.
  @c parent-collection is either a collection UID or a collection RID, depending on the command prefix

  Response:
  A untagged response identical to AkList is sent for every created collection.
 */
class Create : public Handler
{
  Q_OBJECT
public:
    Create( Scope::SelectionScope scope );

    bool parseStream();

  private:
    Scope::SelectionScope m_scope;
};

} // namespace Server
} // namespace Akonadi

#endif
