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
#include "dbqueryjob.h"
#include "bbdatabase.h"
#include "bbsettings.h"

#include <KLocale>

DBPool::DBPool(int pNumConnections)
	: mNumConnections(pNumConnections)
{
	for(int i = 0; i < mNumConnections; ++i)
		mAvailable.push(new BBDatabase());
}

DBPool::~DBPool()
{
	foreach(BBDatabase *i, mAvailable)
	{
		delete i;
	}
}

bool DBPool::canRun(ThreadWeaver::Job *pJob)
{
	if(mAvailable.isEmpty())
		return false;
	else
	{
		DBQueryJob * lJob = qobject_cast<DBQueryJob *>(pJob);
		if(lJob == NULL)
			return false;
		if(lJob->mDatabase != NULL)
			return true;
		lJob->mDatabase = mAvailable.pop();
		if(!lJob->mDatabase->isConnected())
		{
			if(!lJob->mDatabase->connect(BBSettings::self()->fileNameDB()))
				return false;
		}
		return true;
	}
}

void DBPool::free(ThreadWeaver::Job *pJob)
{
	DBQueryJob * lJob = qobject_cast<DBQueryJob *>(pJob);
	if(lJob == NULL)
		return;
	if(lJob->mDatabase != NULL)
		mAvailable.push(lJob->mDatabase);
	lJob->mDatabase = NULL;
}

void FilterQueryJob::run()
{
	BBResultTable lDBResult;
	QSet<BBStringListItem> lResult;

	if(mDatabase->getTable(QString("SELECT DISTINCT %1 FROM songs JOIN albums USING (album_ID) %2 ORDER BY %1 ASC"
	                               ).arg(mCategory, mCondition.isEmpty() ? mCondition: QString("WHERE %1").arg(mCondition)), lDBResult))
	{
		for(int i = 1; i <= lDBResult.mNumRows; ++i)//start with 1 since first row is column names
		{
			if(mShouldAbort)
			{
				BBDatabase::freeTable(lDBResult);
				return;
			}
			lResult.insert(BBStringListItem(QString::fromUtf8(lDBResult.at(i, 0)), i-1));
		}
		BBDatabase::freeTable(lDBResult);
	}

	if(!mShouldAbort)
		emit resultReady(lResult);
}

void SongQueryJob::getSongs(bool pVA)
{
	BBResultTable lDBResult;
	QString lQuery;
	if(pVA)
	{
		lQuery = QString("SELECT title, artist, length, file_name, file_ID, album_ID, album, "
		                 "path, cover_art_path FROM songs JOIN albums USING (album_ID) WHERE is_VA != 0 %1 "
		                 "ORDER BY album, path, track, file_name"
		                 ).arg(mCondition.isEmpty() ? mCondition : QString("AND %1").arg(mCondition));
	}
	else
	{
		lQuery = QString("SELECT title, artist, length, file_name, file_ID, album_ID, album, "
		                 "path, cover_art_path FROM songs JOIN albums USING (album_ID) WHERE is_VA = 0 %1 "
		                 "ORDER BY artist, album, path, track, file_name"
		                 ).arg(mCondition.isEmpty() ? mCondition : QString("AND %1").arg(mCondition));
	}
	if(mDatabase->getTable(lQuery, lDBResult))
	{
		int lCurrentAlbumId = -1; //for use in first run of loop
		int lSongPosition = 0;
		QString lTemp;
		BBAlbumListItem lAlbum;
		BBSongListItem lSong;
		lAlbum.mPosition = -1; //for use in first run of loop
		for(int i = 1; i <= lDBResult.mNumRows; ++i)//start with 1 since first row is column names
		{
			if(mShouldAbort)
			{
				BBDatabase::freeTable(lDBResult);
				return;
			}
			lSong.mData.mTitle = QString::fromUtf8(lDBResult.at(i,0));
			if(pVA)
				lSong.mData.mArtist = QString::fromUtf8(lDBResult.at(i,1));
			lSong.mData.mLength = QString::fromUtf8(lDBResult.at(i,2));
			lSong.mData.mFileName = QString::fromUtf8(lDBResult.at(i,3));
			lTemp = QString::fromUtf8(lDBResult.at(i,4));
			lSong.mData.mFileID = lTemp.toInt();
			lTemp = QString::fromUtf8(lDBResult.at(i,5));
			lSong.mData.mAlbumID = lTemp.toInt();

			if(lCurrentAlbumId != lSong.mData.mAlbumID)
			{
				if(lAlbum.mPosition >= 0)
				{
					mAlbumSet.insert(lAlbum);
					lSongPosition = 0;
				}
				lAlbum.mData.mAlbum = QString::fromUtf8(lDBResult.at(i,6));
				lAlbum.mData.mFolderPath = QString::fromUtf8(lDBResult.at(i,7));
				lAlbum.mData.mCoverArtPath= QString::fromUtf8(lDBResult.at(i,8));
				lAlbum.mData.mIsVA = pVA;
				lAlbum.mData.mArtist = pVA ? i18n("Various Artists") : QString::fromUtf8(lDBResult.at(i,1));
				lAlbum.mPosition = mAlbumSet.count();
				lAlbum.mData.mAlbumID = lSong.mData.mAlbumID;
				lCurrentAlbumId = lSong.mData.mAlbumID;
				lAlbum.mData.mManuallyExpanded = false; //initially all albums are not expanded
			}

			lSong.mIndex.mSongPos = lSongPosition++;
			lSong.mIndex.mAlbumPos = mAlbumSet.count();
			lSong.mPosition = mSongSet.count();
			mSongSet.insert(lSong);
		}
		if(lAlbum.mPosition >= 0)
			mAlbumSet.insert(lAlbum);

		BBDatabase::freeTable(lDBResult);
	}
}

void SongQueryJob::run()
{
	getSongs(false);
	if(mShouldAbort)
		return;
	getSongs(true);
	if(!mShouldAbort)
		emit resultReady(mSongSet, mAlbumSet);
}

#include "dbqueryjob.moc"
