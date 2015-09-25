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
#include "bbfilechangejob.h"
#include "bbmetadata.h"
#include "bbsettings.h"

#include <KLocale>
#include <QFileInfo>
#include <sqlite3.h>

BBFileChangeJob::BBFileChangeJob(QList<int> pFiles, const QString &pField, const QString &pNewValue)
	: mFiles(pFiles), mField(pField), mNewValue(pNewValue)
{
	setCapabilities(Killable | Suspendable);
}


void BBFileChangeJob::doWork()
{
	mDatabase.connect(BBSettings::fileNameDB());

	setTotalAmount(Files, mFiles.count());
	int lFileCount = 0;

	foreach(int lFileID, mFiles)
	{
		if(checkForDeathOrSuspend())
			return;
		BBResultTable lResult;
		mDatabase.getTable(QString("SELECT path || '/' || file_name FROM albums JOIN songs USING (album_ID) "
		                           "WHERE file_ID = '%1'").arg(lFileID), lResult);
		QString lPath = QString::fromUtf8(lResult.at(1, 0));
		mDatabase.freeTable(lResult);

		emit description(this, i18n("Writing change to files"), qMakePair(i18n("Path"), lPath));

		QFileInfo lInfo(lPath);
		if(!lPath.contains("://"))
			BBMetaData::ChangeFile(lPath, mField, mNewValue);

		if(mField == "album")
		{
			int lAlbumID;
			mDatabase.getTable(QString(
				"SELECT DISTINCT album_ID FROM songs JOIN albums USING (album_ID) "
				"WHERE album = '%1' AND path = '%2'").arg(BBDatabase::prepareString(mNewValue),
				                                          BBDatabase::prepareString(lInfo.absolutePath())),
				lResult);
			if(lResult.mNumRows == 0)
			{
				mDatabase.executeStatement(QString(
					"INSERT INTO albums (path, album, is_VA, cover_art_path) "
					"VALUES ('%1','%2','%3','%4')").arg(BBDatabase::prepareString(lInfo.absolutePath()),
					                                    BBDatabase::prepareString(mNewValue),
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
		else
		{
			mDatabase.executeStatement(
				QString("UPDATE songs SET %1 = '%2' WHERE file_ID = '%3'").arg(mField, BBDatabase::prepareString(mNewValue)).arg(lFileID)
				);
		}
		lFileCount++;
		setProcessedAmount(Files, lFileCount);
		setPercent((ulong)(100*processedAmount(Files))/totalAmount(Files));
	}

	mDatabase.executeStatement("DELETE FROM dead_album_view");
	mDatabase.executeStatement("UPDATE albums SET is_VA = 1 WHERE album_ID IN "
	                           "(SELECT album_ID FROM songs JOIN albums USING (album_ID) "
	                           "GROUP BY album_ID HAVING count(DISTINCT artist) > 1)");
	mDatabase.executeStatement("UPDATE albums SET is_VA = 0 WHERE album_ID IN "
	                           "(SELECT album_ID FROM songs JOIN albums USING (album_ID) "
	                           "GROUP BY album_ID HAVING count(DISTINCT artist) = 1)");

	emitResult();
}
