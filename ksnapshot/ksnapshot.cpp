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

#include "ksnapshot.h"

#include <QImageWriter>
#include <klocale.h>
#include <kapplication.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kwindowsystem.h>
#include <kfiledialog.h>
#include <kimageio.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "ksnapshot.h"

static const QByteArray s_snapext = '.' + QImageWriter::defaultImageFormat();

KSnapshot::KSnapshot(QWidget *parent)
    : KMainWindow(parent)
{
    m_ui.setupUi(this);

    m_ui.savebutton->setEnabled(false);

    connect(m_ui.savebutton, SIGNAL(clicked()), this, SLOT(slotSaveSnapshot()));
    connect(m_ui.grabbutton, SIGNAL(clicked()), this, SLOT(slotGrabSnapshot()));
}

KSnapshot::~KSnapshot()
{
}

void KSnapshot::snapshot(const KSnapshotMode mode, const int delay)
{
    switch (mode) {
        case KSnapshot::CaptureDesktop: {
            m_ui.modebox->setCurrentIndex(0);
            break;
        }
        case KSnapshot::CaptureActiveWindow: {
            m_ui.modebox->setCurrentIndex(1);
            break;
        }
        default: {
            Q_ASSERT(false);
            break;
        }
    }
    m_ui.delaybox->setValue(delay);
    slotGrabSnapshot();
}

void KSnapshot::slotGrabSnapshot()
{
    m_ui.savebutton->setEnabled(false);
    if (m_ui.delaybox->value() > 0) {
        m_countdown.start(m_ui.delaybox->value());
        QTimer::singleShot(m_ui.delaybox->value() * 1000, this, SLOT(slotGrabPixmap()));
    } else {
        slotGrabPixmap();
    }
}


void KSnapshot::slotSaveSnapshot()
{
    const QString generatedfilename = QLatin1String("ksnapshot_") + QString::number(QDateTime::currentDateTime().toTime_t()) + s_snapext;
    QPointer<KFileDialog> dlg = new KFileDialog(generatedfilename, KImageIO::pattern(KImageIO::Writing), this);

    dlg->setOperationMode(KFileDialog::Saving);
    dlg->setCaption(i18n("Save As"));
    dlg->setMode(KFile::File);
    dlg->setConfirmOverwrite(true);
 
    if (!dlg->exec()) {
        delete dlg;
        return;
    }

    const KUrl fileurl = dlg->selectedUrl();
    if (!fileurl.isValid()) {
        delete dlg;
        return;
    }

    if (!m_snapshot.save(fileurl.toLocalFile())) {
        KMessageBox::error(this, i18n("Could not save snapshot."));
    }

    // qDebug() << Q_FUNC_INFO << fileurl.toLocalFile();
}

void KSnapshot::slotGrabPixmap()
{
    switch (m_ui.modebox->currentIndex()) {
        case 0: {
            m_snapshot = QPixmap::fromX11Pixmap(QX11Info::appRootWindow());
            m_ui.previewlabel->setPixmap(scaledSnapshot());
            break;
        }
        case 1: {
            m_snapshot = QPixmap::fromX11Pixmap(KWindowSystem::activeWindow());
            m_ui.previewlabel->setPixmap(scaledSnapshot());
            break;
        }
        default: {
            Q_ASSERT(false);
            break;
        }
    }

    const bool success = !m_snapshot.isNull();
    if (!success) {
        KMessageBox::error(this, i18n("Could not grab snapshot."));
    } else {
        KWindowSystem::demandAttention(winId());
    }
    m_ui.savebutton->setEnabled(success);

    // qDebug() << Q_FUNC_INFO << modebox->currentIndex() << m_snapshot.size();
}

QPixmap KSnapshot::scaledSnapshot() const
{
    return m_snapshot.scaled(200, 200, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
}

int main(int argc, char **argv)
{
    KAboutData aboutData("ksnapshot", 0, ki18n("KSnapshot"),
        "0.9.0", ki18n("KDE Screenshot Capture Utility"), KAboutData::License_GPL,
        ki18n("(c) 2002-2003 Aaron J. Seigo,\n(c) 2022 Ivailo Monev"));
    aboutData.addAuthor(ki18n("Aaron J. Seigo"), KLocalizedString(), "aseigo@kde.org");
    aboutData.addAuthor(ki18n("Ivailo Monev"), KLocalizedString(), "xakepa10@gmail.com");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineOptions options;
    options.add("current", ki18n("Captures the active window instead of the desktop"));
    options.add("delay <delay>", ki18n("Captures after delay"), "0");
    KCmdLineArgs::addCmdLineOptions(options); // Add our own options.

    KApplication app;

    KSnapshot* ksnapshot = new KSnapshot();
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->isSet("current")) {
        ksnapshot->snapshot(KSnapshot::CaptureActiveWindow, args->getOption("delay").toInt());
    } else {
        ksnapshot->snapshot(KSnapshot::CaptureDesktop, args->getOption("delay").toInt());
    }
    ksnapshot->show();

    return app.exec();
}

#include "moc_ksnapshot.cpp"
