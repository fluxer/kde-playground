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
#include "bbsetmodel.h"
#include "bbcollectiontab.h"

#include <QtAlgorithms>
#include <KLocale>
#include <KRandom>
#include <QFont>

QVariant BBStringSetModel::data(const QModelIndex &pIndex, int pRole) const
{
	if(!pIndex.isValid())
		return QVariant();

	if(pIndex.row() >= mList.size())
		return QVariant();

	if(pRole == Qt::DisplayRole)
		return mList.at(pIndex.row());
	else
		return QVariant();
}

void BBStringSetModel::setNewSet(const QSet<BBStringListItem> &pNewSet)
{
	QList<BBStringListItem> lOldSongs = (mSet - pNewSet).toList();
	qSort(lOldSongs.begin(), lOldSongs.end(), qGreater<BBStringListItem>());

	int lStartPos = -1, lEndPos = -1;
	QListIterator<BBStringListItem> i(lOldSongs);
	if(i.hasNext())
		lStartPos = lEndPos = i.next().mPosition;
	while(i.hasNext())
	{
		int lNextPos = i.next().mPosition;
		if(lNextPos == lStartPos - 1)
			lStartPos--;
		else
		{
			beginRemoveRows(QModelIndex(), lStartPos, lEndPos);
			for(int j = lEndPos; j>=lStartPos; --j)
				mList.removeAt(j);
			endRemoveRows();
			lStartPos = lEndPos = lNextPos;
		}
	}
	if(lStartPos >= 0)
	{
		beginRemoveRows(QModelIndex(), lStartPos, lEndPos);
		for(int j = lEndPos; j>=lStartPos; --j)
			mList.removeAt(j);
		endRemoveRows();
	}

	QList<BBStringListItem> lNewSongs = (pNewSet - mSet).toList();
	qSort(lNewSongs.begin(), lNewSongs.end());

	lStartPos = -1, lEndPos = -1;
	i = lNewSongs;
	if(i.hasNext())
		lStartPos = lEndPos = i.next().mPosition;
	while(i.hasNext())
	{
		int lNextPos = i.next().mPosition;
		if(lNextPos == lEndPos + 1)
			lEndPos++;
		else
		{
			beginInsertRows(QModelIndex(), lStartPos, lEndPos);
			for(int j = lStartPos; j<=lEndPos; ++j)
				mList.insert(j, lNewSongs.takeFirst().mString);
			endInsertRows();
			lStartPos = lEndPos = lNextPos;
		}
	}
	if(lStartPos >= 0)
	{
		beginInsertRows(QModelIndex(), lStartPos, lEndPos);
		for(int j = lStartPos; j<=lEndPos; ++j)
			mList.insert(j, lNewSongs.takeFirst().mString);
		endInsertRows();
	}

	mSet = pNewSet;
}

int BBAlbumSongModel::rowCount(const QModelIndex & pParent) const
{
	if(!pParent.isValid())
		return mAlbumList.count();
	else
	{
		if(pParent.internalPointer() != NULL || pParent.row() >= mAlbumList.count() || pParent.row() < 0)
			return 0;
		return mAlbumList.at(pParent.row()).mSongs.count();
	}
}

