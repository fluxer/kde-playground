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

#include <QApplication>
#include <QDBusConnectionInterface>
#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <klocale.h>
#include <kglobal.h>
#include <kaction.h>
#include <kicon.h>
#include <kdebug.h>

KPowerControl::KPowerControl(QObject* parent)
    : KStatusNotifierItem(parent),
    m_menu(nullptr),
    m_helpmenu(nullptr)
{
    setTitle(i18n("Power management"));
    setCategory(KStatusNotifierItem::SystemServices);
    setIconByName("preferences-system-power-management");
    setStatus(KStatusNotifierItem::Passive);

    m_menu = new KMenu(associatedWidget());
    setContextMenu(m_menu);

    QDBusConnectionInterface* sessionbusiface = QDBusConnection::sessionBus().interface();
    if (sessionbusiface->isServiceRegistered("org.freedesktop.PowerManagement")) {
        connect(
            &m_powermanager, SIGNAL(profileChanged(QString)),
            this, SLOT(slotProfileChanged(QString))
        );

        foreach (const QString &profile, m_powermanager.profiles()) {
            KAction* profileaction = actionCollection()->addAction(QString::fromLatin1("profile_%1").arg(profile));
            profileaction->setText(profile);
            profileaction->setCheckable(true);
            profileaction->setChecked(profile == m_powermanager.profile());
            connect(profileaction, SIGNAL(triggered()), this, SLOT(slotChangeProfile()));
            m_menu->addAction(profileaction);
        }

        m_menu->addSeparator();
    } else {
        setOverlayIconByName("dialog-error");
        showMessage(i18n("Power management"), i18n("Power manager is not activer"), "dialog-error");
    }

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
        // do not wait for the signal to be emited
        slotProfileChanged(profileaction->iconText());
    } else {
        setOverlayIconByName("dialog-error");
        showMessage(i18n("Power management"), i18n("Could not change power manager profile"), "dialog-error");
    }
}

void KPowerControl::slotProfileChanged(const QString &profile)
{
    const QString profile_objectname = QString::fromLatin1("profile_%1").arg(profile);
    foreach (QAction* qaction, actionCollection()->actions()) {
        const QString qactionobjectname = qaction->objectName();
        if (qactionobjectname.startsWith(QLatin1String("profile_"))) {
            qaction->setChecked(qactionobjectname == profile_objectname);
        }
    }
}
