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

#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QApplication>

int main( int argc, char *argv[] )
{
  QApplication app( argc, argv );
  app.setQuitOnLastWindowClosed( false );

  if ( app.arguments().size() != 3 ) { // Expected usage: ./agent_launcher ${plugin_name} ${identifier}
    qDebug() << "Invalid usage: expected: ./agent_launcher pluginName agentIdentifier";
    return 1;
  }

  const QString agentPluginName = app.arguments().at( 1 );
  const QString agentIdentifier = app.arguments().at( 2 );

  AgentPluginLoader loader;
  QPluginLoader *factory = loader.load( agentPluginName );
  if ( factory == 0 ) {
    return 1;
  }

  QObject *instance = 0;
  const bool invokeSucceeded = QMetaObject::invokeMethod( factory->instance(),
                                                          "createInstance",
                                                          Qt::DirectConnection,
                                                          Q_RETURN_ARG( QObject*, instance ),
                                                          Q_ARG( QString, agentIdentifier ) );
  if ( invokeSucceeded ) {
    qDebug() << "Agent instance created in separate process.";
  } else {
    qDebug() << "Agent instance creation in separate process failed";
    return 2;
  }

  const int rv = app.exec();
  delete instance;
  return rv;
}
