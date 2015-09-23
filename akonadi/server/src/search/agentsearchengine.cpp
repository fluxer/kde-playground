/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

#include "agentsearchengine.h"
#include <akdbus.h>
#include <akdebug.h>
#include "entities.h"

#include "libs/protocol_p.h"

#include <QDBusInterface>
#include <QDBusError>

using namespace Akonadi;
using namespace Akonadi::Server;

void AgentSearchEngine::addSearch( const Collection &collection )
{
  QDBusInterface agentMgr( AkDBus::serviceName( AkDBus::Control ),
                           QLatin1String( AKONADI_DBUS_AGENTMANAGER_PATH ),
                           QLatin1String( "org.freedesktop.Akonadi.AgentManagerInternal" ) );
  if ( agentMgr.isValid() ) {
    QList<QVariant> args;
    args << collection.queryString()
         << QLatin1String("")
         << collection.id();
    agentMgr.callWithArgumentList( QDBus::NoBlock, QLatin1String( "addSearch" ), args );
    return;
  }

  akError() << "Failed to connect to agent manager: " << agentMgr.lastError().message();
}

void AgentSearchEngine::removeSearch( qint64 id )
{
  QDBusInterface agentMgr( AkDBus::serviceName( AkDBus::Control ),
                            QLatin1String( AKONADI_DBUS_AGENTMANAGER_PATH ),
                            QLatin1String( "org.freedesktop.Akonadi.AgentManagerInternal" ) );
  if ( agentMgr.isValid() ) {
    QList<QVariant> args;
    args << id;
    agentMgr.callWithArgumentList( QDBus::NoBlock, QLatin1String( "removeSearch" ), args );
    return;
  }

  akError() << "Failed to connect to agent manager: " << agentMgr.lastError().message();
}
