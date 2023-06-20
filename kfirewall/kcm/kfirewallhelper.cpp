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

#include "kfirewallhelper.h"

#include <QProcess>
#include <kstandarddirs.h>
#include <kdebug.h>

static QByteArray ruleForSettings(const QByteArray &uservalue, const QByteArray &trafficvalue,
                                  const QByteArray &addressvalue, const uint portvalue,
                                  const QByteArray &actionvalue, const bool appendrules, const bool tcprule)
{
    if (!tcprule && portvalue <= 0) {
        // no port specified
        return QByteArray();
    }

    QByteArray iptablesruledata;
    bool isinbound = false;
    QByteArray iptablestraffic = trafficvalue.toUpper();
    if (iptablestraffic == "INBOUND") {
        iptablestraffic = "INPUT";
        isinbound = true;
    } else {
        iptablestraffic = "OUTPUT";
    }

    if (appendrules) {
        iptablesruledata.append("--append ");
    } else {
        iptablesruledata.append("--delete ");
    }
    iptablesruledata.append(iptablestraffic);
    if (!addressvalue.isEmpty()) {
        iptablesruledata.append(" --destination ");
        iptablesruledata.append(addressvalue);
    }
    if (portvalue > 0) {
        if (tcprule) {
            iptablesruledata.append(" --proto tcp --dport ");
            iptablesruledata.append(QByteArray::number(portvalue));
        } else {
            iptablesruledata.append(" --proto udp --dport ");
            iptablesruledata.append(QByteArray::number(portvalue));
        }
    }
    if (!isinbound) {
        // NOTE: only output can be user-bound
        iptablesruledata.append(" --match owner --uid-owner ");
        iptablesruledata.append(uservalue);
    }
    iptablesruledata.append(" --jump ");
    iptablesruledata.append(actionvalue.toUpper());
    iptablesruledata.append("\n");
    return iptablesruledata;
}

static QByteArray rulesForParameters(const QVariantMap &parameters, const bool appendrules)
{
    QByteArray iptablesruledata("*filter\n");
    foreach (const QString &key, parameters.keys()) {
        const QVariantMap rulesettingsmap = parameters.value(key).toMap();
        const QByteArray uservalue = rulesettingsmap.value(QString::fromLatin1("user")).toByteArray();
        const QByteArray trafficvalue = rulesettingsmap.value(QString::fromLatin1("traffic")).toByteArray();
        const QByteArray addressvalue = rulesettingsmap.value(QString::fromLatin1("address")).toByteArray();
        const uint portvalue = rulesettingsmap.value(QString::fromLatin1("port")).toUInt();
        const QByteArray actionvalue = rulesettingsmap.value(QString::fromLatin1("action")).toByteArray();
        // qDebug() << Q_FUNC_INFO << trafficvalue << addressvalue << portvalue << actionvalue;

        iptablesruledata.append(
            ruleForSettings(
                uservalue, trafficvalue, addressvalue, portvalue, actionvalue,
                appendrules, true
            )
        );
        iptablesruledata.append(
            ruleForSettings(
                uservalue, trafficvalue, addressvalue, portvalue, actionvalue,
                appendrules, false
            )
        );
    }
    iptablesruledata.append("COMMIT\n");
    // qDebug() << Q_FUNC_INFO << iptablesruledata;
    return iptablesruledata;
}

static int applyRules(KFirewallHelper *helper, const QString &iptablesexe,
                              const QByteArray &iptablesruledata)
{
    QProcess iptablesproc(helper);
    iptablesproc.start(iptablesexe);
    if (!iptablesproc.waitForStarted()) {
        kWarning() << "Could not start iptables-restore";
        return KAuthorization::HelperError;
    }
    if (iptablesproc.write(iptablesruledata) != iptablesruledata.size()) {
        kWarning() << "Could not write rules";
        return KAuthorization::HelperError;
    }
    iptablesproc.closeWriteChannel();
    iptablesproc.waitForFinished();
    if (iptablesproc.exitCode() != 0) {
        QString errorstring = iptablesproc.readAllStandardError().trimmed();
        if (errorstring.isEmpty()) {
            errorstring = QString::fromLatin1("Could not apply rules");
        }
        kWarning() << errorstring;
        return KAuthorization::HelperError;
    }

    return KAuthorization::NoError;
}

KFirewallHelper::KFirewallHelper(const char* const helper, QObject *parent)
    : KAuthorization(helper, parent)
{
}

int KFirewallHelper::apply(const QVariantMap &parameters)
{
    if (parameters.isEmpty()) {
        kWarning() << "Empty rules";
        return KAuthorization::HelperError;
    }

    const QString iptablesexe = KStandardDirs::findRootExe("iptables-restore");
    if (iptablesexe.isEmpty()) {
        kWarning() << "Could not find iptables-restore";
        return KAuthorization::HelperError;
    }

    return applyRules(this, iptablesexe, rulesForParameters(parameters, true));
}

int KFirewallHelper::revert(const QVariantMap &parameters)
{
    //qDebug() << Q_FUNC_INFO << parameters;

    if (parameters.isEmpty()) {
        kWarning() << "Empty rules";
        return KAuthorization::HelperError;
    }

    const QString iptablesexe = KStandardDirs::findRootExe("iptables-restore");
    if (iptablesexe.isEmpty()) {
        kWarning() << "Could not find iptables-restore";
        return KAuthorization::HelperError;
    }

    return applyRules(this, iptablesexe, rulesForParameters(parameters, false));
}

K_AUTH_MAIN("org.kde.kcontrol.kcmkfirewall", KFirewallHelper)
