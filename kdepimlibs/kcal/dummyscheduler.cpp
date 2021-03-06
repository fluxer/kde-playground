/*
  This file is part of the kcal library.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

//
// DummyScheduler - iMIP implementation of iTIP methods
//

#include "dummyscheduler.h"
#include "event.h"
#include "icalformat.h"

#include <kdebug.h>
#include <kstandarddirs.h>

#include <QtCore/QFile>
#include <QtCore/QTextStream>

using namespace KCal;

DummyScheduler::DummyScheduler( Calendar *calendar )
  : Scheduler( calendar )
{
}

DummyScheduler::~DummyScheduler()
{
}

bool DummyScheduler::publish( IncidenceBase *incidence, const QString &recipients )
{
  Q_UNUSED( recipients );
  QString messageText = mFormat->createScheduleMessage( incidence, iTIPPublish );

  return saveMessage( messageText );
}

bool DummyScheduler::performTransaction( IncidenceBase *incidence,
                                         iTIPMethod method,
                                         const QString &recipients )
{
  Q_UNUSED( recipients );
  QString messageText = mFormat->createScheduleMessage( incidence, method );

  return saveMessage( messageText );
}

bool DummyScheduler::performTransaction( IncidenceBase *incidence, iTIPMethod method )
{
  QString messageText = mFormat->createScheduleMessage( incidence, method );

  return saveMessage( messageText );
}

bool DummyScheduler::saveMessage( const QString &message )
{
  QFile f( "dummyscheduler.store" );
  if ( f.open( QIODevice::WriteOnly | QIODevice::Append ) ) {
    QTextStream t( &f );
    t << message << endl;
    f.close();
    return true;
  } else {
    return false;
  }
}

QList<ScheduleMessage*> DummyScheduler::retrieveTransactions()
{
  QList<ScheduleMessage*> messageList;

  QFile f( "dummyscheduler.store" );
  if ( !f.open( QIODevice::ReadOnly ) ) {
    kDebug() << "Can't open file";
  } else {
    QTextStream t( &f );
    QString messageString;
    QString messageLine = t.readLine();
    while ( !messageLine.isNull() ) {
      messageString += messageLine + '\n';
      if ( messageLine.indexOf( "END:VCALENDAR" ) >= 0 ) {
        kDebug() << "---------------" << messageString;
        ScheduleMessage *message = mFormat->parseScheduleMessage( mCalendar,
                                                                  messageString );
        kDebug() << "--Parsed";
        if ( message ) {
          messageList.append( message );
        } else {
          QString errorMessage;
          if ( mFormat->exception() ) {
            errorMessage = mFormat->exception()->message();
          }
          kDebug() << "Error parsing message:" << errorMessage;
        }
        messageString = "";
      }
      messageLine = t.readLine();
    }
    f.close();
  }

  return messageList;
}

QString DummyScheduler::freeBusyDir()
{
  // the dummy scheduler should never handle freebusy stuff - so it's hardcoded
  return QString( "" );
}
