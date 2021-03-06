/*
    This file is part of oxaccess.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

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

#ifndef OXA_INCIDENCEUTILS_H
#define OXA_INCIDENCEUTILS_H

#include "object.h"

#include <QDomDocument>
#include <QDomElement>

namespace OXA {

/**
 * Namespace that contains helper methods for handling events and tasks.
 */
namespace IncidenceUtils
{
  /**
   * Parses the XML tree under @p propElement and fills the event data of @p object.
   */
  void parseEvent( const QDomElement &propElement, Object &object );

  /**
   * Parses the XML tree under @p propElement and fills the task data of @p object.
   */
  void parseTask( const QDomElement &propElement, Object &object );

  /**
   * Adds the event data of @p object to the @p document under the @p propElement.
   */
  void addEventElements( QDomDocument &document, QDomElement &propElement, const Object &object );

  /**
   * Adds the task data of @p object to the @p document under the @p propElement.
   */
  void addTaskElements( QDomDocument &document, QDomElement &propElement, const Object &object );
}

}

#endif
