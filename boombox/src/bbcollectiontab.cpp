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
#include "bbcollectiontab.h"
#include "bbmainwindow.h"
#include "bbsettings.h"
#include "dbupdatejob.h"
#include "bbsetmodel.h"
#include "dbqueryjob.h"
#include "bbfilterdock.h"
#include "bbresultview.h"
#include "bbsetmodel.h"
#include "bbfilechangejob.h"

#include <QComboBox>
#include <QDir>
#include <QTimer>
#include <QLayout>
#include <QEvent>
#include <QLabel>
#include <KLocale>
#include <KAction>
#include <KActionCollection>
#include <KStandardAction>
#include <kuiserverjobtracker.h>
#include <KGlobal>
#include <KInputDialog>
#include <KMenu>
#include <KMessageBox>
#include <KToolBar>
#include <kuiserverjobtracker.h>
#include <threadweaver/ThreadWeaver.h>
#include <threadweaver/JobCollection.h>
using namespace ThreadWeaver;

BBCollectionTab::BBCollectionTab(int pTabNumber)
   : BBPlaylistSystem(pTabNumber)
{
	setWindowTitle(i18n("Music Collection"));

	mActionCollection = new KActionCollection(this);
	setupActions();
	setupToolBars();

	setDockNestingEnabled(true);

	BBFilterDock *lFilterDock = new BBFilterDock("artist", i18n("Artists"), this);
	mActionCollection->addAction("show_artist", lFilterDock->toggleViewAction());
	addDockWidget(Qt::LeftDockWidgetArea, lFilterDock);
	mDocks.append(lFilterDock);

	BBFilterDock *lFilterDock2 = new BBFilterDock("album", i18n("Albums"), this);
	mActionCollection->addAction("show_album", lFilterDock2->toggleViewAction());
	splitDockWidget(lFilterDock, lFilterDock2, Qt::Horizontal);
	mDocks.append(lFilterDock2);

	lFilterDock = new BBFilterDock("genre", i18n("Genres"), this);
	mActionCollection->addAction("show_genre", lFilterDock->toggleViewAction());
	addDockWidget(Qt::RightDockWidgetArea, lFilterDock);
	mDocks.append(lFilterDock);

	lFilterDock = new BBFilterDock("year", i18n("Years"), this);
	mActionCollection->addAction("show_year", lFilterDock->toggleViewAction());
	addDockWidget(Qt::RightDockWidgetArea, lFilterDock);
	lFilterDock->hide();
	mDocks.append(lFilterDock);

	mModel = new BBAlbumSongModel(this, this);

	mResultView = new BBResultView(this, mModel);
	setCentralWidget(mResultView);

	qRegisterMetaType<QSet<BBStringListItem> >("QSet<BBStringListItem>");
	qRegisterMetaType<QSet<BBSongListItem> >("QSet<BBSongListItem>");
	qRegisterMetaType<QSet<BBAlbumListItem> >("QSet<BBAlbumListItem>");

	setAutoSaveSettings("CollectionTab");
	mCurrentSong = 0;
}

void BBCollectionTab::setupActions()
{
	KStandardAction::openNew(this, SLOT(createNewPlaylist()), mActionCollection);
	KStandardAction::save(this, SLOT(savePlaylist()), mActionCollection);

	KAction *lAction = new KAction(KIcon("view-refresh"), i18n("Update database"), this);
	lAction->setShortcut(KShortcut(Qt::CTRL + Qt::Key_U));
	mActionCollection->addAction("update_database", lAction);
	connect(lAction, SIGNAL(triggered()), SLOT(updateDatabase()));

	lAction = new KAction(KIcon("edit-delete"), i18n("Delete"), this);
	mActionCollection->addAction("delete_playlist", lAction);
	connect(lAction, SIGNAL(triggered()), SLOT(deletePlaylist()));
}

void BBCollectionTab::setupToolBars()
{
	mPlaylistToolBar = toolBar("playlist_toolbar");
	mPlaylistToolBar->setWindowTitle(i18n("Playlist Toolbar"));
	mPlaylistToolBar->addAction(mActionCollection->action(KStandardAction::name(KStandardAction::New)));
	mPlaylistComboBox = new QComboBox(this);
	mPlaylistComboBox->setInsertPolicy(QComboBox::InsertAlphabetically);
	mPlaylistComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	mPlaylistComboBox->setMinimumWidth(mPlaylistComboBox->sizeHint().width()*2);
	mPlaylistToolBar->addWidget(mPlaylistComboBox);
	mPlaylistToolBar->addAction(mActionCollection->action(KStandardAction::name(KStandardAction::Save)));

	mControlToolBar = toolBar("control_toolbar");
	mControlToolBar->setWindowTitle(i18n("Playback Control Toolbar"));
	mControlToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
	mControlsContainer = new QWidget(this);
	mControlsLayout = new QHBoxLayout(mControlsContainer);
	mControlToolBar->addWidget(mControlsContainer);
}

