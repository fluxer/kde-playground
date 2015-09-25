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
#ifndef BBDATABASE_H
#define BBDATABASE_H

#include <QObject>
#include <QList>
#include <QSet>

class BBMetaData;
class BBStringListItem;
class BBDataListItem;
struct sqlite3;

class BBResultTable
{
public:
	int mNumRows;
	int mNumCols;
	char **mTable;
	char *at(int pRow, int pColumn)
	{
		return mTable[pRow*mNumCols + pColumn];
	}
};

class BBDatabase
{
public:
	BBDatabase();
	~BBDatabase();

	bool connect(const QString &pFileName);
	void disconnect();
	bool isConnected(){return mConnected;}
	sqlite3 *getDatabase(){return mSqlite;}
	bool executeStatement(const QString &pStatement);
	bool getTable(const QString &pQuery, BBResultTable &pTable);
	static void freeTable(BBResultTable &pTable);
	void insertLine(BBMetaData &pMetaData);

	static QString prepareString(const QString &pStr)
	{
		return QString(pStr).replace('\'', "''");
	}

	static QString prepareList(const QStringList &pList);

protected:
	bool mConnected;
	sqlite3 *mSqlite;
};

#endif
