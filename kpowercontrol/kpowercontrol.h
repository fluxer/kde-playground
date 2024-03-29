/*  This file is part of the KDE project
    Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>

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

#ifndef KPOWERCONTROL_H
#define KPOWERCONTROL_H

#include <kstatusnotifieritem.h>
#include <kmenu.h>
#include <khelpmenu.h>
#include <kpowermanager.h>

class KPowerControl : public KStatusNotifierItem
{
    Q_OBJECT
public:
    KPowerControl(QObject* parent = nullptr);
    ~KPowerControl();

private Q_SLOTS:
    void slotChangeProfile();
    void slotChangeBattery();
    void slotProfileChanged(const QString &profile);

    void slotChargePercentChanged(const int newstate, const QString &solidudi);
    void slotChargeStateChanged(const int newstate, const QString &solidudi);
    void slotPowerSupplyStateChanged(const bool newstate, const QString &solidudi);
    void slotPlugStateChanged(const bool newstate, const QString &solidudi);

    void slotDeviceAdded(const QString &solidudi);
    void slotDeviceRemoved(const QString &solidudi);

private:
    void setBattery(const QString &solidudi);
    bool isSelectedBattery(const QString &solidudi) const;

    KMenu* m_menu;
    KHelpMenu* m_helpmenu;
    KPowerManager m_powermanager;
    bool m_notifybatterylow;
};

#endif // KPOWERCONTROL_H
