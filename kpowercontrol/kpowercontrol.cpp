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

#include "kpowercontrol.h"

#include <solid/device.h>
#include <solid/battery.h>
#include <solid/devicenotifier.h>
#include <knotification.h>
#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <klocale.h>
#include <kglobal.h>
#include <kaction.h>
#include <kicon.h>
#include <kdebug.h>

static inline QString normalizeUDI(const QString &solidudi)
{
    const QByteArray udihex = solidudi.toLatin1().toHex();
    return QString::fromLatin1(udihex.constData(), udihex.size());
}

static inline QString deviceProduct(const Solid::Device &soliddevice)
{
    QString deviceproduct = soliddevice.product();
    if (deviceproduct.isEmpty()) {
        deviceproduct = soliddevice.udi();
        const int lastslashindex = deviceproduct.lastIndexOf(QLatin1Char('/'));
        deviceproduct = deviceproduct.mid(lastslashindex + 1);
    }
    return deviceproduct;
}

static inline QString batteryState(const Solid::Battery::ChargeState solidstate)
{
    switch (solidstate) {
        case Solid::Battery::UnknownCharge: {
            return i18n("Unknown");
        }
        case Solid::Battery::Charging: {
            return i18n("Charging");
        }
        case Solid::Battery::Discharging: {
            return i18n("Discharging");
        }
        case Solid::Battery::FullyCharged: {
            return i18n("Fully charged");
        }
    }
    Q_ASSERT(false);
    return i18n("Unknown");
}

KPowerControl::KPowerControl(QObject* parent)
    : KStatusNotifierItem(parent),
    m_menu(nullptr),
    m_helpmenu(nullptr),
    m_notifybatterylow(true)
{
    setTitle(i18n("Power management"));
    setCategory(KStatusNotifierItem::Hardware);
    setIconByName("preferences-system-power-management");
    setStatus(KStatusNotifierItem::Passive);

    m_menu = new KMenu(associatedWidget());
    setContextMenu(m_menu);

    if (!KPowerManager::isSupported()) {
        setOverlayIconByName("dialog-error");
        showMessage(
            i18n("Power management"),
            i18n("Power management is not supported on this system"),
            "dialog-error"
        );
    } else {
        connect(
            &m_powermanager, SIGNAL(profileChanged(QString)),
            this, SLOT(slotProfileChanged(QString))
        );

        foreach (const QString &profile, m_powermanager.profiles()) {
            KAction* profileaction = actionCollection()->addAction(
                QString::fromLatin1("profile_%1").arg(profile)
            );
            profileaction->setText(profile);
            profileaction->setCheckable(true);
            profileaction->setChecked(profile == m_powermanager.profile());
            connect(profileaction, SIGNAL(triggered()), this, SLOT(slotChangeProfile()));
            m_menu->addAction(profileaction);
        }

        m_menu->addSeparator();
    }

    const QList<Solid::Device> solidbatteries = Solid::Device::listFromType(
        Solid::DeviceInterface::Battery
    );
    // qDebug() << Q_FUNC_INFO << solidbatteries.size();
    if (!solidbatteries.isEmpty()) {
        QString batteryudi;
        foreach (const Solid::Device &soliddevice, solidbatteries) {
            const Solid::Battery* solidbattery = soliddevice.as<Solid::Battery>();
            // qDebug() << Q_FUNC_INFO << soliddevice.udi() << solidbattery->chargePercent();
            connect(
                solidbattery, SIGNAL(chargePercentChanged(int,QString)),
                this, SLOT(slotChargePercentChanged(int,QString))
            );
            connect(
                solidbattery, SIGNAL(chargeStateChanged(int,QString)),
                this, SLOT(slotChargeStateChanged(int,QString))
            );
            connect(
                solidbattery, SIGNAL(powerSupplyStateChanged(bool,QString)),
                this, SLOT(slotPowerSupplyStateChanged(bool,QString))
            );
            connect(
                solidbattery, SIGNAL(plugStateChanged(bool,QString)),
                this, SLOT(slotPlugStateChanged(bool,QString))
            );
            
            KAction* batteryaction = actionCollection()->addAction(
                QString::fromLatin1("battery_%1").arg(normalizeUDI(soliddevice.udi()))
            );
            batteryaction->setText(deviceProduct(soliddevice));
            batteryaction->setCheckable(true);
            batteryaction->setChecked(false);
            batteryaction->setData(soliddevice.udi());
            connect(batteryaction, SIGNAL(triggered()), this, SLOT(slotChangeBattery()));
            m_menu->addAction(batteryaction);

            if (batteryudi.isEmpty() || solidbattery->isPowerSupply()) {
                batteryudi = soliddevice.udi();
            }
        }
        setBattery(batteryudi);
    }
    m_menu->addSeparator();

    connect(
        Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(QString)),
        this, SLOT(slotDeviceAdded(QString))
    );
    connect(
        Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(QString)),
        this, SLOT(slotDeviceRemoved(QString))
    );

    m_helpmenu = new KHelpMenu(associatedWidget(), KGlobal::mainComponent().aboutData());
    m_menu->addMenu(m_helpmenu->menu());
}