void BBCollectionTab::embedControls(QWidget *pControls)
{
	pControls->setParent(mControlsContainer);
	mControlsLayout->addWidget(pControls);
	pControls->show();
}

BBTabMenuBar *BBCollectionTab::getTabMenuBar()
{
	BBTabMenuBar *lDataMenuBar = new BBTabMenuBar();
	KMenu *lDataMenu = new KMenu(i18n("Playlist"));
	lDataMenu->addAction(mActionCollection->action(KStandardAction::name(KStandardAction::New)));
	lDataMenu->addAction(mActionCollection->action(KStandardAction::name(KStandardAction::Save)));
	lDataMenu->addAction(mActionCollection->action("delete_playlist"));
	lDataMenu->addSeparator();
	lDataMenu->addAction(mActionCollection->action("update_database"));
	lDataMenuBar->addMenu(lDataMenu);

	KMenu *lViewMenu = new KMenu(i18n("View"));
	foreach(BBFilterDock *i, mDocks)
	{
		lViewMenu->addAction(mActionCollection->action(QString("show_%1").arg(i->category())));
	}
	lDataMenuBar->addMenu(lViewMenu);
	return lDataMenuBar;
}

void BBCollectionTab::createNewPlaylist()
{
	QString lNewName = KInputDialog::getText(i18nc("dialog caption", "New Playlist"),
	                                         i18n("Please provide a name for the new playlist:"),
	                                         i18nc("default playlist name", "New Playlist"));
	if(lNewName.isNull())
		return;
	if(mPlaylistComboBox->findText(lNewName) != -1)
	{
		KMessageBox::sorry(this, i18n("The playlist \"%1\" already exists, please choose another name.").arg(lNewName));
		return;
	}
	savePlaylist(lNewName);
	mPlaylistComboBox->disconnect(SIGNAL(currentIndexChanged(QString)));
	mPlaylistComboBox->addItem(lNewName);
	mPlaylistComboBox->setCurrentIndex(mPlaylistComboBox->findText(lNewName));
	connect(mPlaylistComboBox, SIGNAL(currentIndexChanged(QString)), SLOT(loadPlaylist(QString)));

	KConfigGroup lListGroup(mConfig, "PlaylistSystem");
	QStringList lListNames;
	for(int i = 0; i < mPlaylistComboBox->count(); ++i)
		lListNames.append(mPlaylistComboBox->itemText(i));

	lListGroup.writeEntry("ListNames", lListNames);
	mConfig->sync();
}

void BBCollectionTab::savePlaylist()
{
	savePlaylist(mPlaylistComboBox->currentText());
}

void BBCollectionTab::deletePlaylist()
{
	if(mPlaylistComboBox->count() == 1)
		return;

	KConfigGroup lGroup(mConfig, QString("__playlist__") + mPlaylistComboBox->currentText());
	lGroup.deleteGroup();

	mPlaylistComboBox->removeItem(mPlaylistComboBox->currentIndex());

	KConfigGroup lListGroup(mConfig, "PlaylistSystem");
	QStringList lListNames;
	for(int i = 0; i < mPlaylistComboBox->count(); ++i)
		lListNames.append(mPlaylistComboBox->itemText(i));
	lListGroup.writeEntry("ListNames", lListNames);

	mConfig->sync();
}

void BBCollectionTab::savePlaylist(const QString &pName)
{
	KConfigGroup lGroup(mConfig, QString("__playlist__") + pName);
	foreach(BBFilterDock *i, mDocks)
	{
		lGroup.writeEntry(i->category(), i->currentSelection());
		lGroup.writeEntry(i->category() + "_filter", i->filterText());
	}
	mConfig->sync();
}

void BBCollectionTab::updateDatabase()
{
	DBUpdateJob *lJob = new DBUpdateJob();
	mJobTracker->registerJob(lJob);
	connect(lJob, SIGNAL(result(KJob*)), SLOT(databaseUpdated(KJob*)));
	lJob->start();
}

void BBCollectionTab::databaseUpdated(KJob *pJob)
{
	if(!pJob->error())
	{
		BBSettings::setDatabaseUpdateTime(QDateTime::currentDateTime());
		BBSettings::self()->writeConfig();
	}
}