QVariant BBAlbumSongModel::data(const QModelIndex &pIndex, int pRole) const
{
	if(!pIndex.isValid() || pIndex.column() != 0)
		return QVariant();

	if(pIndex.internalPointer() == NULL) //this is an album item
	{
		if(pIndex.row() >= mAlbumList.size() || pIndex.row() < 0)
			return QVariant();

		const BBAlbumData &lData = mAlbumList.at(pIndex.row());
		switch(pRole)
		{
			case Qt::FontRole:
				if(lData.mAlbumID == mCurrentlyPlayingAlbumID)
				{
					QFont lFont = mCollectionTab->font();
					lFont.setBold(true);
					return lFont;
				}
				else
					return QVariant();
			case BBAlbumRole:
				return lData.mAlbum;
			case BBArtistRole:
				return lData.mArtist;
			case BBPathRole:
				return lData.mFolderPath;
			default:
				return QVariant();
		}
	}
	else // this is a song item
	{
		BBAlbumData *lAlbumData = (BBAlbumData *) pIndex.internalPointer();
		if(pIndex.row() >= lAlbumData->mSongs.count())
			return QVariant();

		const BBSongData &lSongData = lAlbumData->mSongs.at(pIndex.row());
		switch(pRole)
		{
		case Qt::FontRole:
			if(lSongData.mFileID == mCurrentlyPlayingSongID)
			{
				QFont lFont = mCollectionTab->font();
				lFont.setBold(true);
				return lFont;
			}
			else
				return QVariant();
		case Qt::DisplayRole:
			if(lAlbumData->mIsVA)
				return QString("%1 - %2").arg(lSongData.mArtist, lSongData.mTitle);
		case BBTitleRole: //fall through
			return lSongData.mTitle;
		case BBArtistRole:
			if(lAlbumData->mIsVA)
				return lSongData.mArtist;
			else
				return lAlbumData->mArtist;
		case BBPathRole:
			return lAlbumData->mFolderPath;
		case BBFileNameRole:
			return lSongData.mFileName;
		case BBLengthRole:
			return lSongData.mLength;
		case BBFileIDRole:
			return lSongData.mFileID;
		case BBAlbumIDRole:
			return lAlbumData->mAlbumID;
		default:
			return QVariant();
		}
	}
}

QModelIndex BBAlbumSongModel::index(int pRow, int pColumn, const QModelIndex &pParent) const
{
	if(!pParent.isValid()) //this is an album item
	{
		if(pRow >= mAlbumList.count() || pRow < 0 || pColumn != 0)
			return QModelIndex();
		return createIndex(pRow, pColumn, (void *)NULL);
	}
	else //this is a song item
	{
		if(pParent.row() >= mAlbumList.count())
			return QModelIndex();
		const BBAlbumData &lAlbumData = mAlbumList.at(pParent.row());
		if(pRow >= lAlbumData.mSongs.count() || pRow < 0 || pColumn != 0)
			return QModelIndex();
		return createIndex(pRow, pColumn, (void *)&lAlbumData);
	}
}

QModelIndex BBAlbumSongModel::parent(const QModelIndex &pIndex) const
{
	if(!pIndex.isValid() || pIndex.internalPointer() == NULL)
		return QModelIndex();
	else
	{
		BBAlbumData *lAlbumData = (BBAlbumData *)pIndex.internalPointer();
		return createIndex(findAlbumRow(lAlbumData->mAlbumID), 0, (void *)NULL);
	}
}