KPowerControl::~KPowerControl()
{
}

void KPowerControl::slotChangeProfile()
{
    KAction* profileaction = qobject_cast<KAction*>(sender());
    if (m_powermanager.setProfile(profileaction->iconText())) {
        setOverlayIconByName(QString());
        // do not wait for the signal to be emited
        slotProfileChanged(profileaction->iconText());
    } else {
        setOverlayIconByName("dialog-error");
        showMessage(
            i18n("Power management"),
            i18n("Could not change power manager profile"),
            "dialog-error"
        );
    }
}

void KPowerControl::slotChangeBattery()
{
    KAction* batteryaction = qobject_cast<KAction*>(sender());
    m_notifybatterylow = true;
    setBattery(batteryaction->data().toString());
}

void KPowerControl::slotProfileChanged(const QString &profile)
{
    const QString profileobjectname = QString::fromLatin1("profile_%1").arg(profile);
    foreach (QAction* qaction, actionCollection()->actions()) {
        const QString qactionobjectname = qaction->objectName();
        if (qactionobjectname.startsWith(QLatin1String("profile_"))) {
            qaction->setChecked(qactionobjectname == profileobjectname);
        }
    }
}

void KPowerControl::setBattery(const QString &solidudi)
{
    // qDebug() << Q_FUNC_INFO << solidudi;
    foreach (QAction* qaction, actionCollection()->actions()) {
        const QString qactionobjectname = qaction->objectName();
        const QString qactiondata = qaction->data().toString();
        if (qactionobjectname.startsWith(QLatin1String("battery_"))) {
            qaction->setChecked(qactiondata == solidudi);
        }
    }

    Solid::Device soliddevice(solidudi);
    const Solid::Battery* solidbattery = soliddevice.as<Solid::Battery>();
    if (!soliddevice.isValid() || !solidbattery) {
        setIconByName("preferences-system-power-management");
        setStatus(KStatusNotifierItem::Passive);
        setToolTip(KIcon("preferences-system-power-management"), QString(), QString());
        if (!solidbattery) {
            kWarning() << "Null battery device pointer";
        }
        kWarning() << "Invalid battery device";
        return;
    }

    if (solidbattery->isPowerSupply()) {
        setIconByName(soliddevice.icon());
        setStatus(KStatusNotifierItem::Active);
    } else {
        setIconByName("preferences-system-power-management");
        setStatus(KStatusNotifierItem::Passive);
    }
    // qDebug() << Q_FUNC_INFO << soliddevice.icon();

    QString batterytooltip = i18n(
        "Product: %1<br>Is power supply: %2<br>Is plugged: %3<br>Charge state: %4<br>Charge percent:%5",
        deviceProduct(soliddevice),
        solidbattery->isPowerSupply() ? i18n("Yes") : i18n("No"),
        solidbattery->isPlugged() ? i18n("Yes") : i18n("No"),
        batteryState(solidbattery->chargeState()),
        solidbattery->chargePercent()
    );
    setToolTip(KIcon(soliddevice.icon()), i18n("Battery details"), batterytooltip);

    if (m_notifybatterylow && solidbattery->isPowerSupply() && solidbattery->chargePercent() <= 20) {
        KNotification *knotification = new KNotification("BatteryLow");
        knotification->setComponentData(KComponentData("kpowercontrol"));
        knotification->setTitle(i18n("Battery status"));
        knotification->setText(i18n("Battery charge is low"));
        knotification->sendEvent();
        m_notifybatterylow = false;
    }
}

