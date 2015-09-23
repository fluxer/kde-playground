/*
    Copyright (c) 2009 Volker Krause <vkrause@kde.org>

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

#ifndef AKONADI_REMOVE_H
#define AKONADI_REMOVE_H

#include "handler.h"
#include "scope.h"

namespace Akonadi {
namespace Server {

/**
  @ingroup akonadi_server_handler

  Handler for the item deletion command.

  <h4>Syntax</h4>
  One of the following three:
  @verbatim
  <tag> REMOVE <uid-set>
  <tag> UID REMOVE <uid-set>
  <tag> RID REMOVE <remote-identifier>
  @endverbatim

  <h4>Semantics</h4>
  Removes the selected items. Item selection can happen within the usual three scopes:
  - based on a uid set relative to the currently selected collection
  - based on a global uid set (UID)
  - based on a remote identifier within the currently selected collection (RID)
*/
class  Remove : public Handler
{
  Q_OBJECT
  public:
    Remove( Scope::SelectionScope scope );
    bool parseStream();

  private:
    Scope mScope;
};

} // namespace Server
} // namespace Akonadi

#endif
