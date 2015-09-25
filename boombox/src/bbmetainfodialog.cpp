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
#include "bbmetainfodialog.h"
#include "kmessagebox.h"
#include "ui_bbmetainfodialog.h"
#include "bbmetadata.h"
#include "bbsetmodel.h"
#include "bbsettings.h"
#include "bbcollectiontab.h"

#include <sqlite3.h>

BBMetaInfoDialog::BBMetaInfoDialog(QWidget *parent, BBCollectionTab *pCollectionTab):
	QDialog(parent), mUI(new Ui::BBMetaInfoDialog), mCollectionTab(pCollectionTab)
{
	mUI->setupUi(this);
	connect(this, SIGNAL(accepted()), this, SLOT(saveValues()));
	connect(mUI->mGuessButton, SIGNAL(clicked()), this, SLOT(guessFieldNames()));
}

BBMetaInfoDialog::~BBMetaInfoDialog()
{
	delete mUI;
}

void BBMetaInfoDialog::changeEvent(QEvent *e)
{
	QDialog::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		mUI->retranslateUi(this);
		break;
	default:
		break;
	}
}

bool BBMetaInfoDialog::fillInValues(const QModelIndex &pModelIndex)
{
	mIndex = pModelIndex;
	mFileInfo = QFileInfo(QString("%1/%2").arg(pModelIndex.data(BBPathRole).toString(),
	                                           pModelIndex.data(BBFileNameRole).toString()));
	if(!mFileInfo.exists())
	{
		KMessageBox::sorry(this, i18n("Sorry, the file does not exist anymore. You should run an update to keep the BoomBox database in sync with the filesystem."), mFileInfo.canonicalFilePath());
		return false;
	}
	if(!mFileInfo.isWritable())
	{
		KMessageBox::sorry(this, i18n("Sorry, the file is not writable. Make sure you have correct permissions to alter the file and then try again."), mFileInfo.canonicalFilePath());
		return false;
	}
	mMetaData.GetInfoFrom(mFileInfo.absolutePath(), mFileInfo.fileName());
	mUI->mPathDisplay->setText(QString("%1/%2").arg(mMetaData.mPath, mMetaData.mFileName));
	mUI->mArtistEdit->setText(mMetaData.mArtist);
	mUI->mAlbumEdit->setText(mMetaData.mAlbum);
	mUI->mTitleEdit->setText(mMetaData.mTitle);
	mUI->mTrackSpinner->setValue(mMetaData.mTrack.toInt());
	mUI->mYearSpinner->setValue(mMetaData.mYear.toInt());
	mUI->mGenreEdit->setText(mMetaData.mGenre);
	mUI->mCommentEdit->setPlainText(mMetaData.mComment);
	return true;
}

