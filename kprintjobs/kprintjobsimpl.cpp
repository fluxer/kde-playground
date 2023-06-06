/*  This file is part of the KDE libraries
    Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kprintjobsimpl.h"
#include "klocale.h"
#include "kdebug.h"

#include <QFile>
#include <cups/cups.h>

KPrintJobsImpl::KPrintJobsImpl(QObject *parent, const int cupsjobid, const int cupsjobstate, const char* const cupsjobdest)
    : KJob(parent),
    m_cupsjobid(cupsjobid),
    m_cupsjobstate(cupsjobstate),
    m_cupsjobdestination(cupsjobdest),
    m_emitdescription(true),
    m_statetimer(this)
{
    setProperty("appName", QString::fromLatin1("kprintjobs"));
    setProperty("appIconName", QString::fromLatin1("document-print"));

    setCapabilities(KJob::Killable);

    connect(&m_statetimer, SIGNAL(timeout()), this, SLOT(slotCheckState()));

    kDebug() << "Tracking print job" << m_cupsjobid;
}

KPrintJobsImpl::~KPrintJobsImpl()
{
    kDebug() << "No longer tracking print job" << m_cupsjobid;
}

void KPrintJobsImpl::start()
{
    m_statetimer.start(500);
}

bool KPrintJobsImpl::doKill()
{
    const int cupsresult = cupsCancelJob(m_cupsjobdestination.constData(), m_cupsjobid);
    return (cupsresult == 1);
}

void KPrintJobsImpl::slotCheckState()
{
    cups_job_t* cupsjobs = nullptr;
    int cupsjobscount = cupsGetJobs(&cupsjobs, NULL, 0, CUPS_WHICHJOBS_ALL);
    // qDebug() << Q_FUNC_INFO << cupsjobscount;
    for (int i = 0; i < cupsjobscount; i++) {
        const int cupsjobid = cupsjobs[i].id;
        const int cupsjobstate = static_cast<int>(cupsjobs[i].state);
        if (cupsjobid != m_cupsjobid) {
            continue;
        }
        // qDebug() << Q_FUNC_INFO << cupsjobs[i].id << cupsjobs[i].dest << cupsjobs[i].title << cupsjobs[i].user << cupsjobs[i].state;
        if (m_emitdescription) {
            m_emitdescription = false;
            emit description(this,
                QString::fromLocal8Bit(cupsjobs[i].title),
                qMakePair(i18nc("The destination of a print operation", "Destination"), QFile::decodeName(cupsjobs[i].dest))
            );
        }
        if (cupsjobstate == static_cast<int>(IPP_JSTATE_HELD) && cupsjobstate != m_cupsjobstate) {
            kDebug() << "Print job suspended" << m_cupsjobid;
            m_cupsjobstate = cupsjobstate;
            suspend();
            continue;
        } else if (cupsjobstate == static_cast<int>(IPP_JSTATE_PROCESSING) && cupsjobstate != m_cupsjobstate) {
            kDebug() << "Print job resumed" << m_cupsjobid;
            m_cupsjobstate = cupsjobstate;
            resume();
            continue;
        } else if (cupsjobstate == static_cast<int>(IPP_JSTATE_CANCELED) || cupsjobstate == static_cast<int>(IPP_JSTATE_ABORTED)) {
            kDebug() << "Print job canceled/aborted" << m_cupsjobid;
            m_statetimer.stop();
            setErrorText(i18n("Aborted"));
            emitResult();
            break;
        } else if (cupsjobstate == static_cast<int>(IPP_JSTATE_COMPLETED)) {
            kDebug() << "Print job completed" << m_cupsjobid;
            m_statetimer.stop();
            emitResult();
            break;
        }
    }
    cupsFreeJobs(cupsjobscount, cupsjobs);
}

#include "moc_kprintjobsimpl.cpp"
