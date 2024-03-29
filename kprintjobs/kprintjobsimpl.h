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

#ifndef KPRINTJOBSIMPL_H
#define KPRINTJOBSIMPL_H

#include <QTimer>
#include <KJob>

class KPrintJobsImpl: public KJob
{
    Q_OBJECT
public:
    KPrintJobsImpl(QObject *parent, const int cupsjobid, const int cupsjobstate, const char* const cupsjobdest);
    ~KPrintJobsImpl();

    void start() final;

protected:
    bool doKill() final;

private Q_SLOTS:
    void slotCheckState();

private:
    int m_cupsjobid;
    int m_cupsjobstate;
    QByteArray m_cupsjobdestination;
    bool m_emitdescription;
    bool m_emittotalamount;
    QTimer m_statetimer;
};

#endif // KPRINTJOBSIMPL_H
