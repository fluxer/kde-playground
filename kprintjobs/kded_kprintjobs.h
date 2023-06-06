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

#ifndef KPRINTJOBS_KDED_H
#define KPRINTJOBS_KDED_H

#include "kdedmodule.h"
#include "kprintjobsimpl.h"

#include <QTimer>
#include <QMap>
#include <kuiserverjobtracker.h>

class KPrintJobsModule: public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kprintjobs")
public:
    KPrintJobsModule(QObject *parent, const QList<QVariant> &args);
    ~KPrintJobsModule();

private Q_SLOTS:
    void slotCheckState();

private:
    QTimer m_statetimer;
    QMap<int, KPrintJobsImpl*> m_printjobs;
    KUiServerJobTracker *m_printjobstracker;
};

#endif // KPRINTJOBS_KDED_H
