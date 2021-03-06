/*
   This file is part of Massif Visualizer

   Copyright 2010 Milian Wolff <mail@milianw.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <KLocalizedString>
#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#include <KCmdLineOptions>
#include <KUrl>

#include <QtCore/QDebug>

#include "mainwindow.h"

int main( int argc, char *argv[] )
{
    KAboutData aboutData( "massif-visualizer", 0, ki18n( "Massif Visualizer" ),
                          "0.5", ki18n("A visualizer for output generated by Valgrind's massif tool."), KAboutData::License_LGPL,
                          ki18n( "Copyright 2010-2014, Milian Wolff <mail@milianw.de>" ),
                          KLocalizedString(), QByteArray(), "massif-visualizer@kde.org" );

    aboutData.addAuthor(ki18n("Milian Wolff"), ki18n("Original author, maintainer"),
                        "mail@milianw.de", "http://milianw.de");

    aboutData.addAuthor(ki18n("Arnold Dumas"), ki18n("Multiple document interface, bug fixes"),
                        "contact@arnolddumas.fr", "http://arnolddumas.fr");

    aboutData.setProgramIconName("office-chart-area");

    KCmdLineArgs::init( argc, argv, &aboutData, KCmdLineArgs::CmdLineArgNone );
    KCmdLineOptions options;
    options.add("+file(s)", ki18n("Opens given output file(s) and visualize it."));

    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    KApplication app;

    Massif::MainWindow* window = new Massif::MainWindow;

    for (int i = 0; i < args->count(); ++i) {
        window->openFile(args->url(i));
    }

    window->show();
    return app.exec();
}
