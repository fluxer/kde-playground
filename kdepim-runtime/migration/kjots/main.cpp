/*
    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#include "kjotsmigrator.h"

#include "infodialog.h"

#include <akonadi/control.h>

#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#include <KGlobal>
#include <KDebug>


int main( int argc, char **argv )
{
  KAboutData aboutData( "kjotsmigrator", 0,
                        ki18n( "KJots Migration Tool" ),
                        "0.1",
                        ki18n( "Migration of KJots notes to Akonadi" ),
                        KAboutData::License_LGPL,
                        ki18n( "(c) 2010 the Akonadi developers" ),
                        KLocalizedString(),
                        "http://pim.kde.org/akonadi/" );
  aboutData.setProgramIconName( QLatin1String("akonadi") );
  aboutData.addAuthor( ki18n( "Stephen Kelly" ),  ki18n( "Author" ), "steveire@gmail.com" );

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineOptions options;
  options.add( "interactive", ki18n( "Show reporting dialog" ) );
  options.add( "interactive-on-change", ki18n( "Show report only if changes were made" ) );
  KCmdLineArgs::addCmdLineOptions( options );
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  KApplication *app = new KApplication();
  app->setQuitOnLastWindowClosed( false );

  KGlobal::setAllowQuit( true );
  KGlobal::locale()->insertCatalog( QLatin1String("libakonadi") );

  if ( !Akonadi::Control::start( 0 ) )
    return 2;

  InfoDialog *infoDialog = 0;
  if ( args->isSet( "interactive" ) || args->isSet( "interactive-on-change" ) ) {
    infoDialog = new InfoDialog( args->isSet( "interactive-on-change" ) );
    infoDialog->show();
  }
  args->clear();

  KJotsMigrator *migrator = new KJotsMigrator;
  if ( infoDialog && migrator ) {
    infoDialog->migratorAdded();
    QObject::connect( migrator, SIGNAL(message(KMigratorBase::MessageType,QString)),
                      infoDialog, SLOT(message(KMigratorBase::MessageType,QString)) );
    QObject::connect( migrator, SIGNAL(destroyed()), infoDialog, SLOT(migratorDone()) );
  }

  const int result = app->exec();
  if ( InfoDialog::hasError() )
    return 3;
  return result;
}
