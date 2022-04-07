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

#include "kupdatenotifier.h"

#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <klocale.h>
#include <kglobal.h>
#include <kicon.h>
#include <kdebug.h>

// for reference:
// https://www.freedesktop.org/software/PackageKit/gtk-doc/PackageKit.html

KUpdateNotifier::KUpdateNotifier(QObject* parent)
    : KStatusNotifierItem(parent),
    m_gotitaction(nullptr),
    m_menu(nullptr),
    m_helpmenu(nullptr),
    m_interface(PACKAGEKIT_SERVICE, PACKAGEKIT_PATH, PACKAGEKIT_IFACE, QDBusConnection::systemBus(), this)
{
    setTitle(i18n("Update notifier"));
    setCategory(KStatusNotifierItem::SystemServices);
    setIconByName("system-software-update");
    setStatus(KStatusNotifierItem::Passive);

    m_menu = new KMenu(associatedWidget());
    setContextMenu(m_menu);

    m_gotitaction = actionCollection()->addAction(QLatin1String("gotit"));
    m_gotitaction->setText(i18n("&Got it"));
    m_gotitaction->setIcon(KIcon(QLatin1String("dialog-ok")));
    m_gotitaction->setVisible(false);
    connect(m_gotitaction, SIGNAL(triggered()), this, SLOT(slotGotIt()));
    m_menu->addAction(m_gotitaction);

    m_menu->addSeparator();

    m_helpmenu = new KHelpMenu(associatedWidget(), KGlobal::mainComponent().aboutData());
    m_menu->addMenu(m_helpmenu->menu());

    if (m_interface.isValid()) {
        connect(&m_interface, SIGNAL(UpdatesChanged()), this, SLOT(slotUpdatesChanged()));
        // qDebug() << Q_FUNC_INFO << m_interface.property("NetworkState");
    } else {
        setOverlayIconByName("dialog-error");
        showMessage(i18n("Update notifier"), i18n("PackageKit interface is not valid"), "dialog-error");
    }
}

KUpdateNotifier::~KUpdateNotifier()
{
}

void KUpdateNotifier::slotGotIt()
{
    // qDebug() << Q_FUNC_INFO;
    setStatus(KStatusNotifierItem::Passive);
    m_gotitaction->setVisible(false);
}

void KUpdateNotifier::slotUpdatesChanged()
{
    // qDebug() << Q_FUNC_INFO;
    setStatus(KStatusNotifierItem::NeedsAttention);
    m_gotitaction->setVisible(true);
    setOverlayIconByName("vcs-update-required");
    showMessage(i18n("Update notifier"), i18n("Updates available"), "vcs-update-required");
}
