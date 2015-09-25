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
#include "bbmetadata.h"

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <QFileInfo>
#include <QTime>

BBMetaData::BBMetaData(const BBMetaData &pMetaData)
{
	mPath = pMetaData.mPath;
	mFileID = pMetaData.mFileID;
	mTitle = pMetaData.mTitle;
	mArtist = pMetaData.mArtist;
	mAlbum = pMetaData.mAlbum;
	mLength = pMetaData.mLength;
	mYear = pMetaData.mYear;
	mGenre = pMetaData.mGenre;
	mTrack = pMetaData.mTrack;
	mComment = pMetaData.mComment;
}

void BBMetaData::GetInfoFrom(const QString &pPath, const QString &pFileName)
{
	mPath = pPath;
	mFileName = pFileName;
	QString lTemp = pFileName.toLower();
	if(lTemp.endsWith(".ogg") || lTemp.endsWith(".mp3") || lTemp.endsWith(".mpc") || lTemp.endsWith(".flac"))
	{
		TagLib::FileRef lFile(QFile::encodeName(QString("%1/%2").arg(pPath, pFileName)));
		TagLib::Tag *lTag = lFile.tag();
		TagLib::AudioProperties *lAudio = lFile.audioProperties();

		if(lTag != NULL)
		{
			mTitle = QString::fromUtf8(lTag->title().toCString(true)).simplified();
			mArtist = QString::fromUtf8(lTag->artist().toCString(true)).simplified();
			mAlbum = QString::fromUtf8(lTag->album().toCString(true)).simplified();
			mGenre = QString::fromUtf8(lTag->genre().toCString(true)).simplified();
			mComment = QString::fromUtf8(lTag->comment().toCString(true)).simplified();
			mYear = QString::number(lTag->year());
			mTrack = QString::number(lTag->track());
		}
		if(lAudio != NULL)
		{
			QTime lTime(0, lAudio->length()/60, lAudio->length()%60);
			mLength = lTime.toString("mm:ss");
		}
	}
	CheckInfo();
}

void BBMetaData::CheckInfo()
{
	QString lReplacer("#-UNKNOWN-#");

	if(mTitle.isEmpty())
	{
		QFileInfo fi(mPath);
		mTitle = fi.baseName();
		mTitle.replace('_', " ");
	}
	if(mArtist.isEmpty())
		mArtist = lReplacer;
	if(mAlbum.isEmpty())
		mAlbum = lReplacer;
	if(mGenre.isEmpty())
		mGenre = lReplacer;
	if(mYear == "0")
		mYear = lReplacer;
	if(mTrack == "0")
		mTrack = lReplacer;
}

void BBMetaData::ChangeFile(const QString &pPath, const QString &pCategory, const QString &pNewValue)
{
	QString lTemp = pPath.toLower();
	if(!(lTemp.endsWith(".ogg") || lTemp.endsWith(".mp3") || lTemp.endsWith(".mpc") || lTemp.endsWith(".flac")))
		return;

	TagLib::FileRef lFile(QFile::encodeName(pPath));
	TagLib::Tag *lTag = lFile.tag();

	if(lTag != NULL)
	{
		TagLib::String lNewValue(pNewValue.toUtf8().data(), TagLib::String::UTF8);

		if(pCategory == "title")
			lTag->setTitle(lNewValue);
		else if(pCategory == "artist")
			lTag->setArtist(lNewValue);
		else if(pCategory == "album")
			lTag->setAlbum(lNewValue);
		else if(pCategory == "comment")
			lTag->setComment(lNewValue);
		else if(pCategory == "genre")
			lTag->setGenre(lNewValue);
		else if(pCategory == "year")
			lTag->setYear(pNewValue.toUInt());
		else if(pCategory == "track")
			lTag->setTrack(pNewValue.toUInt());

		lFile.save();
	}
}

void BBMetaData::WriteInfoToFile()
{
	TagLib::FileRef lFile(QFile::encodeName(QString("%1/%2").arg(mPath, mFileName)));
	TagLib::Tag *lTag = lFile.tag();

	if(lTag != NULL)
	{
		lTag->setTitle(TagLib::String(mTitle.toUtf8().data(), TagLib::String::UTF8));
		lTag->setArtist(TagLib::String(mArtist.toUtf8().data(), TagLib::String::UTF8));
		lTag->setAlbum(TagLib::String(mAlbum.toUtf8().data(), TagLib::String::UTF8));
		lTag->setComment(TagLib::String(mComment.toUtf8().data(), TagLib::String::UTF8));
		lTag->setGenre(TagLib::String(mGenre.toUtf8().data(), TagLib::String::UTF8));
		lTag->setYear(mYear.toUInt());
		lTag->setTrack(mTrack.toUInt());
		lFile.save();
	}
}
