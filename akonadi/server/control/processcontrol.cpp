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

#include "processcontrol.h"

#include <akapplication.h>
#include <akdebug.h>

#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <QtDebug>

#ifdef Q_OS_UNIX
#include <sys/types.h>
#include <signal.h>
#endif

using namespace Akonadi;

static const int s_maxCrashCount = 2;

ProcessControl::ProcessControl( QObject *parent )
  : QObject( parent )
  , mFailedToStart( false )
  , mCrashCount( 0 )
  , mRestartOnceOnExit( false )
  , mShutdownTimeout( 1000 )
{
  connect( &mProcess, SIGNAL(error(QProcess::ProcessError)),
           this, SLOT(slotError(QProcess::ProcessError)) );
  connect( &mProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
           this, SLOT(slotFinished(int,QProcess::ExitStatus)) );
  mProcess.setProcessChannelMode( QProcess::ForwardedChannels );

  if ( AkApplication::hasInstanceIdentifier() ) {
    QProcessEnvironment env = mProcess.processEnvironment();
    if ( env.isEmpty() ) {
      env = QProcessEnvironment::systemEnvironment();
    }
    env.insert( QLatin1String( "AKONADI_INSTANCE" ), AkApplication::instanceIdentifier() );
    mProcess.setProcessEnvironment( env );
  }
}

ProcessControl::~ProcessControl()
{
  stop();
}

void ProcessControl::start( const QString &application, const QStringList &arguments, CrashPolicy policy )
{
  mFailedToStart = false;

  mApplication = application;
  mArguments = arguments;
  mPolicy = policy;

  start();
}

void ProcessControl::setCrashPolicy( CrashPolicy policy )
{
  mPolicy = policy;
}

void ProcessControl::stop()
{
  if ( mProcess.state() != QProcess::NotRunning ) {
    mProcess.waitForFinished( mShutdownTimeout );
    mProcess.terminate();
    mProcess.waitForFinished( 10000 );
    mProcess.kill();
  }
}

void ProcessControl::slotError( QProcess::ProcessError error )
{
  switch ( error ) {
  case QProcess::Crashed:
    mCrashCount++;
    // do nothing, we'll respawn in slotFinished
    break;
  case QProcess::FailedToStart:
  default:
      mFailedToStart = true;
    break;
  }

  akError() << "ProcessControl: Application" << qPrintable( mApplication ) << "stopped unexpectedly (" << mProcess.errorString() << ")";
}

void ProcessControl::slotFinished( int exitCode, QProcess::ExitStatus exitStatus )
{
  if ( exitStatus == QProcess::CrashExit ) {
    if ( mPolicy == RestartOnCrash ) {
      // don't try to start an unstartable application
      if ( !mFailedToStart && mCrashCount <= s_maxCrashCount ) {
        qWarning( "Application '%s' crashed! %d restarts left.", qPrintable( mApplication ), s_maxCrashCount - mCrashCount );
        start();
        Q_EMIT restarted();
      } else {
        if ( mFailedToStart ) {
          qWarning( "Application '%s' failed to start!", qPrintable( mApplication ) );
        } else {
          qWarning( "Application '%s' crashed too often. Giving up!", qPrintable( mApplication ) );
        }
        mPolicy = StopOnCrash;
        Q_EMIT unableToStart();
        return;
      }
    } else {
      qWarning( "Application '%s' crashed. No restart!", qPrintable( mApplication ) );
    }
  } else {
    if ( exitCode != 0 ) {
      qWarning( "ProcessControl: Application '%s' returned with exit code %d (%s)",
              qPrintable( mApplication ), exitCode, qPrintable( mProcess.errorString() ) );
      if ( mPolicy == RestartOnCrash ) {
        if ( mCrashCount > s_maxCrashCount ) {
          qWarning() << mApplication << "crashed too often and will not be restarted!";
          mPolicy = StopOnCrash;
          Q_EMIT unableToStart();
          return;
        }
        ++mCrashCount;
        QTimer::singleShot( 60000, this, SLOT(resetCrashCount()) );
        if ( !mFailedToStart ) { // don't try to start an unstartable application
          start();
          Q_EMIT restarted();
        }
      }
    } else {
      if ( mRestartOnceOnExit ) {
        mRestartOnceOnExit = false;
        qWarning( "Restarting application '%s'.", qPrintable( mApplication ) );
        start();
      } else {
        qWarning( "Application '%s' exited normally...", qPrintable( mApplication ) );
      }
    }
  }
}

static bool listContains( const QStringList &list, const QString &pattern )
{
  Q_FOREACH ( const QString &s, list ) {
    if ( s.contains( pattern ) ) {
      return true;
    }
  }
  return false;
}

void ProcessControl::start()
{
#ifdef Q_OS_UNIX
  QString agentValgrind = getEnv( "AKONADI_VALGRIND" );
  if ( !agentValgrind.isEmpty() && ( mApplication.contains( agentValgrind ) || listContains( mArguments, agentValgrind ) ) ) {

    mArguments.prepend( mApplication );
    const QString originalArguments = mArguments.join( QString::fromLocal8Bit( " " ) );
    mApplication = QString::fromLocal8Bit( "valgrind" );

    const QString valgrindSkin = getEnv( "AKONADI_VALGRIND_SKIN", QString::fromLocal8Bit( "memcheck" ) );
    mArguments.prepend( QLatin1String( "--tool=" ) + valgrindSkin );

    const QString valgrindOptions = getEnv( "AKONADI_VALGRIND_OPTIONS" );
    if ( !valgrindOptions.isEmpty() ) {
      mArguments = valgrindOptions.split( QLatin1Char( ' ' ), QString::SkipEmptyParts ) << mArguments;
    }

    akDebug();
    akDebug() << "============================================================";
    akDebug() << "ProcessControl: Valgrinding process" << originalArguments;
    if ( !valgrindSkin.isEmpty() ) {
      akDebug() << "ProcessControl: Valgrind skin:" << valgrindSkin;
    }
    if ( !valgrindOptions.isEmpty() ) {
      akDebug() << "ProcessControl: Additional Valgrind options:" << valgrindOptions;
    }
    akDebug() << "============================================================";
    akDebug();
  }
#endif

  mProcess.start( mApplication, mArguments );
  if ( !mProcess.waitForStarted() ) {
    qWarning( "ProcessControl: Unable to start application '%s' (%s)",
            qPrintable( mApplication ), qPrintable( mProcess.errorString() ) );
    Q_EMIT unableToStart();
    return;
  }

#ifdef Q_OS_UNIX
  else {
    QString agentDebug = QString::fromLocal8Bit( qgetenv( "AKONADI_DEBUG_WAIT" ) );
    pid_t pid = mProcess.pid();
    if ( !agentDebug.isEmpty() && mApplication.contains( agentDebug ) ) {
      akDebug();
      akDebug() << "============================================================";
      akDebug() << "ProcessControl: Suspending process" << mApplication;
      akDebug() << "'gdb --pid" << pid << "' to debug";
      akDebug() << "'kill -SIGCONT" << pid << "' to continue";
      akDebug() << "============================================================";
      akDebug();
      kill( pid, SIGSTOP );
    }
  }
#endif
}

void ProcessControl::resetCrashCount()
{
  mCrashCount = 0;
}

bool ProcessControl::isRunning() const
{
  return mProcess.state() != QProcess::NotRunning;
}

void ProcessControl::setShutdownTimeout( int msecs )
{
  mShutdownTimeout = msecs;
}
