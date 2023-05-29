/* This file is part of the KDE project
   Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <kapplication.h>
#include <kdialog.h>
#include <khelpmenu.h>
#include <klineedit.h>
#include <kurl.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include <QFile>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>

class KHashDialog : public KDialog
{
    Q_OBJECT
public:
    KHashDialog(QWidget *parent = 0);
    ~KHashDialog();

    void setAlgorithm(const QCryptographicHash::Algorithm algorithm);
    void addSource(const KUrl &source);

    void start();

private:
    QCryptographicHash::Algorithm m_algorithm;
    QList<KUrl> m_sources;
    QWidget* m_dialogwidget;
    QVBoxLayout* m_dialoglayout;
    QList<KLineEdit*> m_checksumedits;
};


KHashDialog::KHashDialog(QWidget *parent)
    : KDialog(parent),
    m_algorithm(QCryptographicHash::Sha1),
    m_dialogwidget(nullptr),
    m_dialoglayout(nullptr)
{
    m_dialogwidget = new QWidget(this);
    m_dialoglayout = new QVBoxLayout(m_dialogwidget);

    setMainWidget(m_dialogwidget);
}

KHashDialog::~KHashDialog()
{
}

void KHashDialog::setAlgorithm(const QCryptographicHash::Algorithm algorithm)
{
    m_algorithm = algorithm;
}

void KHashDialog::addSource(const KUrl &source)
{
    m_sources.append(source);
    const QString sourcefilename = QFileInfo(source.prettyUrl()).fileName();
    setCaption(KDialog::makeStandardCaption(sourcefilename, this));

    QGroupBox* checksumgroup = new QGroupBox(m_dialogwidget);
    QGridLayout* checksumlayout = new QGridLayout(checksumgroup);

    checksumgroup->setTitle(sourcefilename);
    QLabel* checksumlabel = new QLabel(i18n("Checksum:"), checksumgroup);
    checksumlayout->addWidget(checksumlabel, 0, 0);

    KLineEdit* checksumedit = new KLineEdit(checksumgroup);
    checksumedit->setReadOnly(true);
    checksumedit->setText(i18n("Queued.."));
    checksumlayout->addWidget(checksumedit, 0, 1);
    m_checksumedits.append(checksumedit);

    m_dialoglayout->addWidget(checksumgroup);
}

void KHashDialog::start()
{
    int sourcerow = 0;
    foreach (const KUrl &source, m_sources) {
        KLineEdit* checksumedit = m_checksumedits.at(sourcerow);
        checksumedit->setText(i18n("Calculating.."));
        // TODO: threading
        QFile checksumfile(source.toLocalFile());
        if (!checksumfile.open(QFile::ReadOnly)) {
            checksumedit->setText(checksumfile.errorString());
            sourcerow++;
            continue;
        }
        QCryptographicHash checksumer(m_algorithm);
        if (!checksumer.addData(&checksumfile)) {
            checksumedit->setText(i18n("Checksumer error"));
            sourcerow++;
            continue;
        }
        const QByteArray checksumhex = checksumer.result().toHex();
        checksumedit->setText(checksumhex);
        sourcerow++;
    }
}


int main(int argc, char **argv)
{
    KAboutData aboutData(
        "security-high", 0, ki18n("KHash"),
        "1.0.0", ki18n("KDE checksum calculation utility"), KAboutData::License_GPL,
        ki18n("(c) 2023 Ivailo Monev")
    );
    aboutData.addAuthor(ki18n("Ivailo Monev"), KLocalizedString(), "xakepa10@gmail.com");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineOptions options;
    options.add("algorithm <algorithm>", ki18n("Checksum algorithm to use, default value"), "sha1");
    options.add("+[url]", ki18n("URL to be opened"));
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    KHashDialog khashdialog;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    const QString hashtype = args->getOption("algorithm");
    if (hashtype == QLatin1String("md5")) {
        khashdialog.setAlgorithm(QCryptographicHash::Md5);
    } else if (hashtype == QLatin1String("sha1")) {
        khashdialog.setAlgorithm(QCryptographicHash::Sha1);
    } else if (hashtype == QLatin1String("sha256")) {
        khashdialog.setAlgorithm(QCryptographicHash::Sha256);
    } else if (hashtype == QLatin1String("sha512")) {
        khashdialog.setAlgorithm(QCryptographicHash::Sha512);
    } else if (hashtype == QLatin1String("kat")) {
        khashdialog.setAlgorithm(QCryptographicHash::KAT);
    } else {
        KCmdLineArgs::usageError(i18n("Algorithm must be one of: md5, sha1, sha256, sha512 or kat"));
        return 1;
    }

    bool shouldstart = false;
    for (int pos = 0; pos < args->count(); ++pos) {
        khashdialog.addSource(args->url(pos));
        shouldstart = true;
    }

    khashdialog.show();
    khashdialog.setButtons(KDialog::Close | KDialog::Help);
    KHelpMenu khelpmenu(&khashdialog, &aboutData, true);
    khashdialog.setButtonMenu(KDialog::Help, (QMenu*)khelpmenu.menu());

    if (shouldstart) {
        khashdialog.start();
    }

    return app.exec();
}

#include "khash.moc"