void BBCollectionTab::queryNextSong(BBSongQueryJob &pJob)
{
	QModelIndex lCurrentSongIndex = mModel->indexForFileID(mCurrentSong);
	if(lCurrentSongIndex.isValid())
	{
		QPersistentModelIndex lIndex;
		if(pJob.mShuffle)
			lIndex = mModel->nextShuffled(lCurrentSongIndex);
		else
			lIndex = mModel->nextLinear(lCurrentSongIndex);

		if(lIndex.isValid())
		{
			pJob.mSong = lIndex.data(BBFileIDRole);
			emit songQueryReady(pJob);
			return;
		}
	}
	if(pJob.mShuffle)
		pJob.mSong = mModel->firstShuffled().data(BBFileIDRole);
	else
		pJob.mSong = mModel->firstLinear().data(BBFileIDRole);
	emit songQueryReady(pJob);
}

void BBCollectionTab::queryPreviousSong(BBSongQueryJob &pJob)
{
	QModelIndex lCurrentSongIndex = mModel->indexForFileID(mCurrentSong);
	if(lCurrentSongIndex.isValid())
	{
		QPersistentModelIndex lIndex;
		if(pJob.mShuffle)
			lIndex = mModel->prevShuffled(lCurrentSongIndex);
		else
			lIndex = mModel->prevLinear(lCurrentSongIndex);

		if(lIndex.isValid())
		{
			pJob.mSong = lIndex.data(BBFileIDRole);
			emit songQueryReady(pJob);
			return;
		}
	}
	if(pJob.mShuffle)
		pJob.mSong = mModel->lastShuffled().data(BBFileIDRole);
	else
		pJob.mSong = mModel->lastLinear().data(BBFileIDRole);
	emit songQueryReady(pJob);
}

void BBCollectionTab::setCurrentSong(const QVariant &pSong)
{
	int lNewSong = pSong.toInt();
	QModelIndex lNewSongIndex= mModel->indexForFileID(lNewSong);
	QModelIndex lCurrentSongIndex= mModel->indexForFileID(mCurrentSong);
	int lNewAlbum;
	if(lNewSongIndex.isValid())
		lNewAlbum = lNewSongIndex.data(BBAlbumIDRole).toInt();
	else
	{
		BBResultTable lResult;
		if(mDatabase.getTable(QString::fromAscii("SELECT album_ID FROM songs WHERE file_ID = %1").arg(lNewSong),
		                      lResult) && lResult.mNumRows == 1)
		{
			lNewAlbum = QString(lResult.at(1, 0)).toInt();
			mDatabase.freeTable(lResult);
		}
		else
			lNewAlbum = -1;
	}
	mModel->setCurrentSong(lNewSong, lNewAlbum, lCurrentSongIndex);
	mResultView->scrollToNewSong(lNewSongIndex, lCurrentSongIndex);
	mCurrentSong = lNewSong;
}

QVariant BBCollectionTab::currentSong()
{
	return QVariant::fromValue(mCurrentSong);
}

QString BBCollectionTab::displayString(const QVariant &pSong)
{
	int lSong = pSong.toInt();
	QModelIndex lSongIndex = mModel->indexForFileID(lSong);
	if(lSongIndex.isValid())
		return QString("%1 - %2").arg(lSongIndex.data(BBArtistRole).toString(),
		                              lSongIndex.data(BBTitleRole).toString());
	else
	{
		BBResultTable lResult;
		if(mDatabase.getTable(QString::fromAscii("SELECT artist, title FROM songs WHERE file_ID = %1").arg(lSong),
		                      lResult) && lResult.mNumRows == 1)
		{
			QString lDisplay = QString("%1 - %2").arg(lResult.at(1, 0), lResult.at(1, 1));
			mDatabase.freeTable(lResult);
			return lDisplay;
		}
		else
			return QString();
	}
}

KUrl BBCollectionTab::songUrl(const QVariant &pSong)
{
	int lSong = pSong.toInt();
	QModelIndex lSongIndex = mModel->indexForFileID(lSong);
	if(lSongIndex.isValid())
		return KUrl(QString("%1/%2").arg(lSongIndex.data(BBPathRole).toString(),
		                                 lSongIndex.data(BBFileNameRole).toString()));
	else
	{
		BBResultTable lResult;
		if(mDatabase.getTable(QString::fromAscii("SELECT path, file_name FROM songs JOIN albums USING (album_ID) "
		                                         "WHERE file_ID = %1").arg(lSong), lResult) && lResult.mNumRows == 1)
		{
			QString lUrl = QString("%1/%2").arg(lResult.at(1, 0), lResult.at(1, 1));
			mDatabase.freeTable(lResult);
			return KUrl(lUrl);
		}
		else
			return KUrl();
	}
}