void BBMetaInfoDialog::saveValues()
{
	int lFileID = mIndex.data(BBFileIDRole).toInt();
	bool lValuesChanged = false;
	mDatabase.connect(BBSettings::fileNameDB());
	if(mUI->mArtistEdit->text() != mMetaData.mArtist)
	{
		lValuesChanged = true;
		mMetaData.mArtist = mUI->mArtistEdit->text();
		mDatabase.executeStatement(
			QString("UPDATE songs SET artist = '%1' WHERE file_ID = '%2'").arg(BBDatabase::prepareString(mMetaData.mArtist)).arg(lFileID)
			);
	}
	if(mUI->mTitleEdit->text() != mMetaData.mTitle)
	{
		lValuesChanged = true;
		mMetaData.mTitle = mUI->mTitleEdit->text();
		mDatabase.executeStatement(
			QString("UPDATE songs SET title = '%1' WHERE file_ID = '%2'").arg(BBDatabase::prepareString(mMetaData.mTitle)).arg(lFileID)
			);
	}
	if(mUI->mGenreEdit->text() != mMetaData.mGenre)
	{
		lValuesChanged = true;
		mMetaData.mGenre = mUI->mGenreEdit->text();
		mDatabase.executeStatement(
			QString("UPDATE songs SET genre = '%1' WHERE file_ID = '%2'").arg(BBDatabase::prepareString(mMetaData.mGenre)).arg(lFileID)
			);
	}
	QString lComment = mUI->mCommentEdit->document()->toPlainText();
	if(lComment != mMetaData.mComment)
	{
		lValuesChanged = true;
		mMetaData.mComment = lComment;
		mDatabase.executeStatement(
			QString("UPDATE songs SET comment = '%1' WHERE file_ID = '%2'").arg(BBDatabase::prepareString(mMetaData.mComment)).arg(lFileID)
			);
	}
	QString lTrack = QString::number(mUI->mTrackSpinner->value());
	if(lTrack != mMetaData.mTrack)
	{
		lValuesChanged = true;
		mMetaData.mTrack = lTrack;
		mDatabase.executeStatement(
			QString("UPDATE songs SET track = '%1' WHERE file_ID = '%2'").arg(BBDatabase::prepareString(mMetaData.mTrack)).arg(lFileID)
			);
	}
	QString lYear = QString::number(mUI->mYearSpinner->value());
	if(lYear != mMetaData.mYear)
	{
		lValuesChanged = true;
		mMetaData.mYear = lYear;
		mDatabase.executeStatement(
			QString("UPDATE songs SET year = '%1' WHERE file_ID = '%2'").arg(BBDatabase::prepareString(mMetaData.mYear)).arg(lFileID)
			);
	}
	if(mUI->mAlbumEdit->text() != mMetaData.mAlbum)
	{
		lValuesChanged = true;
		mMetaData.mAlbum = mUI->mAlbumEdit->text();
		int lAlbumID;
		BBResultTable lResult;
		mDatabase.getTable(QString(
			"SELECT DISTINCT album_ID FROM songs JOIN albums USING (album_ID) "
			"WHERE album = '%1' AND path = '%2'").arg(BBDatabase::prepareString(mMetaData.mAlbum),
			                                          BBDatabase::prepareString(mMetaData.mPath)),
			lResult);
		if(lResult.mNumRows == 0)
		{
			mDatabase.executeStatement(QString(
				"INSERT INTO albums (path, album, is_VA, cover_art_path) "
				"VALUES ('%1','%2','%3','%4')").arg(BBDatabase::prepareString(mMetaData.mPath),
				                                    BBDatabase::prepareString(mMetaData.mAlbum),
				                                    QString::number(0), QString())
				);
			lAlbumID = sqlite3_last_insert_rowid(mDatabase.getDatabase());
		}
		else
		{
			lAlbumID = QString::fromUtf8(lResult.at(1, 0)).toInt();
		}
		mDatabase.freeTable(lResult);
		mDatabase.executeStatement(
			QString("UPDATE songs SET album_ID = %1 WHERE file_ID = %2").arg(lAlbumID).arg(lFileID)
			);
	}
	if(lValuesChanged)
	{
		mMetaData.WriteInfoToFile();
		mCollectionTab->refreshAllViewsAndUpdateItem(mIndex);
	}
}