bool KPowerControl::isSelectedBattery(const QString &solidudi) const
{
    foreach (QAction* qaction, actionCollection()->actions()) {
        const QString qactionobjectname = qaction->objectName();
        if (qactionobjectname.startsWith(QLatin1String("battery_"))) {
            const QString qactiondata = qaction->data().toString();
            if (qactiondata == solidudi && qaction->isChecked()) {
                return true;
            }
        }
    }
    return false;
}

void KPowerControl::slotChargePercentChanged(const int newstate, const QString &solidudi)
{
    // qDebug() << Q_FUNC_INFO << newstate << solidudi << isSelectedBattery(solidudi);
    if (isSelectedBattery(solidudi)) {
        setBattery(solidudi);
    }
}

void KPowerControl::slotChargeStateChanged(const int newstate, const QString &solidudi)
{
    // qDebug() << Q_FUNC_INFO << newstate << solidudi << isSelectedBattery(solidudi);
    if (isSelectedBattery(solidudi)) {
        setBattery(solidudi);
    }
}

void KPowerControl::slotPowerSupplyStateChanged(const bool newstate, const QString &solidudi)
{
    // qDebug() << Q_FUNC_INFO << newstate << solidudi << isSelectedBattery(solidudi);
    if (isSelectedBattery(solidudi)) {
        m_notifybatterylow = true;
        setBattery(solidudi);

        KNotification *knotification = nullptr;
        if (newstate) {
            knotification = new KNotification("Unconnected");
            knotification->setComponentData(KComponentData("kpowercontrol"));
            knotification->setTitle(i18n("Power profile status"));
            knotification->setText(
                i18n("Disonnected from external power source, switching profile to PowerSave")
            );
        } else {
            knotification = new KNotification("Connected");
            knotification->setComponentData(KComponentData("kpowercontrol"));
            knotification->setTitle(i18n("Power profile status"));
            knotification->setText(
                i18n("Connected to external power source, switching profile to Performance")
            );
        }
        knotification->sendEvent();
    }
}

void KPowerControl::slotPlugStateChanged(const bool newstate, const QString &solidudi)
{
    qDebug() << Q_FUNC_INFO << newstate << solidudi << isSelectedBattery(solidudi);
    if (newstate) {
        // TODO:
    } else {
        foreach (QAction* qaction, actionCollection()->actions()) {
            const QString qactionobjectname = qaction->objectName();
            if (qactionobjectname.startsWith(QLatin1String("battery_"))) {
                const QString qactiondata = qaction->data().toString();
                if (qactiondata == solidudi) {
                    actionCollection()->removeAction(qaction);
                    break;
                }
            }
        }
    }
}

void KPowerControl::slotDeviceAdded(const QString &solidudi)
{
    // qDebug() << Q_FUNC_INFO << solidudi;
    Solid::Device soliddevice(solidudi);
    if (soliddevice.is<Solid::Battery>()) {
        slotPlugStateChanged(true, solidudi);
    }
}

void KPowerControl::slotDeviceRemoved(const QString &solidudi)
{
    // qDebug() << Q_FUNC_INFO << solidudi;
    foreach (QAction* qaction, actionCollection()->actions()) {
        const QString qactionobjectname = qaction->objectName();
        if (qactionobjectname.startsWith(QLatin1String("battery_"))) {
            const QString qactiondata = qaction->data().toString();
            if (qactiondata == solidudi) {
                slotPlugStateChanged(false, solidudi);
                break;
            }
        }
    }
}
