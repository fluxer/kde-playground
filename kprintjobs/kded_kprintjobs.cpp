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

#include "kded_kprintjobs.h"
#include "kpluginfactory.h"
#include "kdebug.h"

#include <cups/cups.h>

K_PLUGIN_FACTORY(KPrintJobsModuleFactory, registerPlugin<KPrintJobsModule>();)
K_EXPORT_PLUGIN(KPrintJobsModuleFactory("kprintjobs"))

KPrintJobsModule::KPrintJobsModule(QObject *parent, const QList<QVariant> &args)
    : KDEDModule(parent),
    m_statetimer(this),
    m_printjobstracker(nullptr)
{
    Q_UNUSED(args);

    connect(&m_statetimer, SIGNAL(timeout()), this, SLOT(slotCheckState()));
    m_statetimer.start(500);
}

KPrintJobsModule::~KPrintJobsModule()
{
    delete m_printjobstracker;
}

void KPrintJobsModule::slotCheckState()
{
    cups_job_t* cupsjobs = nullptr;
    int cupsjobscount = cupsGetJobs(&cupsjobs, NULL, 0, CUPS_WHICHJOBS_ACTIVE);
    // qDebug() << Q_FUNC_INFO << cupsjobscount;
    for (int i = 0; i < cupsjobscount; i++) {
        // qDebug() << Q_FUNC_INFO << cupsjobs[i].id << cupsjobs[i].dest << cupsjobs[i].title << cupsjobs[i].user << cupsjobs[i].state;
        const int cupsjobid = cupsjobs[i].id;
        const int cupsjobstate = static_cast<int>(cupsjobs[i].state);
        KPrintJobsImpl* kprintjobsimpl = m_printjobs.value(cupsjobid, nullptr);
        if (kprintjobsimpl) {
            continue;
        }
        kprintjobsimpl = new KPrintJobsImpl(this, cupsjobid, cupsjobstate, cupsjobs[i].dest);
        if (!m_printjobstracker) {
            m_printjobstracker = new KDynamicJobTracker(this);
        }
        connect(kprintjobsimpl, SIGNAL(destroyed(QObject*)), this, SLOT(slotJobDestroyed(QObject*)));
        m_printjobs.insert(cupsjobid, kprintjobsimpl);
        m_printjobstracker->registerJob(kprintjobsimpl);
        kprintjobsimpl->start();
    }
    cupsFreeJobs(cupsjobscount, cupsjobs);
}

void KPrintJobsModule::slotJobDestroyed(QObject *kprintjobsimpl)
{
    QMutableMapIterator<int, KPrintJobsImpl*> it(m_printjobs);
    while (it.hasNext()) {
        it.next();
        KPrintJobsImpl* itvalue = it.value();
        if (itvalue == kprintjobsimpl) {
            it.remove();
            break;
        }
    }
}

#include "moc_kded_kprintjobs.cpp"
