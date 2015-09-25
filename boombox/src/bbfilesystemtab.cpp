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
#include "bbfilesystemtab.h"
#include "bbmainwindow.h"
#include "bbmetadata.h"

#include <QDockWidget>
#include <QFileSystemModel>
#include <QLayout>
#include <QMetaType>
#include <KActionCollection>
#include <KActionMenu>
#include <KCategorizedSortFilterProxyModel>
#include <KConfigGroup>
#include <KDirModel>
#include <KFilePlacesModel>
#include <KFilePlacesView>
#include <KGlobal>
#include <KGlobalSettings>
#include <KLocale>
#include <KMenu>
#include <KToolBar>
#include <KUrlNavigator>

#include <QDebug>

BBFileSystemTab::BBFileSystemTab(int pTabNumber)
   :BBPlaylistSystem(pTabNumber)
{
	setWindowTitle(i18n("File System"));
}

void BBFileSystemTab::queryNextSong(BBSongQueryJob &pJob)
{
	mSongQuery = pJob;
	mSearchStack.clear();
	mSearchingForward = true;
	buildSearchStack();
	findSongInSearchStack();
}

void BBFileSystemTab::queryPreviousSong(BBSongQueryJob &pJob)
{
	mSongQuery = pJob;
	mSearchStack.clear();
	mSearchingForward = false;
	buildSearchStack();
	findSongInSearchStack();
}

void BBFileSystemTab::buildSearchStack()
{
	QModelIndex lCurrent;
	if(!mCurrentSong.isNull())
		lCurrent = mProxyModel->mapFromSource(mModel->indexForItem(mCurrentSong));
	if(lCurrent.isValid())
	{
		if(mSearchingForward)
		{
			do
			{
				int lStartRow = lCurrent.row() + 1;
				lCurrent = lCurrent.parent();
				int lMax = mProxyModel->rowCount(lCurrent);
				for(int i = lStartRow; i < lMax; ++i)
					mSearchStack.append(mModel->itemForIndex(mProxyModel->mapToSource(mProxyModel->index(i, 0, lCurrent))));
			}while(lCurrent.isValid());
		}
		else
		{
			do
			{
				int lStartRow = lCurrent.row() - 1;
				lCurrent = lCurrent.parent();
				for(int i = lStartRow; i >= 0; --i)
					mSearchStack.append(mModel->itemForIndex(mProxyModel->mapToSource(mProxyModel->index(i, 0, lCurrent))));
			}while(lCurrent.isValid());
		}
	}
	else
	{
		if(mSearchingForward)
		{
			int lMax = mProxyModel->rowCount();
			for(int i = 0; i < lMax; ++i)
				mSearchStack.append(mModel->itemForIndex(mProxyModel->mapToSource(mProxyModel->index(i, 0))));
		}
		else
		{
			for(int i = mProxyModel->rowCount() - 1; i >= 0; --i)
				mSearchStack.append(mModel->itemForIndex(mProxyModel->mapToSource(mProxyModel->index(i, 0))));
		}
	}
}

void BBFileSystemTab::findSongInSearchStack()
{
	while(!mSearchStack.isEmpty())
	{
		KFileItem lCurrent = mSearchStack.takeFirst();
		if((lCurrent.isFile() || lCurrent.isLink()) && lCurrent.isReadable())
		{
			mSongQuery.mSong = lCurrent;
			emit songQueryReady(mSongQuery);
			mSearchStack.clear();
			return;
		}
		if(lCurrent.isDir() && lCurrent.isReadable())
		{
			QModelIndex lCurrentIndex = mModel->indexForItem(lCurrent);
			if(!lCurrentIndex.isValid())
				continue;
			if(mModel->canFetchMore(lCurrentIndex))
			{
				mModel->fetchMore(lCurrentIndex);
				mSearchStack.prepend(lCurrent);
				return;
			}
			QModelIndex lCurrentProxyIndex = mProxyModel->mapFromSource(lCurrentIndex);
			if(mSearchingForward)
			{
				for(int i = mProxyModel->rowCount(lCurrentProxyIndex) - 1; i >= 0; --i)
					mSearchStack.prepend(mModel->itemForIndex(mProxyModel->mapToSource(mProxyModel->index(i, 0, lCurrentProxyIndex))));
			}
			else
			{
				for(int i = 0; i < mProxyModel->rowCount(lCurrentProxyIndex); ++i)
					mSearchStack.prepend(mModel->itemForIndex(mProxyModel->mapToSource(mProxyModel->index(i, 0, lCurrentProxyIndex))));
			}
		}
	}
}

