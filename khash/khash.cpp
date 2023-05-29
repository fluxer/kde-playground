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

class KHashDialog : public KDialog
{
    Q_OBJECT
public:
    KHashDialog(QWidget *parent = 0);
    ~KHashDialog();

    void setAlgorithm(const QCryptographicHash::Algorithm algorithm);
    void setSource(const KUrl &source);

    void start();

private:
    QCryptographicHash::Algorithm m_algorithm;
    KUrl m_source;
    QGridLayout* m_dialoglayout;
    KLineEdit* m_checksumedit;
};


KHashDialog::KHashDialog(QWidget *parent)
    : KDialog(parent),
    m_algorithm(QCryptographicHash::Sha1),
    m_dialoglayout(nullptr),
    m_checksumedit(nullptr)
{
    QWidget* dialogwidget = new QWidget(this);
    m_dialoglayout = new QGridLayout(dialogwidget);

    QLabel* checksumlabel = new QLabel(i18n("Checksum:"), dialogwidget);
    m_dialoglayout->addWidget(checksumlabel, 0, 0);

    m_checksumedit = new KLineEdit(dialogwidget);
    m_checksumedit->setReadOnly(true);
    m_dialoglayout->addWidget(m_checksumedit, 0, 1);

    // TODO: progress bar and threading

    setMainWidget(dialogwidget);
}

KHashDialog::~KHashDialog()
{
}

void KHashDialog::setAlgorithm(const QCryptographicHash::Algorithm algorithm)
{
    m_algorithm = algorithm;
    QFontMetrics dialoglinemetric(m_checksumedit->font());
    switch (algorithm) {
        case QCryptographicHash::Md5: {
            m_dialoglayout->setColumnMinimumWidth(1, 32 * dialoglinemetric.width('x'));
            break;
        }
        case QCryptographicHash::Sha1: {
            m_dialoglayout->setColumnMinimumWidth(1, 40 * dialoglinemetric.width('x'));
            break;
        }
        case QCryptographicHash::Sha256: {
            m_dialoglayout->setColumnMinimumWidth(1, 64 * dialoglinemetric.width('x'));
            break;
        }
        case QCryptographicHash::Sha512: {
            // NOTE: it is 128 long but don't want it to extend too much (small screens)
            m_dialoglayout->setColumnMinimumWidth(1, 64 * dialoglinemetric.width('x'));
            break;
        }
        case QCryptographicHash::KAT: {
            m_dialoglayout->setColumnMinimumWidth(1, 64 * dialoglinemetric.width('x'));
            break;
        }
    }
}

void KHashDialog::setSource(const KUrl &source)
{
    m_source = source;
    setCaption(KDialog::makeStandardCaption(QFileInfo(source.prettyUrl()).fileName(), this));
}

void KHashDialog::start()
{
    QFile checksumfile(m_source.toLocalFile());
    if (!checksumfile.open(QFile::ReadOnly)) {
        m_checksumedit->setText(checksumfile.errorString());
        return;
    }
    QCryptographicHash checksumer(m_algorithm);
    if (!checksumer.addData(&checksumfile)) {
        m_checksumedit->setText(i18n("Checksummer error"));
        return;
    }
    const QByteArray checksumhex = checksumer.result().toHex();
    m_checksumedit->setText(checksumhex);
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
        khashdialog.setSource(args->url(pos));
        shouldstart = true;
    }

    khashdialog.show();
    khashdialog.setButtons(KDialog::Ok | KDialog::Close | KDialog::Help);
    KHelpMenu khelpmenu(&khashdialog, &aboutData, true);
    khashdialog.setButtonMenu(KDialog::Help, (QMenu*)khelpmenu.menu());

    if (shouldstart) {
        khashdialog.start();
    }

    return app.exec();
}

#include "khash.moc"
