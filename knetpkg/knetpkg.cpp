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

#include <QDir>
#include <kapplication.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include "knetpkg.h"

KNetPkg::KNetPkg(QWidget *parent)
    : KMainWindow(parent)
{
    m_ui.setupUi(this);

    QDir netpkgdir("/usr/pkg/pkgdb");
    foreach (const QFileInfo &netpkginfo, netpkgdir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllDirs)) {
        KNetPkgInfo knetpkginfo;
        knetpkginfo.name = netpkginfo.fileName().toLocal8Bit();
        QFile netpkgdesc(netpkginfo.filePath() + QLatin1String("/+DESC"));
        if (netpkgdesc.open(QFile::ReadOnly)) {
            knetpkginfo.description = netpkgdesc.readAll();
        } else {
            kWarning() << "No description for" << netpkginfo.filePath();
        }
        QFile netpkgrequiredby(netpkginfo.filePath() + QLatin1String("/+REQUIRED_BY"));
        if (netpkgrequiredby.open(QFile::ReadOnly)) {
            knetpkginfo.requiredby = netpkgrequiredby.readAll().trimmed();
            const QList<QByteArray> requiredbylist = knetpkginfo.requiredby.split('\n');
            for (int i = 0; i < requiredbylist.size(); i++) {
                if (i == 0) {
                    knetpkginfo.requiredby = requiredbylist.at(i);
                } else {
                    knetpkginfo.requiredby.append(", ");
                    knetpkginfo.requiredby.append(requiredbylist.at(i));
                }
            }
        } else {
            kDebug() << "No required by for" << netpkginfo.filePath();
        }
        QFile netpkgsize(netpkginfo.filePath() + QLatin1String("/+SIZE_PKG"));
        if (netpkgsize.open(QFile::ReadOnly)) {
            knetpkginfo.size = netpkgsize.readAll().trimmed();
        } else {
            kWarning() << "No size for" << netpkginfo.filePath();
        }
        m_packages.append(knetpkginfo);
    }

    foreach (const KNetPkgInfo &knetpkginfo, m_packages) {
        QListWidgetItem* knetpkgitem = new QListWidgetItem(knetpkginfo.name, m_ui.klistwidget);
    }

    m_ui.klistwidgetsearchline->setListWidget(m_ui.klistwidget);
    connect(m_ui.klistwidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(slotItemChanged(QListWidgetItem*)));
}

KNetPkg::~KNetPkg()
{
}

void KNetPkg::slotItemChanged(QListWidgetItem *knetpkgitem)
{
    // qDebug() << Q_FUNC_INFO << knetpkgitem->text();
    foreach (const KNetPkgInfo &knetpkginfo, m_packages) {
        if (knetpkginfo.name == knetpkgitem->text()) {
            m_ui.requiredbylabel->setText(knetpkginfo.requiredby);
            m_ui.sizelabel->setText(KGlobal::locale()->formatByteSize(knetpkginfo.size.toDouble(), 1));
            m_ui.descriptionwidget->setText(knetpkginfo.description);
            return;
        }
    }
    Q_ASSERT(false);
}

int main(int argc, char **argv)
{
    KAboutData aboutData("knetpkg", 0, ki18n("KNetPkg"),
        "1.0.0", ki18n("NetBSD package information utility"), KAboutData::License_GPL,
        ki18n("(c) 2022 Ivailo Monev"));
    aboutData.addAuthor(ki18n("Ivailo Monev"), KLocalizedString(), "xakepa10@gmail.com");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication app;

    KNetPkg* knetpkg = new KNetPkg();
    knetpkg->show();

    return app.exec();
}

#include "moc_knetpkg.cpp"
