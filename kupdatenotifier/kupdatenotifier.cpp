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

#include <QApplication>
#include <QDBusReply>
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
    m_state(KUpdateNotifier::PassiveState),
    m_gotitaction(nullptr),
    m_menu(nullptr),
    m_helpmenu(nullptr),
    m_interface(PACKAGEKIT_SERVICE, PACKAGEKIT_PATH, PACKAGEKIT_IFACE, QDBusConnection::systemBus(), this),
    m_finished(false)
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

    if (m_interface.isValid()) {
        KAction* refreshcacheaction = actionCollection()->addAction(QLatin1String("refresh_cache"));
        refreshcacheaction->setText(i18n("&Refresh cache"));
        refreshcacheaction->setIcon(KIcon(QLatin1String("view-refresh")));
        connect(refreshcacheaction, SIGNAL(triggered()), this, SLOT(slotRefreshCache()));
        m_menu->addAction(refreshcacheaction);

        m_menu->addSeparator();
    }

    m_helpmenu = new KHelpMenu(associatedWidget(), KGlobal::mainComponent().aboutData());
    m_menu->addMenu(m_helpmenu->menu());

    if (m_interface.isValid()) {
        connect(&m_interface, SIGNAL(UpdatesChanged()), this, SLOT(slotUpdatesChanged()));
        connect(&m_interface, SIGNAL(RestartSchedule()), this, SLOT(slotRestartSchedule()));

        // qDebug() << Q_FUNC_INFO << m_interface.property("NetworkState");

        refreshCache();
    } else {
        setOverlayIconByName("dialog-error");
        showMessage(i18n("Update notifier"), i18n("PackageKit interface is not valid"), "dialog-error");
    }
}

KUpdateNotifier::~KUpdateNotifier()
{
}

void KUpdateNotifier::refreshCache()
{
    QDBusReply<QDBusObjectPath> transactionreply = m_interface.call("CreateTransaction");
    QDBusInterface transactioniface(
        PACKAGEKIT_SERVICE,
        transactionreply.value().path(),
        PACKAGEKIT_TRANSACTION_IFACE,
        QDBusConnection::systemBus(), this
    );

    if (transactioniface.isValid()) {
        m_finished = false;
        connect(&transactioniface, SIGNAL(ErrorCode(uint,QString)), this, SLOT(slotErrorCode(uint,QString)));
        connect(&transactioniface, SIGNAL(Finished(uint,uint)), this, SLOT(slotFinished(uint,uint)));
        transactioniface.call("RefreshCache", true); // force
        while (!m_finished) {
            QApplication::processEvents();
        }
        kDebug() << transactioniface.property("Status");
    } else {
        kWarning() << "transaction interface is not valid";
    }
}

QStringList KUpdateNotifier::getUpdates()
{
    QStringList result;

    QDBusReply<QDBusObjectPath> transactionreply = m_interface.call("CreateTransaction");
    QDBusInterface transactioniface(
        PACKAGEKIT_SERVICE,
        transactionreply.value().path(),
        PACKAGEKIT_TRANSACTION_IFACE,
        QDBusConnection::systemBus(), this
    );

    m_packages.clear();
    if (transactioniface.isValid()) {
        m_finished = false;
        connect(&transactioniface, SIGNAL(Package(uint,QString,QString)), this, SLOT(slotPackage(uint,QString,QString)));
        connect(&transactioniface, SIGNAL(ErrorCode(uint,QString)), this, SLOT(slotErrorCode(uint,QString)));
        connect(&transactioniface, SIGNAL(Finished(uint,uint)), this, SLOT(slotFinished(uint,uint)));
        transactioniface.call("GetUpdates", qulonglong(1)); // PK_FILTER_ENUM_NONE filter
        while (!m_finished) {
            QApplication::processEvents();
        }
        foreach (const KPackageKitPackage &package, m_packages) {
            result.append(package.package_id);
        }
        kDebug() << transactioniface.property("Status");
    } else {
        kWarning() << "transaction interface is not valid";
    }

    return result;
}

void KUpdateNotifier::slotGotIt()
{
    // qDebug() << Q_FUNC_INFO;

    m_state = KUpdateNotifier::PassiveState;
    setStatus(KStatusNotifierItem::Passive);
    m_gotitaction->setVisible(false);
}

void KUpdateNotifier::slotRefreshCache()
{
    // qDebug() << Q_FUNC_INFO;

    if (m_state == KUpdateNotifier::UpdatesAvaiableState) {
        setOverlayIconByName(QString());
        slotGotIt();
    }

    refreshCache();
}

void KUpdateNotifier::slotUpdatesChanged()
{
    // qDebug() << Q_FUNC_INFO;

    if (m_state == KUpdateNotifier::UpdatesAvaiableState) {
        return;
    }

    const QStringList updates = getUpdates();
    if (!updates.isEmpty()) {
        m_state = KUpdateNotifier::UpdatesAvaiableState;
        setStatus(KStatusNotifierItem::NeedsAttention);
        m_gotitaction->setVisible(true);
        setOverlayIconByName("vcs-update-required");
        QString message = i18n("1 update available");
        if (updates.size() > 1) {
            message = i18n("%1 updates available", updates.size());
        }
        showMessage(i18n("Update notifier"), message, "vcs-update-required");
    }
}

void KUpdateNotifier::slotRestartSchedule()
{
    // qDebug() << Q_FUNC_INFO;

    if (m_state == KUpdateNotifier::RebootScheduledState) {
        return;
    }

    m_state = KUpdateNotifier::RebootScheduledState;
    setStatus(KStatusNotifierItem::NeedsAttention);
    m_gotitaction->setVisible(true);
    setOverlayIconByName("system-reboot");
    showMessage(i18n("Update notifier"), i18n("System restart has been sceduled"), "system-reboot");
}

void KUpdateNotifier::slotPackage(const uint info, const QString &package_id, const QString &summary)
{
    // qDebug() << Q_FUNC_INFO << info << package_id << summary;

    KPackageKitPackage package;
    package.info = info;
    package.package_id = package_id;
    package.summary = summary;
    m_packages.append(package);
}

void KUpdateNotifier::slotErrorCode(const uint code, const QString &details)
{
    kWarning() << code << details;
}

void KUpdateNotifier::slotFinished(const uint exit, const uint runtime)
{
    // qDebug() << Q_FUNC_INFO << exit << runtime;

    m_finished = true;
}
