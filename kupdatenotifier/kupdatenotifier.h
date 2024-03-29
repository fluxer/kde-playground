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

#ifndef KUPDATENOTIFIER_H
#define KUPDATENOTIFIER_H

#include <QDBusInterface>
#include <kstatusnotifieritem.h>
#include <kaction.h>
#include <kmenu.h>
#include <khelpmenu.h>

#define PACKAGEKIT_SERVICE "org.freedesktop.PackageKit"
#define PACKAGEKIT_PATH "/org/freedesktop/PackageKit"
#define PACKAGEKIT_IFACE "org.freedesktop.PackageKit"
#define PACKAGEKIT_TRANSACTION_IFACE "org.freedesktop.PackageKit.Transaction"

struct KPackageKitPackage
{
    uint info;
    QString package_id;
    QString summary;
};

class KUpdateNotifier : public KStatusNotifierItem
{
    Q_OBJECT
public:
    // UpdatesChanged() can be emited multiple times after single cache refresh
    // so tracking the state manually
    enum UpdateNotifierState {
        PassiveState = 0,
        UpdatesAvaiableState = 1,
        RebootScheduledState = 2
    };

    KUpdateNotifier(QObject* parent = nullptr);
    ~KUpdateNotifier();

private Q_SLOTS:
    void slotGotIt();
    void slotRefreshCache();
    void slotUpdatesChanged();
    void slotRestartSchedule();

    void slotPackage(const uint info, const QString &package_id, const QString &summary);
    void slotErrorCode(const uint code, const QString &details);
    void slotFinished(const uint exit, const uint runtime);

private:
    void refreshCache();
    QStringList getUpdates();

    UpdateNotifierState m_state;
    KAction* m_gotitaction;
    KMenu* m_menu;
    KHelpMenu* m_helpmenu;
    QDBusInterface m_interface;
    bool m_finished;
    QList<KPackageKitPackage> m_packages;
};

#endif // KUPDATENOTIFIER_H
