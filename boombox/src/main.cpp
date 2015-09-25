/***************************************************************************
 *   Copyright (C) by Simon Persson                                        *
 *   simonop@spray.se                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "bbmainwindow.h"
#include "mpris2playerclient.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <KStartupInfo>
#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>

static const char description[] = I18N_NOOP("Music player with dynamic playlists.");

static const char version[] = "0.4";

BBMainWindow *gMainWindow;

extern "C" int main(int argc, char **argv)
{
	KAboutData lAbout("boombox", 0, ki18n("BoomBox"), version, ki18n(description),
	                  KAboutData::License_GPL, ki18n("(C) 2009 Simon Persson"),
	                  KLocalizedString(), 0, "simonop@spray.se");
	lAbout.addAuthor( ki18n("Simon Persson"), KLocalizedString(), "simonop@spray.se" );
	KCmdLineArgs::init(argc, argv, &lAbout);

	KCmdLineOptions lOptions;
	lOptions.add("+[URL]", ki18n("File or URL to add to playlist"));
	KCmdLineArgs::addCmdLineOptions(lOptions);
	KApplication lApp;

	QDBusConnection lConnection = QDBusConnection::sessionBus();

	QDBusConnectionInterface *lSessionInterface = lConnection.interface();
	QDBusReply<bool> lIsRegistered = lSessionInterface->isServiceRegistered(QLatin1String("org.mpris.MediaPlayer2.BoomBox"));
	if(!lIsRegistered.value())
	{
		lApp.setAttribute(Qt::AA_DontUseNativeMenuBar);
		gMainWindow = new BBMainWindow(&lAbout);
		gMainWindow->readSession();
		gMainWindow->show();
	}

	QDBusPendingReply<> lReply;
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	if (args->count() > 0)
	{
		org::mpris::MediaPlayer2::Player *lInterface;
		lInterface = new org::mpris::MediaPlayer2::Player(QLatin1String("org.mpris.MediaPlayer2.BoomBox"),
		                                                  QLatin1String("/org/mpris/MediaPlayer2"), lConnection);
		for(int i = 0; i < args->count(); ++i)
			lReply = lInterface->OpenUri(args->arg(i));
	}

	if(lIsRegistered.value())
	{
		KStartupInfo::appStarted(); //make startup notification go away.
		if(args->count() > 0)
			lReply.waitForFinished();
		return 0; //one instance already running in this session.
	}
	else
	{
		if(args->count() > 0)
			gMainWindow->togglePlayback();
		return lApp.exec();
	}

}
