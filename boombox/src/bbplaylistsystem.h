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
#ifndef BBPLAYLISTSYSTEM_H
#define BBPLAYLISTSYSTEM_H

#include "bbmainwindow.h"

#include <QVariant>
#include <KConfigGroup>
#include <KMainWindow>
#include <KUrl>

class BBPlaylistSystem: public KMainWindow
{
Q_OBJECT
public:
	BBPlaylistSystem(int pTabNumber) : mTabNumber(pTabNumber) {}
	virtual ~BBPlaylistSystem() {}
	virtual void queryNextSong(BBSongQueryJob &pJob) = 0;
	virtual void queryPreviousSong(BBSongQueryJob &pJob) = 0;

	virtual QVariant currentSong() = 0;
	virtual void setCurrentSong(const QVariant &pSong) = 0;

	virtual QString displayString(const QVariant &pSong) = 0;
	virtual KUrl songUrl(const QVariant &pSong) = 0;

	virtual void readSession(KConfigGroup &pConfigGroup) = 0;
	virtual void saveSession(KConfigGroup &pConfigGroup) = 0;

	virtual void embedControls(QWidget *pControls) = 0;

	int tabNumber() {return mTabNumber;}

signals:
	void songQueryReady(const BBSongQueryJob &pJob);

protected:
	int mTabNumber;
};


enum BBPlaylistRoles
{
	BBPathRole = Qt::UserRole + 1, BBFileNameRole,
	BBArtistRole, BBTitleRole, BBAlbumRole, BBYearRole, BBLengthRole, BBFileIDRole,
	BBIsFolderRole, BBUrlRole, BBAlbumIDRole
};

#endif
