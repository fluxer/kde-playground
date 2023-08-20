/*  This file is part of KArchiveManager
    Copyright (C) 2018 Ivailo Monev <xakepa10@gmail.com>

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

#include <KAboutData>
#include <KCmdLineArgs>
#include <KUniqueApplication>
#include <KUrl>

#include "karchiveapp.hpp"

int main(int argc, char** argv)
{
    QApplication::setAttribute(Qt::AA_X11InitThreads, true);

    KAboutData aboutData(
        "karchivemanager", 0, ki18n("Archive Manager"),
        "1.0.0", ki18n("Simple archive manager for KDE."),
        KAboutData::License_GPL_V2,
        ki18n("(c) 2018 Ivailo Monev")
    );

    aboutData.addAuthor(ki18n("Ivailo Monev"),
                        ki18n("Maintainer"),
                        "xakepa10@gmail.com");
    aboutData.setProgramIconName(QLatin1String("package-x-generic"));

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineOptions option;
    option.add("+[url]", ki18n("URL to be opened"));
    KCmdLineArgs::addCmdLineOptions(option);

    KApplication *karchiveapp = new KApplication();
    KArchiveApp karchivewin;
    karchivewin.show();

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    for (int pos = 0; pos < args->count(); ++pos) {
        karchivewin.changePath(args->url(pos).toLocalFile());
    }

    return karchiveapp->exec();
}