void BBAlbumSongModel::setNewSongs(const QSet<BBSongListItem> &pNewSongs, const QSet<BBAlbumListItem> &pNewAlbums)
{
	//remove songs to be updated now, added again later in this method.
	QList<BBSongListItem> lOldSongs = (mSongSet - pNewSongs + mSongsToUpdate).toList();
	// sort in reverse order so that removal starts in end of list, keeps indexes correct.
	qSort(lOldSongs.begin(), lOldSongs.end(), qGreater<BBSongListItem>());

	int lStartPos = -1, lEndPos = -1, lCurrentAlbumPos = -1;

	QListIterator<BBSongListItem> i(lOldSongs);
	if(i.hasNext())
	{
		const BBSongListItem &lNext = i.next();
		lStartPos = lEndPos = lNext.mIndex.mSongPos;
		lCurrentAlbumPos = lNext.mIndex.mAlbumPos;
	}
	while(i.hasNext())
	{
		const BBSongListItem &lNext = i.next();
		int lNextPos = lNext.mIndex.mSongPos;
		int lNextAlbumPos = lNext.mIndex.mAlbumPos;

		if(lNextPos == lStartPos - 1 && lCurrentAlbumPos == lNextAlbumPos)
			lStartPos--;
		else
		{
			beginRemoveRows(createIndex(lCurrentAlbumPos, 0, (void *)NULL), lStartPos, lEndPos);
			for(int j = lEndPos; j>=lStartPos; --j)
				mAlbumList[lCurrentAlbumPos].mSongs.removeAt(j);
			endRemoveRows();
			lStartPos = lEndPos = lNextPos;
			lCurrentAlbumPos = lNextAlbumPos;
		}
	}
	if(lStartPos >= 0)
	{
		beginRemoveRows(createIndex(lCurrentAlbumPos, 0, (void *)NULL), lStartPos, lEndPos);
		for(int j = lEndPos; j>=lStartPos; --j)
			mAlbumList[lCurrentAlbumPos].mSongs.removeAt(j);
		endRemoveRows();
	}

	QList<BBAlbumListItem> lOldAlbums = (mAlbumSet - pNewAlbums).toList();
	// sort in reverse order so that removal starts in end of list, keeps indexes correct.
	qSort(lOldAlbums.begin(), lOldAlbums.end(), qGreater<BBAlbumListItem>());

	lStartPos = -1;
	lEndPos = -1;

	QListIterator<BBAlbumListItem> i2(lOldAlbums);
	if(i2.hasNext())
		lStartPos = lEndPos = i2.next().mPosition;
	while(i2.hasNext())
	{
		int lNextPos = i2.next().mPosition;
		if(lNextPos == lStartPos - 1)
			lStartPos--;
		else
		{
			beginRemoveRows(QModelIndex(), lStartPos, lEndPos);
			for(int j = lEndPos; j>=lStartPos; --j)
				mAlbumList.removeAt(j);
			endRemoveRows();
			lStartPos = lEndPos = lNextPos;
		}
	}
	if(lStartPos >= 0)
	{
		beginRemoveRows(QModelIndex(),lStartPos, lEndPos);
		for(int j = lEndPos; j>=lStartPos; --j)
			mAlbumList.removeAt(j);
		endRemoveRows();
	}

	//removing updated songs from sets here will cause them to be added
	mSongSet -= mSongsToUpdate;

	QList<BBAlbumListItem> lNewAlbums = (pNewAlbums- mAlbumSet).toList();
	qSort(lNewAlbums.begin(), lNewAlbums.end());

	lStartPos = -1;
	lEndPos = -1;
	i2 = lNewAlbums;
	if(i2.hasNext())
		lStartPos = lEndPos = i2.next().mPosition;
	while(i2.hasNext())
	{
		int lNextPos = i2.next().mPosition;
		if(lNextPos == lEndPos + 1)
			lEndPos++;
		else
		{
			beginInsertRows(QModelIndex(), lStartPos, lEndPos);
			for(int j = lStartPos; j<=lEndPos; ++j)
				mAlbumList.insert(j, lNewAlbums.takeFirst().mData);
			endInsertRows();
			lStartPos = lEndPos = lNextPos;
		}
	}
	if(lStartPos >= 0)
	{
		beginInsertRows(QModelIndex(), lStartPos, lEndPos);
		for(int j = lStartPos; j<=lEndPos; ++j)
			mAlbumList.insert(j, lNewAlbums.takeFirst().mData);
		endInsertRows();
	}

	QList<BBSongListItem> lNewSongs = (pNewSongs - mSongSet).toList();
	qSort(lNewSongs.begin(), lNewSongs.end());

	lStartPos = -1;
	lEndPos = -1;
	lCurrentAlbumPos = -1;
	i = lNewSongs;
	if(i.hasNext())
	{
		const BBSongListItem &lNext = i.next();
		lStartPos = lEndPos = lNext.mIndex.mSongPos;
		lCurrentAlbumPos = lNext.mIndex.mAlbumPos;
	}
	while(i.hasNext())
	{
		const BBSongListItem &lNext = i.next();
		int lNextPos = lNext.mIndex.mSongPos;
		int lNextAlbumPos = lNext.mIndex.mAlbumPos;

		if(lNextPos == lEndPos + 1 && lCurrentAlbumPos == lNextAlbumPos)
			lEndPos++;
		else
		{
			beginInsertRows(createIndex(lCurrentAlbumPos, 0, (void *)NULL), lStartPos, lEndPos);
			for(int j = lStartPos; j<=lEndPos; ++j)
				mAlbumList[lCurrentAlbumPos].mSongs.insert(j, lNewSongs.takeFirst().mData);
			endInsertRows();
			lStartPos = lEndPos = lNextPos;
			lCurrentAlbumPos = lNextAlbumPos;
		}
	}
	if(lStartPos >= 0)
	{
		beginInsertRows(createIndex(lCurrentAlbumPos, 0, (void *)NULL), lStartPos, lEndPos);
		for(int j = lStartPos; j<=lEndPos; ++j)
			mAlbumList[lCurrentAlbumPos].mSongs.insert(j, lNewSongs.takeFirst().mData);
		endInsertRows();
	}

	mAlbumSet = pNewAlbums;
	mSongSet = pNewSongs;
	shuffle();
}

