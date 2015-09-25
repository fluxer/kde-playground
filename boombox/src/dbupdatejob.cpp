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
#include "dbupdatejob.h"
#include "bbdatabase.h"
#include "bbmetadata.h"
#include "bbsettings.h"

#include <QMap>
#include <QFileInfo>
#include <QDir>
#include <KLocale>
#include <sqlite3.h>

DBUpdateJob::DBUpdateJob()
{
	mFileTypes << "*.mp3" << "*.MP3" <<"*.ogg" <<"*.OGG" <<"*.flac" <<"*.FLAC" <<"*.mpc" << "*.MPC";
	setCapabilities(Killable | Suspendable);
}

void DBUpdateJob::aboutToDie()
{
	mDatabase.executeStatement("ROLLBACK");
}

void DBUpdateJob::doWork()
{
	QMap<QString, int> lDatabaseSet, lFileSystemSet;

	emit infoMessage(this, i18n("Reading list of files from database"));

	mDatabase.connect(BBSettings::fileNameDB());

	BBResultTable lResult;
	mDatabase.getTable("SELECT path || '/' || file_name, file_ID FROM songs JOIN albums USING (album_ID)", lResult);

	for(int i = 1; i <= lResult.mNumRows; ++i)
	{
		if(checkForDeathOrSuspend())
			return;

		lDatabaseSet.insert(QString::fromUtf8(lResult.at(i,0)), QString::fromUtf8(lResult.at(i,1)).toInt());
	}

	mDatabase.freeTable(lResult);

	emit infoMessage(this, i18n("Scanning directories for music files"));

	QStringList lPathList = BBSettings::musicDirectories();
	QString lPath;
	foreach(lPath, lPathList)
	{
		if(!addPathToSet(lPath, lFileSystemSet))
			return;
	}

	mDatabase.executeStatement("BEGIN");
	QMapIterator<QString, int> i(lDatabaseSet);
	while(i.hasNext())
	{
		if(checkForDeathOrSuspend())
			return;
		i.next();
		if(!lFileSystemSet.contains(i.key()) && !i.key().contains("://"))
		{
			emit description(this, i18n("Removing old entries"), qMakePair(i18n("Path"), i.key()));

			mDatabase.executeStatement(QString("DELETE FROM songs WHERE file_ID = %1").arg(i.value()));
		}
	}

	mDatabase.executeStatement("COMMIT");
	mDatabase.executeStatement("BEGIN");
	setTotalAmount(Files, lFileSystemSet.count());

	BBMetaData lMetaData;
	int lInsertCount = 0, lFileCount=0;
	i = lFileSystemSet;
	while(i.hasNext())
	{
		if(checkForDeathOrSuspend())
			return;
		i.next();
		QFileInfo lFileInfo(i.key());
		bool lSongInDatabase = lDatabaseSet.contains(i.key());

		if(lFileInfo.isFile() &&
		   (!lSongInDatabase || lFileInfo.lastModified() > BBSettings::databaseUpdateTime()))
		{
			if(lSongInDatabase)
				mDatabase.executeStatement(QString("DELETE FROM songs WHERE file_ID = %1").arg(lDatabaseSet.value(i.key())));
			lMetaData.GetInfoFrom(lFileInfo.absolutePath(), lFileInfo.fileName());
			emit description(this, i18n("Adding new entries"),
			                 qMakePair(i18n("Path"), lFileInfo.absoluteFilePath()),
			                 qMakePair(i18n("Info"), QString("%1 - %2").arg(lMetaData.mArtist, lMetaData.mTitle)));
			addSongToDatabase(lMetaData);
			lInsertCount++;
		}

		if(lInsertCount > 99) //commit to db every hundred inserts.
		{
			mDatabase.executeStatement("COMMIT");
			mDatabase.executeStatement("BEGIN");
			lInsertCount = 0;
		}
		lFileCount++;
		setProcessedAmount(Files, lFileCount);
		setPercent((ulong)(100*processedAmount(Files))/totalAmount(Files)); //BUG in KJob??
	}

	mDatabase.executeStatement("COMMIT");
	mDatabase.executeStatement("DELETE FROM dead_album_view");

	mDatabase.executeStatement("UPDATE albums SET is_VA = 1 WHERE album_ID IN "
	                           "(SELECT album_ID FROM songs JOIN albums USING (album_ID) "
	                           "GROUP BY album_ID HAVING count(DISTINCT artist) > 1)");
	mDatabase.executeStatement("UPDATE albums SET is_VA = 0 WHERE album_ID IN "
	                           "(SELECT album_ID FROM songs JOIN albums USING (album_ID) "
	                           "GROUP BY album_ID HAVING count(DISTINCT artist) = 1)");
	emitResult();
}

void DBUpdateJob::addSongToDatabase(BBMetaData &pMetaData)
{
	int lAlbumID;
	BBResultTable lResult;
	mDatabase.getTable(QString("SELECT album_ID FROM albums WHERE path = '%1' AND album = '%2'"
	                           ).arg(BBDatabase::prepareString(pMetaData.mPath),
	                                 BBDatabase::prepareString(pMetaData.mAlbum)), lResult);
	if(lResult.mNumRows == 0)
	{
		mDatabase.executeStatement(QString("INSERT INTO albums (path, album, is_VA, cover_art_path) VALUES ('%1','%2','%3','%4')"
		                                   ).arg(BBDatabase::prepareString(pMetaData.mPath),
		                                         BBDatabase::prepareString(pMetaData.mAlbum),
		                                         QString::number(0), QString()));
		lAlbumID = sqlite3_last_insert_rowid(mDatabase.getDatabase());
	}
	else
	{
		lAlbumID = QString::fromUtf8(lResult.at(1, 0)).toInt();
	}
	mDatabase.freeTable(lResult);

	mDatabase.executeStatement(QString("INSERT INTO songs (album_ID, file_name, artist, title, track, year, genre, "
	                                   "comment, length) VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8', '%9')"
	                                   ).arg(lAlbumID).arg(BBDatabase::prepareString(pMetaData.mFileName),
	                                                       BBDatabase::prepareString(pMetaData.mArtist),
	                                                       BBDatabase::prepareString(pMetaData.mTitle),
	                                                       BBDatabase::prepareString(pMetaData.mTrack),
	                                                       BBDatabase::prepareString(pMetaData.mYear),
	                                                       BBDatabase::prepareString(pMetaData.mGenre),
	                                                       BBDatabase::prepareString(pMetaData.mComment),
	                                                       BBDatabase::prepareString(pMetaData.mLength)));

	pMetaData.mFileID = sqlite3_last_insert_rowid(mDatabase.getDatabase());
}

bool DBUpdateJob::addPathToSet(const QString &pPath, QMap<QString, int> &pSet)
{
	QDir lDir(pPath);
	if(!lDir.isReadable())
		return true;

	if(checkForDeathOrSuspend())
		return false;

	QFileInfoList lInfoList = lDir.entryInfoList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot);
	QFileInfo lFileInfo;
	foreach(lFileInfo, lInfoList)
	{
		if(!addPathToSet(lFileInfo.absoluteFilePath(), pSet))
			return false;
	}
	lInfoList = lDir.entryInfoList(mFileTypes, QDir::Files | QDir::Readable);
	foreach(lFileInfo, lInfoList)
	{
		if(checkForDeathOrSuspend())
			return false;

		pSet.insert(lFileInfo.absoluteFilePath(), 0);
	}
	return true;
}

#include "dbupdatejob.moc"
