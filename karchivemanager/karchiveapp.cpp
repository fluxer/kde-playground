/*  This file is part of KArchiveManager
    Copyright (C) 2018 Ivailo Monev <xakepa10@gmail.com>

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

#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <KFileDialog>
#include <KDebug>
#include <KLocale>

#include "karchivemanager.hpp"
#include "karchiveapp.hpp"
#include "ui_karchiveapp.h"

static const QStringList s_readwritemimes = QStringList()
    << "application/x-archive"
    << "application/x-deb"
    << "application/x-cd-image"
    << "application/x-bcpio"
    << "application/x-cpio"
    << "application/x-cpio-compressed"
    << "application/x-sv4cpio"
    << "application/x-sv4crc"
    << "application/x-rpm"
    << "application/x-source-rpm"
    << "application/vnd.ms-cab-compressed"
    << "application/x-servicepack"
    << "application/x-lzop"
    << "application/x-lz4"
    << "application/x-tar"
    << "application/zstd"
    << "application/x-zstd-compressed-tar"
    << "application/x-compressed-tar"
    << "application/x-bzip-compressed-tar"
    << "application/x-gzip-compressed-tar"
    << "application/x-tarz"
    << "application/x-xz"
    << "application/x-xz-compressed-tar"
    << "application/x-lzma-compressed-tar"
    << "application/x-java-archive"
    << "application/zip"
    << "application/x-7z-compressed"
    << "application/x-iso9660-image"
    << "application/x-raw-disk-image";

class KArchiveAppPrivate {

    public:
        Ui_KArchiveAppWindow ui;

        KArchiveModel m_model;
        KArchiveManager *m_archive;
};

KArchiveApp::KArchiveApp()
    :  d(new KArchiveAppPrivate()) {
    d->ui.setupUi(this);
    show();

    d->ui.archiveView->setModel(&d->m_model);

    connect(d->ui.actionOpen, SIGNAL(triggered()), this, SLOT(slotOpenAction()));
    d->ui.actionOpen->setShortcut(QKeySequence::Open);
    connect(d->ui.actionQuit, SIGNAL(triggered()), this, SLOT(slotQuitAction()));
    d->ui.actionQuit->setShortcut(QKeySequence::Quit);

    connect(d->ui.actionAdd, SIGNAL(triggered()), this, SLOT(slotAddAction()));
    connect(d->ui.actionRemove, SIGNAL(triggered()), this, SLOT(slotRemoveAction()));
    connect(d->ui.actionExtract, SIGNAL(triggered()), this, SLOT(slotExtractAction()));

    connect(d->ui.archiveView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SLOT(slotSelectionChanged(const QItemSelection&, const QItemSelection&)));
    connect(&d->m_model, SIGNAL(loadStarted()), this, SLOT(slotLoadStarted()));
    connect(&d->m_model, SIGNAL(loadFinished()), this, SLOT(slotLoadFinished()));
}

KArchiveApp::~KArchiveApp() {
    if (d->m_archive) {
        delete d->m_archive;
    }
    delete d;
}

void KArchiveApp::changePath(const QString path) {
    if (d->m_archive) {
        delete d->m_archive;
    }

    d->m_archive = new KArchiveManager(path);
    d->m_model.loadArchive(d->m_archive);

    statusBar()->showMessage(path);

    const bool iswritable = d->m_archive->writable();
    d->ui.actionAdd->setEnabled(iswritable);
    d->ui.actionRemove->setEnabled(iswritable);
    d->ui.actionExtract->setEnabled(iswritable);
}

void KArchiveApp::slotOpenAction() {
    QString mimespattern;
    foreach(const QString &mimetype, s_readwritemimes) {
        const KMimeType::Ptr mime = KMimeType::mimeType(mimetype);
        if (mime) {
            if (!mimespattern.isEmpty()) {
                mimespattern.append(" ");
            }
            mimespattern.append(mime->patterns().join(" "));
        }
    }

    const QString path = KFileDialog::getOpenFileName(KUrl("kfiledialog:///KArchiveManager"), mimespattern, this, i18n("Archive path"));
    if (!path.isEmpty()) {
        changePath(path);
    }
}

void KArchiveApp::slotQuitAction() {
    qApp->quit();
}

void KArchiveApp::slotAddAction() {
    QFileDialog opendialog(this, windowTitle());
    opendialog.setFileMode(QFileDialog::ExistingFiles);
    opendialog.exec();
    const QStringList selected = opendialog.selectedFiles();
    if (!opendialog.result() || selected.isEmpty()) {
        return;
    }

    kDebug() << "adding" << selected;
    const QByteArray stripdir = opendialog.directory().path().toUtf8();
    QString destdir;
    foreach (const QModelIndex &item, d->ui.archiveView->selectionModel()->selectedIndexes()) {
        destdir = d->m_model.dir(item);
    }
    d->m_archive->add(selected, stripdir + "/", destdir.toUtf8() + "/");

    kDebug() << "reloading archive";
    d->m_model.loadArchive(d->m_archive);
}

void KArchiveApp::slotRemoveAction() {
    QStringList selected;
    foreach (const QModelIndex &item, d->ui.archiveView->selectionModel()->selectedIndexes()) {
        selected += d->m_model.paths(item);
    }

    QMessageBox::StandardButton answer = QMessageBox::question(this, windowTitle(),
        QApplication::tr("Are you sure you want to delete:\n\n%1?").arg(selected.join("\n")),
        QMessageBox::No | QMessageBox::Yes);
    if (answer != QMessageBox::Yes) {
        return;
    }

    kDebug() << "removing" << selected;
    d->m_archive->remove(selected);

    kDebug() << "reloading archive";
    d->m_model.loadArchive(d->m_archive);
}

void KArchiveApp::slotExtractAction() {
    QStringList selected;
    foreach (const QModelIndex &item, d->ui.archiveView->selectionModel()->selectedIndexes()) {
        selected += d->m_model.paths(item);
    }

    const QString destination = QFileDialog::getExistingDirectory(this, windowTitle());
    if (destination.isEmpty()) {
        return;
    }

    kDebug() << "extracting" << selected << "to" << destination;
    d->m_archive->extract(selected, destination, true);
}

void KArchiveApp::slotSelectionChanged(const QItemSelection &current, const QItemSelection &previous) {
    Q_UNUSED(previous);
    d->ui.menuAction->setEnabled(true);

    if (current.indexes().isEmpty()) {
        d->ui.menuAction->setEnabled(false);
    }
}

void KArchiveApp::slotLoadStarted() {
    d->ui.menuAction->setEnabled(false);
    d->ui.archiveView->setEnabled(false);
    d->ui.progressBar->setRange(0, 0);
    d->ui.progressBar->setVisible(true);

    QHeaderView* header = d->ui.archiveView->header();
    if (header) {
        header->setVisible(false);
    }
}

void KArchiveApp::slotLoadFinished() {
    d->ui.archiveView->setEnabled(true);
    d->ui.progressBar->setRange(0, 1);
    d->ui.progressBar->setVisible(false);

    QHeaderView* header = d->ui.archiveView->header();
    if (header && header->count() > 0) {
        header->setVisible(true);
        header->setResizeMode(0, QHeaderView::Stretch);
    }

    d->m_model.sort(1, Qt::AscendingOrder);
}
