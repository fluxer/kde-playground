/*
  Copyright (c) 2010 Kevin Funk <kevin.funk@kdab.com>

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

#include "networkaccesshelper.h"

using namespace KPIMUtils;

/**
 * Fake backend
 */
NetworkAccessHelper::NetworkAccessHelper( QObject *parent )
  : QObject( parent ), d_ptr(0)
{
}

NetworkAccessHelper::~NetworkAccessHelper()
{
}

void NetworkAccessHelper::establishConnection()
{
}

void NetworkAccessHelper::releaseConnection()
{
}

#include "moc_networkaccesshelper.cpp"