JobCollection *BBCollectionTab::getRefreshJob(BBFilterDock *pNotThisOne)
{
	JobCollection *lJobCollection = new JobCollection();
	connect(lJobCollection, SIGNAL(done(ThreadWeaver::Job *)), SLOT(deleteJob(ThreadWeaver::Job *)));

	Job *lJob = new SongQueryJob(getSongsCondition());
	lJob->assignQueuePolicy(mDBPool);
	lJobCollection->addJob(lJob);
	connect(lJob, SIGNAL(resultReady(QSet<BBSongListItem>, QSet<BBAlbumListItem>)),
	        mModel, SLOT(setNewSongs(QSet<BBSongListItem>,QSet<BBAlbumListItem>)));
	connect(lJob, SIGNAL(done(ThreadWeaver::Job *)), SLOT(deleteJob(ThreadWeaver::Job *)));

	foreach(BBFilterDock *lCurrent, mDocks)
	{
		if(lCurrent != pNotThisOne)
		{
			Job *lJob = new FilterQueryJob(lCurrent->category(), getQueryFor(lCurrent));
			lJob->assignQueuePolicy(mDBPool);
			lJobCollection->addJob(lJob);
			connect(lJob, SIGNAL(resultReady(QSet<BBStringListItem>)),
			        lCurrent, SLOT(processResult(QSet<BBStringListItem>)));
			connect(lJob, SIGNAL(done(ThreadWeaver::Job *)), SLOT(deleteJob(ThreadWeaver::Job *)));
		}
	}
	return lJobCollection;
}

void BBCollectionTab::refreshAllExcept(BBFilterDock *pNotThisOne)
{
	if(mRefreshInhibited)
		return;

	Weaver::instance()->requestAbort();
	Weaver::instance()->dequeue();
	Weaver::instance()->enqueue(getRefreshJob(pNotThisOne));
}

QString BBCollectionTab::getQueryFor(BBFilterDock *pSubject)
{
	QStringList lTerms;

	BBFilterDock *lCurrent;
	foreach(lCurrent, mDocks)
	{
		lCurrent->addCondition(lCurrent != pSubject, lTerms);
	}

	return lTerms.join(" AND ");
}

QString BBCollectionTab::getSongsCondition()
{
	QString lQuery1, lQuery = getQueryFor(NULL);
	QString lFilter = BBDatabase::prepareString(mResultView->filterText());

	if(!lFilter.isEmpty())
		lQuery1 = QString("(artist LIKE '%%1%' OR album LIKE '%%1%' OR title LIKE '%%1%')").arg(lFilter);

	if(!lQuery1.isEmpty())
	{
		if(lQuery.isEmpty())
		{
			lQuery.append(lQuery1);
		}
		else
		{
			lQuery.append(" AND ");
			lQuery.append(lQuery1);
		}
	}
	return lQuery;
}

void BBCollectionTab::deleteJob(ThreadWeaver::Job *pJob)
{
	pJob->deleteLater();
}

void BBCollectionTab::changeFiles(const QString &pField, const QString &pOldValue)
{
	QSet<BBSongListItem> lFileSet = mResultView->filesToUpdate();
	QString lCaption = i18ncp("dialog title", "Change Tag", "Change Tags", lFileSet.count());
	QString lLabel = i18np("Change the %2 field of one file to:", "Change the %2 field of %1 files to:",
	                       lFileSet.count(), i18nc("file tag type", pField.toLocal8Bit()));
	QString lNewValue = KInputDialog::getText(lCaption, lLabel, pOldValue, 0, this);
	if(lNewValue.isNull())
		return;

	//set songs to be removed and added again in the refresh triggered later.
	mModel->updateSongs(lFileSet);

	QList<int> lFileIDList;
	QSetIterator<BBSongListItem> lIterator(lFileSet);
	while(lIterator.hasNext())
		lFileIDList.append(lIterator.next().mData.mFileID);
	BBFileChangeJob *lJob = new BBFileChangeJob(lFileIDList, pField, lNewValue);
	mJobTracker->registerJob(lJob);
	connect(lJob, SIGNAL(result(KJob*)), SLOT(fileChangeJobDone(KJob*)));
	lJob->start();
}

