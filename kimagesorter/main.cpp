/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2011, Dmitry Chernov diman4ik.chernov@gmail.com
 */

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KGlobal>

#include "main_window.h"


int main(int argc, char *argv[])
{
    KAboutData aboutData( "kimagesorter", "kimagesorter",
        ki18n("kimagesorter"), "0.5",
        ki18n("A simple program to sort your image files."),
        KAboutData::License_GPL,
        ki18n("Copyright (c) 2011 Chernov Dmitry") );
    
    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    options.add("+[file]", ki18n("A file to open"));
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    if( app.isSessionRestored() )
        RESTORE( MainWindow )
    else 
    {
        MainWindow *window = new MainWindow;
        window->show();

        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
        for (int pos = 0; pos < args->count(); ++pos) {
            window->openFile(args->url(pos));
        }
    }

  
    return app.exec();
}
