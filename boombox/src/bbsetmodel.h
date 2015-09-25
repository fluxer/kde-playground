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
#ifndef BBSETMODEL_H
#define BBSETMODEL_H

#include <QAbstractItemModel>
#include <QAbstractListModel>
#include <QSet>
#include <QStringList>

#include "bblistitem.h"

class BBCollectionTab;


class BBStringSetModel : public QAbstractListModel
{
public:
	BBStringSetModel(QObject *pParent)
	   : QAbstractListModel(pParent)
	{}

	virtual int rowCount(const QModelIndex & /*parent = QModelIndex()*/ ) const
	{ return mList.count();}
	virtual QVariant data(const QModelIndex &pIndex, int pRole = Qt::DisplayRole) const;
	void setNewSet(const QSet<BBStringListItem> &pNewSet);
	int findStringPos(const QString &pString)
	{
		return mList.indexOf(pString);
	}

protected:
	QSet<BBStringListItem> mSet;
	QStringList mList;
};


class BBAlbumSongModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	BBAlbumSongModel(QObject *pParent, BBCollectionTab *pCollectionTab)
	   : QAbstractItemModel(pParent), mLastShuffleItem(), mFirstShuffleItem(),
	     mCurrentlyPlayingSongID(-1), mCurrentlyPlayingAlbumID(-1), mCollectionTab(pCollectionTab)
	{}

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
	{
		Q_UNUSED(section);
		Q_UNUSED(orientation);
		Q_UNUSED(role);
		return QVariant();
	}

	virtual int columnCount(const QModelIndex &pParent = QModelIndex()) const
	{
		Q_UNUSED(pParent);
		return 1;
	}
	virtual QModelIndex index(int pRow, int pColumn = 0, const QModelIndex &pParent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex & pIndex) const;
	virtual int rowCount(const QModelIndex & pParent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &pIndex, int pRole = Qt::DisplayRole) const;

	void setCurrentSong(int pCurrentSong, int pCurrentAlbum, QModelIndex pPreviousIndex);
	void updateSongs(const QSet<BBSongListItem> &pSongsToUpdate) { mSongsToUpdate = pSongsToUpdate; }
	void setManuallyExpanded(const QModelIndex &pIndex, bool pManuallyExpanded);
	bool isManuallyExpanded(const QModelIndex &pIndex);

	QSet<BBSongListItem> allSongs() {return mSongSet;}
	BBSongListItem findSongItem(QModelIndex pIndex);
	QModelIndex indexForFileID(int pFileID);

	QPersistentModelIndex nextLinear(const QPersistentModelIndex &pIndex);
	QPersistentModelIndex nextShuffled(const QPersistentModelIndex &pIndex);
	QPersistentModelIndex prevLinear(const QPersistentModelIndex &pIndex);
	QPersistentModelIndex prevShuffled(const QPersistentModelIndex &pIndex);
	QPersistentModelIndex firstLinear();
	QPersistentModelIndex firstShuffled();
	QPersistentModelIndex lastLinear();
	QPersistentModelIndex lastShuffled();

public slots:
	void setNewSongs(const QSet<BBSongListItem> &pNewSongs, const QSet<BBAlbumListItem> &pNewAlbums);

protected:
	int findAlbumRow(int pAlbumID) const;
	BBSongData &findSongData(const BBIndex &pIndex)
	{
		return mAlbumList[pIndex.mAlbumPos].mSongs[pIndex.mSongPos];
	}
	void shuffle();

	QSet<BBSongListItem> mSongSet;
	QSet<BBAlbumListItem> mAlbumSet;
	QList<BBAlbumData> mAlbumList;
	BBIndex mLastShuffleItem, mFirstShuffleItem;
	int mCurrentlyPlayingSongID, mCurrentlyPlayingAlbumID;
	BBCollectionTab *mCollectionTab;
	QSet<BBSongListItem> mSongsToUpdate;
};

#endif
