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
#include "threadjob.h"

#include <QApplication>
#include <QMetaType>
#include <QTimer>

void ThreadJobExecuter::run()
{
	QTimer::singleShot(0, mJob, SLOT(doWork()));
	connect(mJob, SIGNAL(finished(KJob*)), SLOT(quit()));
	exec();
}

ThreadJob::ThreadJob()
	:mShouldDie(false), mShouldSuspend(false), mHasStarted(false)
{
	static bool onlyOnce = false;
	if(!onlyOnce)
	{
		qRegisterMetaType<QPair<QString,QString> >("QPair<QString,QString>");
		onlyOnce = true;
	}
}

void ThreadJob::start()
{
	mThread = new ThreadJobExecuter(this);
	moveToThread(mThread);
	mThread->start();
	mHasStarted = true;
}

void ThreadJob::emitResult()
{
	moveToThread(QApplication::instance()->thread());
	KJob::emitResult();
}

bool ThreadJob::doKill()
{
	if(!(capabilities() & Killable) || !mHasStarted)
		return false;

	mShouldDie = true;
	return true;
}

bool ThreadJob::checkForDeath()
{
	if(!mShouldDie)
		QCoreApplication::processEvents();

	if(mShouldDie)
	{
		aboutToDie();
		mThread->quit();
		moveToThread(QApplication::instance()->thread());
	}
	return mShouldDie;
}

bool ThreadJob::doSuspend()
{
	if(!(capabilities() & Suspendable) || isSuspended())
		return false;

	mShouldSuspend = true;
	return true;
}

bool ThreadJob::checkForDeathOrSuspend()
{
	if(!mShouldSuspend && !mShouldDie)
		QCoreApplication::processEvents();

	if(mShouldDie)
	{
		aboutToDie();
		mThread->quit();
		moveToThread(QApplication::instance()->thread());
		return true;
	}

	if(mShouldSuspend)
	{
		aboutToSuspend();
		while(mShouldSuspend && !mShouldDie)
			QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);

		if(mShouldDie)
		{
			aboutToDie();
			mThread->quit();
			moveToThread(QApplication::instance()->thread());
			return true;
		}
		aboutToResume();
		return false;
	}
	return false;
}

bool ThreadJob::doResume()
{
	if(!(capabilities() & Suspendable) || !isSuspended())
		return false;
	mShouldSuspend = false;
	return true;
}
