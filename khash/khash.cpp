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
#include <kpushbutton.h>
#include <kurl.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include <QFile>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QRunnable>
#include <QThreadPool>
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include <QClipboard>

#define KHASH_TIMEOUT 100
#define KHASH_SLEEPTIME 50
#define KHASH_BUFFSIZE 1024 * 1000 // 1MB

class KHashRunnable : public QRunnable
{
public:
    KHashRunnable(const QCryptographicHash::Algorithm algorithm, const QString &source, QLabel *label, KPushButton* button, QAtomicInt *interrupt);

protected:
    void run() final;

private:
    QCryptographicHash::Algorithm m_algorithm;
    QString m_source;
    QLabel *m_label;
    KPushButton *m_button;
    QAtomicInt* m_interrupt;
};


KHashRunnable::KHashRunnable(const QCryptographicHash::Algorithm algorithm, const QString &source, QLabel *label, KPushButton* button, QAtomicInt *interrupt)
    : QRunnable(),
    m_algorithm(algorithm),
    m_source(source),
    m_label(label),
    m_button(button),
    m_interrupt(interrupt)
{
}

void KHashRunnable::run()
{
    m_label->setText(i18n("Calculating.."));
    m_button->setEnabled(false);

    QFile checksumfile(m_source);
    if (!checksumfile.open(QFile::ReadOnly)) {
        m_label->setText(checksumfile.errorString());
        return;
    }

    QCryptographicHash checksumer(m_algorithm);
    QByteArray checksumbuffer(KHASH_BUFFSIZE, '\0');
    qint64 checksumfileresult = checksumfile.read(checksumbuffer.data(), checksumbuffer.size());
    while (checksumfileresult > 0) {
        if (m_interrupt->load() != 0) {
            kDebug() << "Checksuming interrupted" << m_source;
            break;
        }

        checksumer.addData(checksumbuffer.constData(), checksumfileresult);

        QCoreApplication::processEvents(QEventLoop::AllEvents, KHASH_TIMEOUT);
        QThread::msleep(KHASH_SLEEPTIME);

        checksumfileresult = checksumfile.read(checksumbuffer.data(), checksumbuffer.size());
    }

    const QByteArray checksumhex = checksumer.result().toHex();
    m_label->setText(QString::fromLatin1(checksumhex.constData(), checksumhex.size()));
    m_button->setEnabled(true);
}


class KHashDialog : public KDialog
{
    Q_OBJECT
public:
    KHashDialog(QWidget *parent = 0);
    ~KHashDialog();

    void setAlgorithm(const QCryptographicHash::Algorithm algorithm);
    void addSource(const KUrl &source);

    void start();

private Q_SLOTS:
    void stop();
    void slotCopyToClipboard();

private:
    QCryptographicHash::Algorithm m_algorithm;
    QList<KUrl> m_sources;
    QWidget* m_dialogwidget;
    QVBoxLayout* m_dialoglayout;
    QList<QLabel*> m_checksumlabels;
    QList<KPushButton*> m_checksumbuttons;
    QThreadPool *m_threadpool;
    QAtomicInt m_interrupt;
};


KHashDialog::KHashDialog(QWidget *parent)
    : KDialog(parent),
    m_algorithm(QCryptographicHash::Sha1),
    m_dialogwidget(nullptr),
    m_dialoglayout(nullptr),
    m_threadpool(nullptr),
    m_interrupt(0)
{
    m_dialogwidget = new QWidget(this);
    m_dialoglayout = new QVBoxLayout(m_dialogwidget);
    m_dialoglayout->setSpacing(KDialog::spacingHint());

    setMainWidget(m_dialogwidget);

    m_threadpool = new QThreadPool(this);

    connect(this, SIGNAL(closeClicked()), this, SLOT(stop()));
}

KHashDialog::~KHashDialog()
{
    stop();
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
    checksumlayout->setSpacing(KDialog::spacingHint());

    checksumgroup->setTitle(sourcefilename);
    QLabel* checksumlabel = new QLabel(i18n("Queued.."), checksumgroup);
    checksumlayout->addWidget(checksumlabel, 0, 0);
    m_checksumlabels.append(checksumlabel);

    checksumlayout->addItem(new QSpacerItem(KDialog::marginHint(), 1, QSizePolicy::Expanding, QSizePolicy::Expanding), 0, 1);

    KPushButton* checksumbutton = new KPushButton(checksumgroup);
    checksumbutton->setIcon(KIcon("edit-copy"));
    checksumbutton->setToolTip(i18n("Copy to clipboard"));
    connect(checksumbutton, SIGNAL(pressed()), this, SLOT(slotCopyToClipboard()));
    checksumlayout->addWidget(checksumbutton, 0, 2);
    m_checksumbuttons.append(checksumbutton);

    m_dialoglayout->addWidget(checksumgroup);
}

void KHashDialog::start()
{
    m_interrupt.store(0);
    int sourcerow = 0;
    foreach (const KUrl &source, m_sources) {
        QLabel* checksumlabel = m_checksumlabels.at(sourcerow);
        KPushButton* checksumbutton = m_checksumbuttons.at(sourcerow);
        m_threadpool->start(new KHashRunnable(m_algorithm, source.toLocalFile(), checksumlabel, checksumbutton, &m_interrupt));
        sourcerow++;
    }
}

void KHashDialog::stop()
{
    m_interrupt.store(1);
    m_threadpool->waitForDone();
}

void KHashDialog::slotCopyToClipboard()
{
    KPushButton* checksumbutton = qobject_cast<KPushButton*>(sender());
    Q_ASSERT(checksumbutton);
    int itcounter = 0;
    foreach (const KPushButton* it, m_checksumbuttons) {
        if (it == checksumbutton) {
            const QString checksumtext = m_checksumlabels.at(itcounter)->text();
            QApplication::clipboard()->setText(checksumtext, QClipboard::Clipboard);
            return;
        }
        itcounter++;
    }
    kWarning() << "Could not find the button";
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
