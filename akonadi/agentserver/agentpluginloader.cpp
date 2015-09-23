/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>

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
#include "agentpluginloader.h"

#include "libs/xdgbasedirs_p.h"
#include "shared/akdebug.h"

using namespace Akonadi;

AgentPluginLoader::~AgentPluginLoader()
{
  qDeleteAll( m_pluginLoaders );
  m_pluginLoaders.clear();
}

QPluginLoader *AgentPluginLoader::load( const QString &pluginName )
{
  const QString pluginFile = XdgBaseDirs::findPluginFile( pluginName );
  if ( pluginFile.isEmpty() ) {
    akError() << Q_FUNC_INFO << "plugin file:" << pluginName << "not found!";
    return 0;
  }

  if ( m_pluginLoaders.contains( pluginFile ) ) {
    return m_pluginLoaders.value( pluginFile );
  } else {
    QPluginLoader *loader = new QPluginLoader( pluginFile );
    if ( !loader->load() ) {
      akError() << Q_FUNC_INFO << "Failed to load agent: " << loader->errorString();
      delete loader;
      return 0;
    }
    m_pluginLoaders.insert( pluginFile, loader );
    return loader;
  }
}