QVariant BBFileSystemTab::currentSong()
{
	return mCurrentSong;
}

void BBFileSystemTab::setCurrentSong(const QVariant &pSong)
{
	mCurrentSong = pSong.value<KFileItem>();
	KUrl lUrl(mCurrentSong.url().directory());
	QString lDirectory = lUrl.url();
	QString lDirectory2 = mDirOperator->url().url();
	if(!lDirectory.startsWith(lDirectory2))
	{
		mDirOperator->setUrl(lDirectory, true);
		mFilePlacesView->setUrl(lDirectory);
		mUrlNavigator->setLocationUrl(lDirectory);
	}
	mDirOperator->setCurrentItem(mCurrentSong);
}

QString BBFileSystemTab::displayString(const QVariant &pSong)
{
	KFileItem lFileItem = pSong.value<KFileItem>();
	if(!lFileItem.isLocalFile())
		return QString();
	BBMetaData lMetaData;
	QString lPath = lFileItem.localPath();
	QString lFileName = lFileItem.name();
	lPath.chop(lFileName.length());
	lMetaData.GetInfoFrom(lPath, lFileName);
	return QString("%1 - %2").arg(lMetaData.mArtist, lMetaData.mTitle);
}

KUrl BBFileSystemTab::songUrl(const QVariant &pSong)
{
	KFileItem lSong = pSong.value<KFileItem>();
	if(lSong.isNull())
		return KUrl();
	else
		return lSong.targetUrl();
}

