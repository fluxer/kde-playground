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

#include "kded_kfirewall.h"

#include <QJsonDocument>
#include <QFile>
#include <kstandarddirs.h>
#include <kauthaction.h>
#include <kpluginfactory.h>
#include <kdebug.h>

K_PLUGIN_FACTORY(KFirewallModuleFactory, registerPlugin<KFirewallModule>();)
K_EXPORT_PLUGIN(KFirewallModuleFactory("kfirewall"))

KFirewallModule::KFirewallModule(QObject *parent, const QList<QVariant>&)
    : KDEDModule(parent),
    m_kfirewallconfigpath(KStandardDirs::locateLocal("data", "kfirewall.json")),
    m_watcher(this)
{
    enable();

    connect(
        &m_watcher, SIGNAL(fileChanged(QString)),
        this, SLOT(slotFileChanged(QString))
    );
    m_watcher.addPath(m_kfirewallconfigpath);
}

KFirewallModule::~KFirewallModule()
{
    disable();
}

bool KFirewallModule::enable()
{
    m_kfirewallsettingsmap.clear();
    QFile kfirewallfile(m_kfirewallconfigpath);
    if (!kfirewallfile.open(QFile::ReadOnly)) {
        // may not exist yet but if it is created eventually it will be applied
        kDebug() << "Could not open config for reading" << kfirewallfile.errorString();
        return true;
    }

    const QByteArray kfirewalljsondata = kfirewallfile.readAll();
    QJsonDocument kfirewalljsondocument = QJsonDocument::fromJson(kfirewalljsondata);
    if (!kfirewalljsondata.isEmpty() && kfirewalljsondocument.isNull()) {
        kWarning() << "Could not create JSON document" << kfirewalljsondocument.errorString();
        return false;
    }
    m_kfirewallsettingsmap = kfirewalljsondocument.toVariant().toMap();
    if (m_kfirewallsettingsmap.isEmpty()) {
        kDebug() << "No firewall rules";
        return true;
    }

    KAuth::Action kfirewallaction("org.kde.kcontrol.kcmkfirewall.apply");
    kfirewallaction.setHelperID("org.kde.kcontrol.kcmkfirewall");
    kfirewallaction.setArguments(m_kfirewallsettingsmap);
    KAuth::ActionReply kfirewallreply = kfirewallaction.execute();
    // qDebug() << Q_FUNC_INFO << kfirewallreply.errorCode() << kfirewallreply.errorDescription();

    if (kfirewallreply != KAuth::ActionReply::SuccessReply) {
        kWarning() << kfirewallreply.errorCode() << kfirewallreply.errorDescription();
        return false;
    }

    return true;
}

bool KFirewallModule::disable()
{
    if (m_kfirewallsettingsmap.isEmpty()) {
        kDebug() << "No firewall rules";
        return true;
    }

    KAuth::Action kfirewallaction("org.kde.kcontrol.kcmkfirewall.revert");
    kfirewallaction.setHelperID("org.kde.kcontrol.kcmkfirewall");
    kfirewallaction.setArguments(m_kfirewallsettingsmap);
    KAuth::ActionReply kfirewallreply = kfirewallaction.execute();
    // qDebug() << Q_FUNC_INFO << kfirewallreply.errorCode() << kfirewallreply.errorDescription();

    if (kfirewallreply != KAuth::ActionReply::SuccessReply) {
        kWarning() << kfirewallreply.errorCode() << kfirewallreply.errorDescription();
        return false;
    }

    m_kfirewallsettingsmap.clear();
    return true;
}

void KFirewallModule::slotFileChanged(const QString &path)
{
    Q_UNUSED(path);
    disable();
    enable();
}

#include "moc_kded_kfirewall.cpp"
