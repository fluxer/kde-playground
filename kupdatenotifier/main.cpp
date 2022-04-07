/*  This file is part of the KDE project
    Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QDBusConnectionInterface>
#include <kuniqueapplication.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include "kupdatenotifier.h"

int main(int argc, char** argv)
{
    KAboutData aboutData(
        "kupdatenotifier", 0, ki18n("KUpdateNotifier"),
        "1.0.0", ki18n("A panel applet for software updates notification."),
        KAboutData::License_GPL_V2,
        ki18n("(c) 2022 Ivailo Monev"),
        KLocalizedString(),
        "http://github.com/fluxer/katana"
    );

    aboutData.addAuthor(
        ki18n("Ivailo Monev"),
        ki18n("Maintainer"),
        "xakepa10@gmail.com"
    );
    aboutData.setProgramIconName(QLatin1String("system-software-update"));

    KCmdLineArgs::init(argc, argv, &aboutData);
    KUniqueApplication::addCmdLineOptions();

    KUniqueApplication kupdatenotifierapp;
    kupdatenotifierapp.setQuitOnLastWindowClosed(false);
    if (!KUniqueApplication::start()) {
        kDebug() << "kupdatenotifier is already running!";
        return 0;
    }

    QDBusConnectionInterface* systembusiface = QDBusConnection::systemBus().interface();
    if (!systembusiface->isServiceRegistered(PACKAGEKIT_SERVICE)) {
        kDebug() << "activating" << PACKAGEKIT_SERVICE;
        systembusiface->startService(PACKAGEKIT_SERVICE);
    }

    KUpdateNotifier kupdatenotifier(&kupdatenotifierapp);

    return kupdatenotifierapp.exec();
}
