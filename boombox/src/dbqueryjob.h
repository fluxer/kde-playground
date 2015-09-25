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
#ifndef DBQUERYJOB_H
#define DBQUERYJOB_H

#include <threadweaver/ThreadWeaver.h>
#include <threadweaver/QueuePolicy.h>
#include <threadweaver/Job.h>
#include <QStringList>
#include <QStack>
#include <QSet>

#include "bblistitem.h"

class BBDatabase;

class DBPool: public ThreadWeaver::QueuePolicy
{
	public:
		DBPool(int pNumConnections);
		virtual ~DBPool();

		virtual bool canRun(ThreadWeaver::Job *pJob);
		virtual void free(ThreadWeaver::Job *pJob);
		virtual void release(ThreadWeaver::Job *pJob)
		{
			free(pJob);
		}
		virtual void destructed(ThreadWeaver::Job *pJob)
		{
			free(pJob);
		}

	protected:
		int mNumConnections;
		QStack<BBDatabase *> mAvailable;
};

class DBQueryJob: public ThreadWeaver::Job
{
	Q_OBJECT
	public:
		DBQueryJob()
			: mDatabase(NULL)
		{}

	protected:
		BBDatabase *mDatabase;

	friend class DBPool;
};

class FilterQueryJob: public DBQueryJob
{
	Q_OBJECT
	public:
		FilterQueryJob(const QString &pCategory, const QString &pCondition)
			: mCategory(pCategory), mCondition(pCondition), mShouldAbort(false)
		{}

		virtual void requestAbort() {mShouldAbort = true;}

	signals:
		void resultReady(QSet<BBStringListItem>);

	protected:
		virtual void run();

		QString mCategory, mCondition;
		bool mShouldAbort;
};

class SongQueryJob: public DBQueryJob
{
	Q_OBJECT
	public:
		SongQueryJob(const QString &pCondition)
		: mCondition(pCondition), mShouldAbort(false)
		{}

		virtual void requestAbort() {mShouldAbort = true;}

	signals:
		void resultReady(QSet<BBSongListItem>, QSet<BBAlbumListItem>);

	protected:
		virtual void run();
		void getSongs(bool pVA);

		QString mCategory, mCondition;
		bool mShouldAbort;
		QSet<BBSongListItem> mSongSet;
		QSet<BBAlbumListItem> mAlbumSet;

};

#endif
