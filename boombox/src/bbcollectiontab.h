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
#ifndef BBCOLLECTIONTAB_H
#define BBCOLLECTIONTAB_H

#include "bbdatabase.h"
#include "bbplaylistsystem.h"

#include <QList>
#include <QPersistentModelIndex>
#include <QSet>

#include <KSharedConfig>

class QHBoxLayout;
class QComboBox;

class KUiServerJobTracker;
class KActionCollection;
class KJob;
class KToolBar;
class BBPlaylistSystem;
class BBTabMenuBar;
class DBPool;
class BBFilterDock;
class BBResultView;
class BBAlbumSongModel;
class BBSongListItem;
class BBAlbumListItem;

Q_DECLARE_METATYPE(QPersistentModelIndex)

namespace ThreadWeaver
{
	class Job;
	class JobCollection;
}

class BBCollectionTab : public BBPlaylistSystem
{
	Q_OBJECT
public:
	explicit BBCollectionTab(int pTabNumber);
	BBTabMenuBar *getTabMenuBar();

	virtual void queryNextSong(BBSongQueryJob &pJob);
	virtual void queryPreviousSong(BBSongQueryJob &pJob);
	virtual QVariant currentSong();
	virtual void setCurrentSong(const QVariant &pSong);
	virtual QString displayString(const QVariant &pSong);
	virtual KUrl songUrl(const QVariant &pSong);

	virtual void readSession(KConfigGroup &pConfigGroup);
	virtual void saveSession(KConfigGroup &pConfigGroup);
	virtual void embedControls(QWidget *pControls);

	void refreshAllExcept(BBFilterDock *pSubject);
	void changeFiles(const QString &pField, const QString &pOldValue);
	KActionCollection *actionCollection()
	{
		return mActionCollection;
	}

public slots:
	void savePlaylist(const QString &pName);
	void loadPlaylist(const QString &pName);
	void refreshAllViewsAndUpdateItem(QModelIndex pIndex);

protected slots:
	void deleteJob(ThreadWeaver::Job *pJob);
	void fileChangeJobDone(KJob *pJob);
	void resetAllFilters();
	void finishLoadingPlaylist();
	void createNewPlaylist();
	void savePlaylist();
	void updateDatabase();
	void databaseUpdated(KJob *);
	void deletePlaylist();

protected:
	void setupActions();
	void setupToolBars();
	ThreadWeaver::JobCollection *getRefreshJob(BBFilterDock *pNotThisOne);
	QString getQueryFor(BBFilterDock *pSubject);
	QString getSongsCondition();

	DBPool *mDBPool;
	QList<BBFilterDock *> mDocks;
	BBResultView *mResultView;
	BBAlbumSongModel *mModel;
	int mCurrentSong;
	bool mRefreshInhibited;
	QString mPlaylistName;
	QComboBox *mPlaylistComboBox;
	KToolBar *mControlToolBar;
	KToolBar *mPlaylistToolBar;
	QWidget *mControlsContainer;
	QHBoxLayout *mControlsLayout;

	KUiServerJobTracker *mJobTracker;
	BBPlaylistSystem *mPlaylistSystem;

	KSharedConfigPtr mConfig;
	KActionCollection *mActionCollection;

	BBDatabase mDatabase;
};

#endif
