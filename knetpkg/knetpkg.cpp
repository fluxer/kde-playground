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

static QByteArray pkgLink(const QByteArray &netpkg)
{
    return QString::fromLatin1("<a href=\"%1\">%1</a>").arg(netpkg.constData()).toLatin1();
}

KNetPkg::KNetPkg(QWidget *parent)
    : KMainWindow(parent)
{
    m_ui.setupUi(this);

    QDir netpkgdir("/usr/pkg/pkgdb");
    foreach (const QFileInfo &netpkginfo, netpkgdir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllDirs)) {
        KNetPkgInfo knetpkginfo;
        knetpkginfo.name = netpkginfo.fileName().toLocal8Bit();
        QFile netpkgcomment(netpkginfo.filePath() + QLatin1String("/+COMMENT"));
        if (netpkgcomment.open(QFile::ReadOnly)) {
            knetpkginfo.comment = netpkgcomment.readAll().trimmed();
        } else {
            kWarning() << "No comment for" << netpkginfo.filePath();
        }
        QFile netpkgrequiredby(netpkginfo.filePath() + QLatin1String("/+REQUIRED_BY"));
        if (netpkgrequiredby.open(QFile::ReadOnly)) {
            knetpkginfo.requiredby = netpkgrequiredby.readAll().trimmed();
            const QList<QByteArray> requiredbylist = knetpkginfo.requiredby.split('\n');
            for (int i = 0; i < requiredbylist.size(); i++) {
                if (i == 0) {
                    knetpkginfo.requiredby = pkgLink(requiredbylist.at(i));
                } else {
                    knetpkginfo.requiredby.append(", ");
                    knetpkginfo.requiredby.append(pkgLink(requiredbylist.at(i)));
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
        QFile netpkgcontents(netpkginfo.filePath() + QLatin1String("/+CONTENTS"));
        if (netpkgcontents.open(QFile::ReadOnly)) {
            knetpkginfo.contents = netpkgcontents.readAll().trimmed();
            const QList<QByteArray> contentslist = knetpkginfo.contents.split('\n');
            knetpkginfo.contents.clear();
            for (int i = 0; i < contentslist.size(); i++) {
                const QByteArray contentsline = contentslist.at(i);
                if (contentsline.isEmpty() || contentsline.startsWith('@') || contentsline.startsWith('+')) {
                    continue;
                }
                knetpkginfo.contents.append(contentsline);
                knetpkginfo.contents.append('\n');
            }
        } else {
            kWarning() << "No contents for" << netpkginfo.filePath();
        }
        m_packages.append(knetpkginfo);
    }

    foreach (const KNetPkgInfo &knetpkginfo, m_packages) {
        QListWidgetItem* knetpkgitem = new QListWidgetItem(knetpkginfo.name, m_ui.klistwidget);
    }

    m_ui.klistwidgetsearchline->setListWidget(m_ui.klistwidget);
    connect(
        m_ui.klistwidget, SIGNAL(currentTextChanged(QString)),
        this, SLOT(slotCurrentTextChanged(QString))
    );
    connect(
        m_ui.requiredbylabel, SIGNAL(linkActivated(QString)),
        this, SLOT(slotLinkActivated(QString))
    );
}

KNetPkg::~KNetPkg()
{
}

void KNetPkg::slotCurrentTextChanged(const QString &netpkg)
{
    // qDebug() << Q_FUNC_INFO << netpkg;
    foreach (const KNetPkgInfo &knetpkginfo, m_packages) {
        if (knetpkginfo.name == netpkg) {
            m_ui.commentlabel->setText(knetpkginfo.comment);
            m_ui.requiredbylabel->setText(knetpkginfo.requiredby);
            m_ui.sizelabel->setText(KGlobal::locale()->formatByteSize(knetpkginfo.size.toDouble(), 1));
            m_ui.contentswidget->setText(knetpkginfo.contents);
            return;
        }
    }
    Q_ASSERT(false);
}

void KNetPkg::slotLinkActivated(const QString &link)
{
    // qDebug() << Q_FUNC_INFO << link;
    for (int i = 0; i < m_ui.klistwidget->count(); i++) {
        if (m_ui.klistwidget->item(i)->text() == link) {
            m_ui.klistwidget->setCurrentRow(i);
            break;
        }
    }
}

int main(int argc, char **argv)
{
    KAboutData aboutData(
        "knetpkg", 0, ki18n("KNetPkg"),
        "1.0.0", ki18n("NetBSD package information utility"), KAboutData::License_GPL,
        ki18n("(c) 2022 Ivailo Monev")
    );
    aboutData.addAuthor(ki18n("Ivailo Monev"), KLocalizedString(), "xakepa10@gmail.com");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication app;

    KNetPkg* knetpkg = new KNetPkg();
    knetpkg->show();

    return app.exec();
}

#include "moc_knetpkg.cpp"
