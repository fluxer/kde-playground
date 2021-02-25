/*  This file is part of KMan
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
#include "kmanmainwindow.h"

int main(int argc, char** argv)
{
    KAboutData aboutData("kman", 0, ki18n("KMan"),
                         "1.0.0", ki18n("Simple manual page reader for KDE."),
                         KAboutData::License_GPL_V2,
                         ki18n("(c) 2018 Ivailo Monev"),
                         KLocalizedString(),
                        "http://github.com/fluxer/katana"
                        );

    aboutData.addAuthor(ki18n("Ivailo Monev"),
                        ki18n("Maintainer"),
                        "xakepa10@gmail.com");
    aboutData.setProgramIconName(QLatin1String("help-browser"));

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineOptions option;
    option.add("+[url]", ki18n("URL to be opened"));
    KCmdLineArgs::addCmdLineOptions(option);

    KUniqueApplication *kmanapp = new KUniqueApplication();
    KManMainWindow *kmanwin = new KManMainWindow();
    kmanwin->show();

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    for (int pos = 0; pos < args->count(); ++pos) {
        kmanwin->changePath(args->url(pos).prettyUrl());
    }

    return kmanapp->exec();
}