void BBAlbumSongModel::shuffle()
{
	if(mSongSet.isEmpty())
	{
		mLastShuffleItem = mFirstShuffleItem = BBIndex();
		return;
	}
	QList<BBIndex> lTempList;
	for(int i = 0; i < mAlbumList.count(); ++i)
	{
		int max = mAlbumList.at(i).mSongs.count();
		for(int j = 0; j < max; ++j)
			lTempList.append(BBIndex(i, j));
	}

	BBIndex lRandomPos = lTempList.takeAt(KRandom::random() % lTempList.count());
	mLastShuffleItem = mFirstShuffleItem = lRandomPos;
	findSongData(mFirstShuffleItem).mPrevShuffled = BBIndex();

	while(!lTempList.isEmpty())
	{
		lRandomPos = lTempList.takeAt(KRandom::random() % lTempList.count());
		findSongData(mLastShuffleItem).mNextShuffled = lRandomPos;
		findSongData(lRandomPos).mPrevShuffled = mLastShuffleItem;
		mLastShuffleItem = lRandomPos;
	}
	findSongData(mLastShuffleItem).mNextShuffled = BBIndex();
}

QPersistentModelIndex BBAlbumSongModel::nextLinear(const QPersistentModelIndex &pIndex)
{
	if(!pIndex.isValid())
		return QModelIndex();
	BBAlbumData *lAlbumData = (BBAlbumData *)pIndex.internalPointer();
	int lNextSongPos = pIndex.row() + 1;
	if(lNextSongPos >= lAlbumData->mSongs.count())
	{
		lNextSongPos = 0;
		int lNextAlbumPos = findAlbumRow(lAlbumData->mAlbumID) + 1;
		if(lNextAlbumPos >= mAlbumList.count())
			return QModelIndex();
		lAlbumData = (BBAlbumData *) &mAlbumList.at(lNextAlbumPos);
	}
	return createIndex(lNextSongPos, 0, (void *)lAlbumData);
}

QPersistentModelIndex BBAlbumSongModel::nextShuffled(const QPersistentModelIndex &pIndex)
{
	if(!pIndex.isValid())
		return QModelIndex();
	BBAlbumData *lAlbumData = (BBAlbumData *)pIndex.internalPointer();
	BBIndex lNextPos = lAlbumData->mSongs.at(pIndex.row()).mNextShuffled;
	if(!lNextPos.isValid())
		return QModelIndex();
	return createIndex(lNextPos.mSongPos, 0, (void *)&mAlbumList.at(lNextPos.mAlbumPos));
}

QPersistentModelIndex BBAlbumSongModel::prevLinear(const QPersistentModelIndex &pIndex)
{
	if(!pIndex.isValid())
		return QModelIndex();
	BBAlbumData *lAlbumData = (BBAlbumData *)pIndex.internalPointer();
	int lNextSongPos = pIndex.row() - 1;
	if(lNextSongPos < 0)
	{
		int lNextAlbumPos = findAlbumRow(lAlbumData->mAlbumID) - 1;
		if(lNextAlbumPos < 0)
			return QModelIndex();
		lAlbumData = (BBAlbumData *) &mAlbumList.at(lNextAlbumPos);
		lNextSongPos = lAlbumData->mSongs.count() - 1;
	}
	return createIndex(lNextSongPos, 0, (void *)lAlbumData);
}

QPersistentModelIndex BBAlbumSongModel::prevShuffled(const QPersistentModelIndex &pIndex)
{
	if(!pIndex.isValid())
		return QModelIndex();
	BBAlbumData *lAlbumData = (BBAlbumData *)pIndex.internalPointer();
	BBIndex lNextPos = lAlbumData->mSongs.at(pIndex.row()).mPrevShuffled;
	if(!lNextPos.isValid())
		return QModelIndex();
	return createIndex(lNextPos.mSongPos, 0, (void *)&mAlbumList.at(lNextPos.mAlbumPos));
}

