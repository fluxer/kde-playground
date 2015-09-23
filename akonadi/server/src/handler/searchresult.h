/*
 * Copyright 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef AKONADI_SEARCHRESULT_H
#define AKONADI_SEARCHRESULT_H

#include <handler.h>

#include "scope.h"

namespace Akonadi {
namespace Server {

/**
  @ingroup akonadi_server_handler

  Handler for the search_result command

  @verbatim
  tag " " scope-selector " SEARCH_RESULT " searchId " " result
  scope-selector = [ "UID" / "RID" ]
  result = [ scope / "DONE" ]
  @endverbatim
*/
class SearchResult : public Handler
{
    Q_OBJECT
  public:
    SearchResult( Scope::SelectionScope scope );
    ~SearchResult();

    bool parseStream();

  private:
    void fail( const QByteArray &searchId, const char *error );

    Scope mScope;
};

} // namespace Server
} // namespace Akonadi

#endif // AKONADI_SEARCHRESULT_H
