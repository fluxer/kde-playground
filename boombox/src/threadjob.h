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
#ifndef THREADJOB_H
#define THREADJOB_H

#include <QThread>
#include <KJob>

class ThreadJob;

class ThreadJobExecuter : public QThread
{
	Q_OBJECT
	public:
		ThreadJobExecuter(ThreadJob *pJob) {mJob = pJob;}
		void run();

	private:
		ThreadJob *mJob;
};

class ThreadJob: public KJob
{
	Q_OBJECT
	public:
		ThreadJob();

		virtual void start();

	public slots:
		virtual void doWork()=0;

	protected:
		virtual void emitResult();
		virtual bool doKill();
		virtual bool doSuspend();
		virtual bool doResume();

		virtual bool checkForDeathOrSuspend();
		virtual bool checkForDeath();

		virtual void aboutToDie(){}
		virtual void aboutToSuspend(){}
		virtual void aboutToResume(){}

		ThreadJobExecuter *mThread;

		bool mShouldDie;
		bool mShouldSuspend;
		bool mHasStarted;
};


#endif
