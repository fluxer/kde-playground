/*
 * Copyright (C) 2010 Tobias Koenig <tokoe@kde.org>
 * Copyright (C) 2014 Daniel Vrátil <dvratil@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "utils.h"

#include <akdebug.h>
#include <akstandarddirs.h>
#include <libs/xdgbasedirs_p.h>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtNetwork/QHostInfo>

#if !defined(Q_OS_WIN)
#include <cstdlib>
#include <sys/types.h>
#include <cerrno>
#include <pwd.h>
#include <unistd.h>

static QString akonadiSocketDirectory();
static bool checkSocketDirectory( const QString &path );
static bool createSocketDirectory( const QString &link, const QString &tmpl );
#endif

#ifdef Q_OS_LINUX
#include <stdio.h>
#include <mntent.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#endif

using namespace Akonadi;
using namespace Akonadi::Server;

QString Utils::preferredSocketDirectory( const QString &defaultDirectory )
{
  const QString serverConfigFile = AkStandardDirs::serverConfigFile( XdgBaseDirs::ReadWrite );
  const QSettings serverSettings( serverConfigFile, QSettings::IniFormat );

#if defined(Q_OS_WIN)
  const QString socketDir = serverSettings.value( QLatin1String( "Connection/SocketDirectory" ), defaultDirectory ).toString();
#else
  QString socketDir = defaultDirectory;
  if ( !serverSettings.contains( QLatin1String( "Connection/SocketDirectory" ) ) ) {
    // if no socket directory is defined, use the symlinked from /tmp
    socketDir = akonadiSocketDirectory();

    if ( socketDir.isEmpty() ) { // if that does not work, fall back on default
      socketDir = defaultDirectory;
    }
  } else {
    socketDir = serverSettings.value( QLatin1String( "Connection/SocketDirectory" ), defaultDirectory ).toString();
  }

  const QString userName = QString::fromLocal8Bit( qgetenv( "USER" ) );
  if ( socketDir.contains( QLatin1String( "$USER" ) ) && !userName.isEmpty() ) {
    socketDir.replace( QLatin1String( "$USER" ), userName );
  }

  if ( socketDir[0] != QLatin1Char( '/' ) ) {
    QDir::home().mkdir( socketDir );
    socketDir = QDir::homePath() + QLatin1Char( '/' ) + socketDir;
  }

  QFileInfo dirInfo( socketDir );
  if ( !dirInfo.exists() ) {
    QDir::home().mkpath( dirInfo.absoluteFilePath() );
  }
#endif
  return socketDir;
}

#if !defined(Q_OS_WIN)
QString akonadiSocketDirectory()
{
  const QString hostname = QHostInfo::localHostName();

  if ( hostname.isEmpty() ) {
    qCritical() << "QHostInfo::localHostName() failed";
    return QString();
  }

  const uid_t uid = getuid();
  const struct passwd *pw_ent = getpwuid( uid );
  if ( !pw_ent ) {
    qCritical() << "Could not get passwd entry for user id" << uid;
    return QString();
  }

  const QString link = AkStandardDirs::saveDir( "data" ) + QLatin1Char( '/' ) + QLatin1String( "socket-" ) + hostname;
  const QString tmpl = QLatin1String( "akonadi-" ) + QLatin1String( pw_ent->pw_name ) + QLatin1String( ".XXXXXX" );

  if ( checkSocketDirectory( link ) ) {
    return QFileInfo( link ).symLinkTarget();
  }

  if ( createSocketDirectory( link, tmpl ) ) {
    return QFileInfo( link ).symLinkTarget();
  }

  qCritical() << "Could not create socket directory for Akonadi.";
  return QString();
}

static bool checkSocketDirectory( const QString &path )
{
  QFileInfo info( path );

  if ( !info.exists() ) {
    return false;
  }

  if ( info.isSymLink() ) {
    info = QFileInfo( info.symLinkTarget() );
  }

  if ( !info.isDir() ) {
    return false;
  }

  if ( info.ownerId() != getuid() ) {
    return false;
  }

  return true;
}

static bool createSocketDirectory( const QString &link, const QString &tmpl )
{
  QString directory = QString::fromLatin1( "%1%2%3" ).arg( QDir::tempPath() ).arg( QDir::separator() ).arg( tmpl );

  QByteArray directoryString = directory.toLocal8Bit().data();

  if ( !mkdtemp( directoryString.data() ) ) {
    qCritical() << "Creating socket directory with template" << directoryString << "failed:" << strerror( errno );
    return false;
  }

  directory = QString::fromLocal8Bit( directoryString );

  QFile::remove( link );

  if ( !QFile::link( directory, link ) ) {
    qCritical() << "Creating symlink from" << directory << "to" << link << "failed";
    return false;
  }

  return true;
}
#endif

QString Utils::getDirectoryFileSystem(const QString &directory)
{
#ifndef Q_OS_LINUX
    return QString();
#else
    QString bestMatchPath;
    QString bestMatchFS;

    FILE *mtab = setmntent("/etc/mtab", "r");
    while (mntent *mnt = getmntent(mtab)) {
        if (qstrcmp(mnt->mnt_type, MNTTYPE_IGNORE) == 0) {
            continue;
        }

        const QString dir = QString::fromLocal8Bit(mnt->mnt_dir);
        if (!directory.startsWith(dir) || dir.length() < bestMatchPath.length()) {
            continue;
        }

        bestMatchPath = dir;
        bestMatchFS = QString::fromLocal8Bit(mnt->mnt_type);
    }

    endmntent(mtab);

    return bestMatchFS;
#endif
}

void Utils::disableCoW(const QString& path)
{
#ifndef Q_OS_LINUX
    Q_UNUSED(path);
#else
    qDebug() << "Detected Btrfs, disabling copy-on-write on database files";

    // from linux/fs.h, so that Akonadi does not depend on Linux header files
    #ifndef FS_IOC_GETFLAGS
    #define FS_IOC_GETFLAGS     _IOR('f', 1, long)
    #endif
    #ifndef FS_IOC_SETFLAGS
    #define FS_IOC_SETFLAGS     _IOW('f', 2, long)
    #endif

    // Disable COW on file
    #ifndef FS_NOCOW_FL
    #define FS_NOCOW_FL         0x00800000
    #endif

    ulong flags = 0;
    const int fd = open(qPrintable(path), O_RDONLY);
    if (fd == -1) {
        qWarning() << "Failed to open" << path << "to modify flags (" << errno << ")";
        return;
    }

    if (ioctl(fd, FS_IOC_GETFLAGS, &flags) == -1) {
        qWarning() << "ioctl error: failed to get file flags (" << errno << ")";
        close(fd);
        return;
    }
    if (!(flags & FS_NOCOW_FL)) {
        flags |= FS_NOCOW_FL;
        if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == -1) {
            qWarning() << "ioctl error: failed to set file flags (" << errno << ")";
            close(fd);
            return;
        }
    }
    close(fd);
#endif
}