QStringList BBMetaInfoDialog::splitName(QString lName)
{
	QStringList lSplitFileName = lName.split(QRegExp("[\\.-]")); //split on dot or hyphen

	QStringList lFields;
	foreach (QString lField, lSplitFileName)
	{
		QString lSimp = lField.replace('_', ' ').simplified();
		QRegExp lRegExp("^\\d\\d\\s");
		if(lSimp.contains(lRegExp))
		{
			lFields << lSimp.left(2);
			lSimp = lSimp.right(lSimp.length() - 2).simplified();
		}

		lRegExp = QRegExp("\\b(\\w+)\\b");
		QStringList lDontTouch;
		lDontTouch <<"a" <<"an" <<"the" <<"it" <<"this" <<"that" <<"but" <<"and" <<"or" <<"for" <<"so" <<"yet"
		           <<"in" <<"out" <<"on" <<"over" <<"of" <<"off" <<"to" <<"from" <<"by" <<"with";
		int lPos = 0;
		while((lPos = lRegExp.indexIn(lSimp, lPos)) != -1)
		{
			QString lMatch = lRegExp.cap(1).toLower();
			if(!lDontTouch.contains(lMatch) || lPos == 0)
			{
				lMatch.replace(0, 1, lMatch.at(0).toUpper());
			}
			lSimp.replace(lPos, lMatch.length(), lMatch);
			lPos += lRegExp.matchedLength();
		}

		lRegExp = QRegExp("\\s\\d\\d$");
		if(lSimp.contains(lRegExp))
		{
			lFields << lSimp.left(lSimp.length() - 2).simplified();
			lSimp = lSimp.right(2);
		}

		lFields << lSimp;
	}

	if(lFields.last().contains("mp3", Qt::CaseInsensitive) || lFields.last().contains("ogg", Qt::CaseInsensitive)
	   || lFields.last().contains("flac", Qt::CaseInsensitive))
	{
		lFields.takeLast();
	}
	return lFields;
}

