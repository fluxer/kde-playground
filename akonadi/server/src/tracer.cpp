/***************************************************************************
 *   Copyright (C) 2006 by Tobias Koenig <tokoe@kde.org>                   *
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
#include "tracer.h"

#include <QtCore/QSettings>
#include <QtCore/QString>

#include "traceradaptor.h"

#include "dbustracer.h"
#include "filetracer.h"
#include "nulltracer.h"
#include <libs/xdgbasedirs_p.h>
#include <akstandarddirs.h>

// #define DEFAULT_TRACER QLatin1String( "dbus" )
#define DEFAULT_TRACER QLatin1String( "null" )

using namespace Akonadi::Server;

Tracer *Tracer::mSelf = 0;

Tracer::Tracer()
  : mTracerBackend( 0 )
{
  activateTracer( currentTracer() );

  new TracerAdaptor( this );

  QDBusConnection::sessionBus().registerObject( QLatin1String( "/tracing" ), this, QDBusConnection::ExportAdaptors );
}

Tracer::~Tracer()
{
  delete mTracerBackend;
  mTracerBackend = 0;
}

Tracer *Tracer::self()
{
  if ( !mSelf ) {
    mSelf = new Tracer();
  }

  return mSelf;
}

void Tracer::beginConnection( const QString &identifier, const QString &msg )
{
  mMutex.lock();
  mTracerBackend->beginConnection( identifier, msg );
  mMutex.unlock();
}

void Tracer::endConnection( const QString &identifier, const QString &msg )
{
  mMutex.lock();
  mTracerBackend->endConnection( identifier, msg );
  mMutex.unlock();
}

void Tracer::connectionInput( const QString &identifier, const QByteArray &msg )
{
  mMutex.lock();
  mTracerBackend->connectionInput( identifier, msg );
  mMutex.unlock();
}

void Tracer::connectionOutput( const QString &identifier, const QByteArray &msg )
{
  mMutex.lock();
  mTracerBackend->connectionOutput( identifier, msg );
  mMutex.unlock();
}

void Tracer::signal( const QString &signalName, const QString &msg )
{
  mMutex.lock();
  mTracerBackend->signal( signalName, msg );
  mMutex.unlock();
}

void Tracer::signal( const char *signalName, const QString &msg )
{
  signal( QLatin1String( signalName ), msg );
}

void Tracer::warning( const QString &componentName, const QString &msg )
{
  mMutex.lock();
  mTracerBackend->warning( componentName, msg );
  mMutex.unlock();
}

void Tracer::error( const QString &componentName, const QString &msg )
{
  mMutex.lock();
  mTracerBackend->error( componentName, msg );
  mMutex.unlock();
}

void Tracer::error( const char *componentName, const QString &msg )
{
  error( QLatin1String( componentName ), msg );
}

QString Tracer::currentTracer() const
{
  QMutexLocker locker( &mMutex );
  const QSettings settings( AkStandardDirs::serverConfigFile(), QSettings::IniFormat );
  return settings.value( QLatin1String( "Debug/Tracer" ), DEFAULT_TRACER ).toString();
}

void Tracer::activateTracer( const QString &type )
{
  QMutexLocker locker( &mMutex );
  delete mTracerBackend;
  mTracerBackend = 0;

  QSettings settings( AkStandardDirs::serverConfigFile(), QSettings::IniFormat );
  settings.setValue( QLatin1String( "Debug/Tracer" ), type );
  settings.sync();

  if ( type == QLatin1String( "file" ) ) {
    const QSettings settings( AkStandardDirs::serverConfigFile(), QSettings::IniFormat );
    const QString file = settings.value( QLatin1String( "Debug/File" ), QLatin1String( "/dev/null" ) ).toString();
    mTracerBackend = new FileTracer( file );
  } else if ( type == QLatin1String( "null" ) ) {
    mTracerBackend = new NullTracer();
  } else {
    mTracerBackend = new DBusTracer();
  }
  Q_ASSERT( mTracerBackend );
}
