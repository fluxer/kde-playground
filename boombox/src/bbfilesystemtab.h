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
#ifndef BBFILESYSTEMTAB_H
#define BBFILESYSTEMTAB_H

#include "bbplaylistsystem.h"

#include <QList>

#include <KDirOperator>
#include <KFileItem>

class QFileSystemModel;
class QHBoxLayout;
class QModelIndex;
class QVBoxLayout;
class KCategorizedSortFilterProxyModel;
class KDirModel;
class KFilePlacesModel;
class KFilePlacesView;
class KToolBar;
class KUrl;
class KUrlNavigator;

class BBFileSystemTab : public BBPlaylistSystem
{
Q_OBJECT
public:
	explicit BBFileSystemTab(int pTabNumber);
	virtual void queryNextSong(BBSongQueryJob &pJob);
	virtual void queryPreviousSong(BBSongQueryJob &pJob);
	virtual QVariant currentSong();
	virtual void setCurrentSong(const QVariant &pSong);
	virtual QString displayString(const QVariant &pSong);
	virtual KUrl songUrl(const QVariant &pSong);

	void addManualUrl(const KUrl &pUrl);

	virtual void readSession(KConfigGroup &pConfigGroup);
	virtual void saveSession(KConfigGroup &pConfigGroup);
	virtual void embedControls(QWidget *pControls);

	KActionCollection *actionCollection()
	{
		return mDirOperator->actionCollection();
	}

protected slots:
	void setOperatorUrl(const KUrl &pUrl);
	void selectFile(const KFileItem &pFileItem);
	void setCurrentPath(const KUrl &pUrl);
	void buildSearchStack();
	void findSongInSearchStack();
	void enqueueTriggered();

protected:
	QVBoxLayout *mLayout;
	KFilePlacesModel *mFilePlacesModel;
	KFilePlacesView *mFilePlacesView;
	KUrlNavigator *mUrlNavigator;
	KDirOperator *mDirOperator;
	QWidget *mCenterWidget;
	KToolBar *mControlToolBar;
	QWidget *mControlsContainer;
	QHBoxLayout *mControlsLayout;
	KFileItem mCurrentSong;
	KDirModel *mModel;
	KCategorizedSortFilterProxyModel *mProxyModel;
	QList<KFileItem> mSearchStack;
	BBSongQueryJob mSongQuery;
	bool mSearchingForward;
};

#endif // BBFILESYSTEMTAB_H