void BBMetaInfoDialog::guessFieldNames()
{
	int lYear = -1, lTrack = -1;
	QString lArtist, lAlbum, lTitle;

	QStringList lFileFields = splitName(mFileInfo.fileName());
	QString lFolderName = mFileInfo.canonicalPath();
	int lLastSlash = lFolderName.lastIndexOf('/');
	if(lLastSlash != -1)
		lFolderName.remove(0, lLastSlash + 1);
	QStringList lFolderFields = splitName(lFolderName);

	bool lIsNumber;
	int lFileNumberBitField = 0;
	int lFolderNumberBitField = 0;
	for(int i = 0; i < lFileFields.count(); ++i)
	{
		lFileFields.at(i).toInt(&lIsNumber);
		if(lIsNumber)
			lFileNumberBitField |= 1 << i;
	}
	for(int i = 0; i < lFolderFields.count(); ++i)
	{
		lFolderFields.at(i).toInt(&lIsNumber);
		if(lIsNumber)
			lFolderNumberBitField |= 1 << i;
	}

	switch(lFileFields.count())
	{
	case 1:
		if(lFileNumberBitField != 0)
			lTrack = lFileFields.at(0).toInt();
		else
			lTitle = lFileFields.at(0);
		break;
	case 2:
		switch(lFileNumberBitField)
		{
		case 0:
			lArtist = lFileFields.at(0);
			lTitle = lFileFields.at(1);
			break;
		case 1:
		case 3:
			lTrack = lFileFields.at(0).toInt();
			lTitle = lFileFields.at(1);
			break;
		case 2:
			lAlbum = lFileFields.at(0);
			lTrack = lFileFields.at(1).toInt();
			break;
		}
		break;
	case 3:
		switch(lFileNumberBitField & 3) //only expect number in first two fields, otherwise maybe numeric title or such.
		{
		case 0:
			lArtist = lFileFields.at(0);
			lAlbum = lFileFields.at(1);
			lTitle = lFileFields.at(2);
			break;
		case 1:
		case 3:
			lTrack = lFileFields.at(0).toInt();
			lTitle = lFileFields.at(1);
			break;
		case 2:
			lArtist = lFileFields.at(0);
			lTrack = lFileFields.at(1).toInt();
			lTitle = lFileFields.at(2);
			break;
		}
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		switch(lFileNumberBitField & 7) //only expect number in first three fields, otherwise maybe numeric title or such.
		{
		case 0:
			lArtist = lFileFields.at(0);
			lAlbum = lFileFields.at(1);
			lTitle = lFileFields.at(2);
			break;
		case 1:
		case 3:
		case 5:
		case 7:
			lTrack = lFileFields.at(0).toInt();
			lArtist = lFileFields.at(1);
			lTitle = lFileFields.at(2);
			break;
		case 2:
		case 6:
			lArtist = lFileFields.at(0);
			lYear = lFileFields.at(1).toInt();
			lAlbum = lFileFields.at(2);
			lTitle = lFileFields.at(3);
			break;
		case 4:
			lArtist = lFileFields.at(0);
			lAlbum = lFileFields.at(1);
			lTrack = lFileFields.at(2).toInt();
			lTitle = lFileFields.at(3);
			break;
		}
		break;
	}

	switch(lFolderFields.count())
	{
	case 1:
		if(lFolderNumberBitField == 0)
			lAlbum = lFolderFields.at(0);
		break;
	case 2:
		switch(lFolderNumberBitField)
		{
		case 0:
			if(lArtist.isEmpty())
				lArtist = lFolderFields.at(0);
			if(lAlbum.isEmpty())
				lAlbum = lFolderFields.at(1);
			break;
		case 1:
			if(lYear == -1)
				lYear = lFolderFields.at(0).toInt();
			if(lAlbum.isEmpty())
				lAlbum = lFolderFields.at(1);
			break;
		case 2:
		case 3:
			if(lAlbum.isEmpty())
				lAlbum = lFolderFields.at(0);
			if(lYear == -1)
				lYear = lFolderFields.at(1).toInt();
			break;
		}
		break;
	case 3:
	case 4:
	case 5:
	case 6:
		switch(lFolderNumberBitField & 15)
		{
		case 0:
			if(lArtist.isEmpty())
				lArtist = lFolderFields.at(0);
			if(lAlbum.isEmpty())
				lAlbum = lFolderFields.at(1) + " - " + lFolderFields.at(2);
			break;
		case 1:
			if(lYear == -1)
				lYear = lFolderFields.at(0).toInt();
			if(lArtist.isEmpty())
				lArtist = lFolderFields.at(1);
			if(lAlbum.isEmpty())
				lAlbum = lFolderFields.at(2);
			break;
		case 2:
		case 3:
			if(lArtist.isEmpty())
				lArtist = lFolderFields.at(0);
			if(lYear == -1)
				lYear = lFolderFields.at(1).toInt();
			if(lAlbum.isEmpty())
				lAlbum = lFolderFields.at(2);
			break;
		case 4:
		case 5:
		case 6:
		case 7:
			if(lArtist.isEmpty())
				lArtist = lFolderFields.at(0);
			if(lAlbum.isEmpty())
				lAlbum = lFolderFields.at(1);
			if(lYear == -1)
				lYear = lFolderFields.at(2).toInt();
			break;
		case 8:
			if(lArtist.isEmpty())
				lArtist = lFolderFields.at(0);
			if(lAlbum.isEmpty())
				lAlbum = lFolderFields.at(1) + " - " + lFolderFields.at(2);
			if(lYear == -1)
				lYear = lFolderFields.at(3).toInt();
			break;
		}
		break;
	}

	int lCDNumber = 0;
	if(lYear != -1)
		mUI->mYearSpinner->setValue(lYear);
	if(lTrack != -1)
	{
		lCDNumber = lTrack / 100;
		lTrack = lTrack - lCDNumber*100;
		mUI->mTrackSpinner->setValue(lTrack);

		if(lCDNumber > 0 && !lAlbum.isEmpty())
			lAlbum.append(QString::fromAscii(" CD %1").arg(lCDNumber));
	}
	if(!lArtist.isNull())
		mUI->mArtistEdit->setText(lArtist);
	if(!lAlbum.isNull())
		mUI->mAlbumEdit->setText(lAlbum);
	if(!lTitle.isNull())
		mUI->mTitleEdit->setText(lTitle);

	//TODO: if some fields are not filled out now, check folder name too.

}