void BBFileSystemTab::readSession(KConfigGroup &pConfigGroup)
{
	mFilePlacesModel = new KFilePlacesModel(this);
	mCenterWidget = new QWidget(this);
	setCentralWidget(mCenterWidget);
	mLayout = new QVBoxLayout(mCenterWidget);

	mFilePlacesView = new KFilePlacesView();
	mFilePlacesView->setModel(mFilePlacesModel);
	mDirOperator = new KDirOperator(KGlobalSettings::musicPath(), this);
	QString lMimeTypes = QString("application/x-ogg;audio/basic;audio/vnd.rn-realaudio;audio/x-aiff;"
	                             "audio/x-flac;audio/x-matroska;audio/x-mp3;audio/mpeg;audio/ogg;"
	                             "audio/x-flac+ogg;audio/x-vorbis+ogg;audio/x-ms-wma;"
	                             "audio/x-pn-realaudio;audio/x-wav;inode/directory");
	QStringList lMimeFilter = lMimeTypes.split(";");
	mDirOperator->setMimeFilter(lMimeFilter);
	mDirOperator->readConfig(pConfigGroup);
	mDirOperator->setView(KFile::DetailTree);
	mDirOperator->view()->setAlternatingRowColors(true);
	mDirOperator->actionCollection()->action("preview")->setChecked(false);

	KAction *lEnqueueAction = new KAction(i18n("Add to Queue"), this);
	lEnqueueAction->setShortcut(Qt::CTRL + Qt::Key_E);
	lEnqueueAction->setShortcutContext(Qt::WidgetShortcut);
	connect(lEnqueueAction, SIGNAL(triggered()), SLOT(enqueueTriggered()));
	mDirOperator->view()->addAction(lEnqueueAction);

	KMenu *lPopupMenu = new KMenu(this);
	lPopupMenu->addAction(lEnqueueAction);
	lPopupMenu->addSeparator();
	lPopupMenu->addAction(mDirOperator->actionCollection()->action("sorting menu"));
	lPopupMenu->addAction(mDirOperator->actionCollection()->action("view menu"));
	lPopupMenu->addSeparator();
	lPopupMenu->addAction(mDirOperator->actionCollection()->action("properties"));
	KActionMenu *lActionMenu = (KActionMenu *)mDirOperator->actionCollection()->action("popupMenu");
	lActionMenu->setMenu(lPopupMenu);

	mUrlNavigator = new KUrlNavigator(mFilePlacesModel, mDirOperator->url(), this);

	connect(mUrlNavigator, SIGNAL(urlChanged(KUrl)), this, SLOT(setOperatorUrl(KUrl)));
	connect(mFilePlacesView, SIGNAL(urlChanged(KUrl)),this, SLOT(setOperatorUrl(KUrl)));
	connect(mDirOperator, SIGNAL(urlEntered(KUrl)), this, SLOT(setCurrentPath(KUrl)));
	connect(mDirOperator, SIGNAL(fileSelected(KFileItem)), this, SLOT(selectFile(KFileItem)));
	mLayout->addWidget(mUrlNavigator);
	mLayout->addWidget(mDirOperator);

	QDockWidget *lPlacesDock = new QDockWidget(i18n("Places"), this);
	lPlacesDock->setObjectName("fs_places_dock");
	lPlacesDock->setFeatures(QDockWidget::DockWidgetMovable);
	lPlacesDock->setWidget(mFilePlacesView);
	addDockWidget(Qt::LeftDockWidgetArea, lPlacesDock);

	mControlToolBar = toolBar("fs_control_toolbar");
	mControlToolBar->setWindowTitle(i18n("Playback Control Toolbar"));
	mControlToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
	mControlsContainer = new QWidget(this);
	mControlsLayout = new QHBoxLayout(mControlsContainer);
	mControlToolBar->addWidget(mControlsContainer);

	setAutoSaveSettings("FileSystemTab");
	mProxyModel = (KCategorizedSortFilterProxyModel *)mDirOperator->view()->model();
	mModel = (KDirModel *)mProxyModel->sourceModel();
	connect(mModel->dirLister(), SIGNAL(completed()), this, SLOT(findSongInSearchStack()));
	setCurrentPath(pConfigGroup.readEntry("filesystemtab_path", KGlobalSettings::musicPath()));
}

void BBFileSystemTab::saveSession(KConfigGroup &pConfigGroup)
{
	mDirOperator->writeConfig(pConfigGroup);
	pConfigGroup.writePathEntry("filesystemtab_path", mDirOperator->url().toEncoded());
}

void BBFileSystemTab::embedControls(QWidget *pControls)
{
	pControls->setParent(mControlsContainer);
	mControlsLayout->addWidget(pControls);
	pControls->show();
}

void BBFileSystemTab::setOperatorUrl(const KUrl &pUrl)
{
	mDirOperator->setUrl(pUrl, true);
}

void BBFileSystemTab::selectFile(const KFileItem &pFileItem)
{
	if(pFileItem.isFile() && pFileItem.isReadable() && pFileItem != mCurrentSong)
	{
		gMainWindow->setCurrentPlaylistSystem(tabNumber());
		gMainWindow->setCurrentSong(pFileItem);
	}
}

void BBFileSystemTab::enqueueTriggered()
{
	KFileItemList lSelectedItems = mDirOperator->selectedItems();
	foreach(KFileItem lItem, lSelectedItems)
	{
		if(lItem.isFile() && lItem.isReadable())
			gMainWindow->addToPlayQueue(lItem, displayString(lItem), tabNumber());
	}
}

void BBFileSystemTab::setCurrentPath(const KUrl &pUrl)
{
	mUrlNavigator->setLocationUrl(pUrl);
	mFilePlacesView->setUrl(pUrl);
}

void BBFileSystemTab::addManualUrl(const KUrl &pUrl)
{
	KFileItem lItem(KFileItem::Unknown, KFileItem::Unknown, pUrl, true);
	if(lItem.isFile() && lItem.isReadable())
		gMainWindow->addToPlayQueue(lItem, displayString(lItem), tabNumber());
}