void BBCollectionTab::fileChangeJobDone(KJob *pJob)
{
	BBFileChangeJob *lJob = qobject_cast<BBFileChangeJob *>(pJob);

	mRefreshInhibited = true;
	foreach(BBFilterDock *i, mDocks)
	{
		if(i->category() == lJob->mField)
		{
			i->clearSelection();
			i->clearFilterText();
			QStringList lList(lJob->mNewValue);
			i->forceSelection(lList);
			break;
		}
	}
	mRefreshInhibited = false;
	JobCollection *lRefreshJob = getRefreshJob(NULL);
	connect(lRefreshJob, SIGNAL(done(ThreadWeaver::Job*)), SLOT(finishLoadingPlaylist()));
	Weaver::instance()->enqueue(lRefreshJob);
}

void BBCollectionTab::resetAllFilters()
{
	mRefreshInhibited = true;
	foreach(BBFilterDock *i, mDocks)
	{
		i->clearFilterText();
		i->clearSelection();
	}
	mRefreshInhibited = false;
	refreshAllExcept(NULL);
}

void BBCollectionTab::loadPlaylist(const QString &pName)
{
	if(mRefreshInhibited)
		return;
	mPlaylistName = pName;
	KSharedConfigPtr lConfig = KGlobal::config();
	KConfigGroup lGroup(lConfig, QString("__playlist__") + mPlaylistName);

	mRefreshInhibited = true;
	foreach(BBFilterDock *i, mDocks)
	{
		i->clearFilterText();
		i->clearSelection();
		i->forceSelection(lGroup.readEntry(i->category(), QStringList()));
		i->setFilterText(lGroup.readEntry(i->category() + "_filter", QString()));
	}
	mRefreshInhibited = false;

	JobCollection *lRefreshJob = getRefreshJob(NULL);
	connect(lRefreshJob, SIGNAL(done(ThreadWeaver::Job*)), SLOT(finishLoadingPlaylist()));
	Weaver::instance()->enqueue(lRefreshJob);
}

void BBCollectionTab::finishLoadingPlaylist()
{
	mRefreshInhibited = true;
	foreach(BBFilterDock *i, mDocks)
	{
		i->syncSelection();
	}
	mRefreshInhibited = false;
	mModel->updateSongs(QSet<BBSongListItem>());
}

void BBCollectionTab::saveSession(KConfigGroup &pConfigGroup)
{
	pConfigGroup.writeEntry("chosen_playlist", mPlaylistComboBox->currentIndex());
	foreach(BBFilterDock *i, mDocks)
	{
		pConfigGroup.writeEntry(i->category(), i->currentSelection());
		pConfigGroup.writeEntry(i->category() + "_filter", i->filterText());
	}
}

void BBCollectionTab::readSession(KConfigGroup &pConfigGroup)
{
	mJobTracker = new KUiServerJobTracker(this);
	Weaver::instance()->setMaximumNumberOfThreads(QThread::idealThreadCount());
	mDBPool = new DBPool(QThread::idealThreadCount());

	mDatabase.connect(BBSettings::fileNameDB());

	mConfig = KGlobal::config();
	KConfigGroup lListGroup(mConfig, "PlaylistSystem");
	QStringList lDefaultPlaylistNames;
	lDefaultPlaylistNames << i18n("Default Playlist");
	mPlaylistComboBox->addItems(lListGroup.readEntry("ListNames", lDefaultPlaylistNames));

	connect(mPlaylistComboBox, SIGNAL(currentIndexChanged(QString)), SLOT(loadPlaylist(QString)));

	mRefreshInhibited = true;
	mPlaylistComboBox->setCurrentIndex(pConfigGroup.readEntry("chosen_playlist", 0));

	foreach(BBFilterDock *i, mDocks)
	{
		i->clearFilterText();
		i->clearSelection();
		i->forceSelection(pConfigGroup.readEntry(i->category(), QStringList()));
		i->setFilterText(pConfigGroup.readEntry(i->category() + "_filter", QString()));
	}
	mRefreshInhibited = false;
	JobCollection *lRefreshJob = getRefreshJob(NULL);
	connect(lRefreshJob, SIGNAL(done(ThreadWeaver::Job*)), SLOT(finishLoadingPlaylist()));
	Weaver::instance()->enqueue(lRefreshJob);
}


void BBCollectionTab::refreshAllViewsAndUpdateItem(QModelIndex pIndex)
{
	QSet<BBSongListItem> lSongToUpdate;
	lSongToUpdate.insert(mModel->findSongItem(pIndex));
	mModel->updateSongs(lSongToUpdate);
	JobCollection *lRefreshJob = getRefreshJob(NULL);
	connect(lRefreshJob, SIGNAL(done(ThreadWeaver::Job*)), SLOT(finishLoadingPlaylist()));
	Weaver::instance()->enqueue(lRefreshJob);
}


