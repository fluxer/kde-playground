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
#include "bbdatabase.h"
#include "bbmetadata.h"

#include <QDir>
#include <sqlite3.h>

BBDatabase::BBDatabase()
	: mConnected(false), mSqlite(NULL)
{
}

BBDatabase::~BBDatabase()
{
	if(mConnected)
		disconnect();
}

bool BBDatabase::connect(const QString &pFileName)
{
	if(mConnected)
		disconnect();

	int lError = sqlite3_open(pFileName.toUtf8(), &mSqlite);

	if(lError != SQLITE_OK)
	{
		qCritical("error opening: %i", lError);
		return false;
	}
	executeStatement("CREATE TABLE IF NOT EXISTS albums (album_ID INTEGER PRIMARY KEY, path STRING, "
	                 "album STRING, is_VA INTEGER, cover_art_path STRING)");
	executeStatement("CREATE TABLE IF NOT EXISTS songs (file_ID INTEGER PRIMARY KEY, "
	                 "album_ID INTEGER, file_name STRING, "
	                 "artist STRING, title STRING, track INTEGER, year INTEGER, genre STRING, "
	                 "comment STRING, length STRING)");
	executeStatement("CREATE VIEW IF NOT EXISTS dead_album_view AS SELECT * FROM albums "
	                 "LEFT OUTER JOIN songs USING (album_ID) WHERE file_name IS NULL");
	executeStatement("CREATE TRIGGER IF NOT EXISTS delete_album_trigger INSTEAD OF DELETE ON dead_album_view BEGIN "
	                 "DELETE FROM albums WHERE albums.album_ID = old.album_ID; END");
	mConnected = true;
	return true;
}

void BBDatabase::disconnect()
{
	if(mConnected)
	{
		mConnected = false;
		sqlite3_close(mSqlite);
		mSqlite = NULL;
	}
}

bool BBDatabase::executeStatement(const QString &pStatement)
{
	char *lError = NULL;
	int lReturnCode;
	qDebug("Executing statement: %s", (const char *)pStatement.toUtf8());
	lReturnCode = sqlite3_exec(mSqlite, pStatement.toUtf8(), NULL, NULL, &lError);
	if(lError != NULL)
	{
		qWarning("database error: %s", lError);
		sqlite3_free(lError);
	}
	return lReturnCode == SQLITE_OK;
}

bool BBDatabase::getTable(const QString &pQuery, BBResultTable &pTable)
{
	char *lError = NULL;
	int lReturnCode;
	qDebug("Executing statement: %s", (const char *)pQuery.toUtf8());
	lReturnCode = sqlite3_get_table(mSqlite, pQuery.toUtf8(), &pTable.mTable, &pTable.mNumRows, &pTable.mNumCols, &lError);
	if(lError != NULL)
	{
		qWarning("database error: %s", lError);
		sqlite3_free(lError);
	}
	qDebug("Number of rows in result: %i", pTable.mNumRows);
	return lReturnCode == SQLITE_OK;
}

void BBDatabase::freeTable(BBResultTable &pTable)
{
	sqlite3_free_table(pTable.mTable);
	pTable.mTable = NULL;
	pTable.mNumRows = 0;
	pTable.mNumCols = 0;
}

QString BBDatabase::prepareList(const QStringList &pList)
{
	if(pList.isEmpty())
		return QString();
	QStringList lTmp;
	QStringListIterator i(pList);
	while(i.hasNext())
		lTmp << prepareString(i.next());

	return "(\'" + lTmp.join("\', \'") + "\')";
}