QPersistentModelIndex BBAlbumSongModel::firstLinear()
{
	if(mSongSet.isEmpty())
		return QModelIndex();
	return createIndex(0, 0, (void *)&mAlbumList.first());
}

QPersistentModelIndex BBAlbumSongModel::firstShuffled()
{
	if(!mFirstShuffleItem.isValid())
		return QModelIndex();
	return createIndex(mFirstShuffleItem.mSongPos, 0, (void *)&mAlbumList.at(mFirstShuffleItem.mAlbumPos));
}

QPersistentModelIndex BBAlbumSongModel::lastLinear()
{
	if(mSongSet.isEmpty())
		return QModelIndex();

	int lSongPos = mAlbumList.last().mSongs.count() - 1;
	return createIndex(lSongPos, 0, (void *)&mAlbumList.last());
}

QPersistentModelIndex BBAlbumSongModel::lastShuffled()
{
	if(!mLastShuffleItem.isValid())
		return QModelIndex();
	return createIndex(mLastShuffleItem.mSongPos, 0, (void *)&mAlbumList.at(mLastShuffleItem.mAlbumPos));
}

void BBAlbumSongModel::setCurrentSong(int pCurrentSong, int pCurrentAlbum, QModelIndex pPreviousIndex)
{
	mCurrentlyPlayingSongID = pCurrentSong;
	mCurrentlyPlayingAlbumID = pCurrentAlbum;

	QModelIndex lCurrentIndex = indexForFileID(pCurrentSong);
	if(lCurrentIndex.isValid() && lCurrentIndex.column() == 0 && lCurrentIndex.internalPointer() != NULL)
	{
		emit dataChanged(lCurrentIndex, lCurrentIndex);
		QModelIndex lAlbumIndex = lCurrentIndex.parent();
		emit dataChanged(lAlbumIndex, lAlbumIndex);
	}
	if(pPreviousIndex.isValid() && pPreviousIndex.column() == 0 && pPreviousIndex.internalPointer() != NULL)
	{
		emit dataChanged(pPreviousIndex, pPreviousIndex);
		QModelIndex lAlbumIndex = pPreviousIndex.parent();
		emit dataChanged(lAlbumIndex, lAlbumIndex);
	}
}

void BBAlbumSongModel::setManuallyExpanded(const QModelIndex &pIndex, bool pManuallyExpanded)
{
	if(!pIndex.isValid() || pIndex.column() != 0 || pIndex.internalPointer() != NULL)
		return;
	if(pIndex.row() >= mAlbumList.count())
		return;
	mAlbumList[pIndex.row()].mManuallyExpanded = pManuallyExpanded;
}

bool BBAlbumSongModel::isManuallyExpanded(const QModelIndex &pIndex)
{
	if(!pIndex.isValid() || pIndex.column() != 0 || pIndex.internalPointer() != NULL)
		return false;
	if(pIndex.row() >= mAlbumList.count())
		return false;
	return mAlbumList[pIndex.row()].mManuallyExpanded;
}

int BBAlbumSongModel::findAlbumRow(int pAlbumID) const
{
	BBAlbumListItem lItem;
	lItem.mData.mAlbumID = pAlbumID;
	QSet<BBAlbumListItem>::const_iterator lIter = mAlbumSet.find(lItem);
	return (*lIter).mPosition;
}

BBSongListItem BBAlbumSongModel::findSongItem(QModelIndex pIndex)
{
	BBSongListItem lItem;
	lItem.mData.mFileID = pIndex.data(BBFileIDRole).toInt();
	QSet<BBSongListItem>::const_iterator lIter = mSongSet.find(lItem);
	return *lIter;
}

QModelIndex BBAlbumSongModel::indexForFileID(int pFileID)
{
	BBSongListItem lItem;
	lItem.mData.mFileID = pFileID;
	QSet<BBSongListItem>::const_iterator lIter = mSongSet.find(lItem);
	if(lIter == mSongSet.constEnd())
		return QModelIndex();
	else
		return index((*lIter).mIndex.mSongPos, 0, index((*lIter).mIndex.mAlbumPos));
}
