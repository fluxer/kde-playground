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
#ifndef BBLISTITEM_H
#define BBLISTITEM_H

#include <QHash>
#include <QList>

class BBIndex
{
	public:
		BBIndex()
		: mAlbumPos(-1), mSongPos(-1)
		{}
		BBIndex(int pAlbumPos, int pSongPos)
		: mAlbumPos(pAlbumPos), mSongPos(pSongPos)
		{}
		bool isValid() {return mAlbumPos >= 0 && mSongPos >= 0;}
		int mAlbumPos, mSongPos;
};

class BBSongData
{
	public:
		BBSongData()
		: mNextShuffled(), mPrevShuffled()
		{
			mFileID = -1;
			mAlbumID = -1;
		}
		QString mTitle, mArtist, mLength, mFileName;
		int mFileID, mAlbumID;
		BBIndex mNextShuffled, mPrevShuffled;
};

class BBAlbumData
{
	public:
		QString mAlbum, mFolderPath, mCoverArtPath, mArtist;
		int mAlbumID;
		bool mIsVA;
		bool mManuallyExpanded;
		QList<BBSongData> mSongs;
};

class BBListItem
{
	public:
		BBListItem(int pPosition)
			: mPosition(pPosition)
		{}
		BBListItem()
			: mPosition(-1)
		{}

		int mPosition;
};

class BBStringListItem : public BBListItem
{
	public:
		BBStringListItem(const QString & pString, int pPosition)
			: BBListItem(pPosition), mString(pString)
		{}

		QString mString;
};

class BBSongListItem : public BBListItem
{
	public:
		BBSongData mData;
		BBIndex mIndex;
};

class BBAlbumListItem : public BBListItem
{
	public:
		BBAlbumData mData;
};

inline bool operator< (const BBListItem &a, const BBListItem &b)
{
	return a.mPosition < b.mPosition;
}

inline bool operator> (const BBListItem &a, const BBListItem &b)
{
	return a.mPosition > b.mPosition;
}

inline uint qHash(const BBStringListItem &a)
{
	return qHash(a.mString);
}

inline bool operator==(const BBStringListItem &a, const BBStringListItem &b)
{
	return a.mString == b.mString;
}

inline uint qHash(const BBAlbumListItem &a)
{
	return a.mData.mAlbumID;
}

inline bool operator==(const BBAlbumListItem &a, const BBAlbumListItem &b)
{
	return a.mData.mAlbumID == b.mData.mAlbumID;
}

inline uint qHash(const BBSongListItem &a)
{
	return a.mData.mFileID;
}

inline bool operator==(const BBSongListItem &a, const BBSongListItem &b)
{
	return a.mData.mFileID == b.mData.mFileID;
}

#endif
