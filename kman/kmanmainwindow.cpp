/*  This file is part of KMan
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

#include <KAction>
#include <KActionCollection>
#include <KLocale>
#include <KIcon>
#include <KDebug>
#include <KStatusBar>
#include <KFileDialog>
#include <KFilterDev>
#include <QThread>
#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QDirIterator>
#include <QStandardPaths>
#include <QProcess>
#include <QMessageBox>
#include <QInputDialog>

#include "kmanmainwindow.h"
#include "ui_kman.h"

static QMap<QString,QString> s_mans;
static const QString groffWorkingDir(QDir::tempPath());

class KManLister : public QThread {
    Q_OBJECT

    public:
        KManLister(QObject *parent = Q_NULLPTR);

    public Q_SLOTS:
        void slotScan(QString path);

    Q_SIGNALS:
        void resultRead(const QString path, const QString fancy);

    protected:
        // reimplementation
        virtual void run();

    private:
        QList<QByteArray> m_paths;
        QFileSystemWatcher m_watcher;
        bool m_interrupt;
};

KManLister::KManLister(QObject *parent)
    : QThread(parent),
    m_interrupt(false) {
    QByteArray manpath = qgetenv("MANPATH");
    if (manpath.isEmpty()) {
        manpath = "/usr/local/share/man:/usr/share/man:/usr/local/man:/usr/man";
    }
    foreach (const QByteArray &path, manpath.split(':')) {
        QFileInfo pathinfo = QFileInfo(path);
        if (!pathinfo.exists()) {
            continue;
        }
        m_paths << path;
        kDebug() << "watching" << path;
        m_watcher.addPath(path);
    }

    connect(&m_watcher, SIGNAL(directoryChanged(QString)), this, SLOT(slotScan(QString)));
}

void KManLister::slotScan(QString path) {
    kDebug() << "rescanning" << path;
    if (isRunning()) {
        m_interrupt = true;
        wait();
    }
    start();
}

void KManLister::run() {
    setPriority(QThread::LowPriority);

    s_mans.clear();
    m_interrupt = false;

    foreach (const QByteArray &path, m_paths) {
        kDebug() << "scanning" << path;

        QDirIterator iterator(path, QDirIterator::Subdirectories);
        while (iterator.hasNext()) {
            if (m_interrupt) {
                return;
            }

            const QString manpage = QDir::cleanPath(iterator.next());
            const QFileInfo info = iterator.fileInfo();
            if (info.isDir()) {
                continue;
            }

            const QString section = info.path().remove(0, path.size() + 1);
            QString fancypage = section + "/" + info.fileName();
            const QString suffix = info.completeSuffix();
            if (!suffix.isEmpty()) {
                fancypage = fancypage.left(fancypage.size() - suffix.size() - 1);
            }

            s_mans.insert(manpage, fancypage);
            emit resultRead(path, fancypage);
        }
    }
}

KManMainWindow::KManMainWindow(QWidget *parent, Qt::WindowFlags flags)
    : KXmlGuiWindow(parent, flags), m_kmanui(new Ui_KManWindow), m_lister(new KManLister(this))
{
    m_kmanui->setupUi(this);

    m_actionopen = actionCollection()->addAction("file_open", this, SLOT(slotOpenAction()));
    m_actionopen->setText(i18n("Open"));
    m_actionopen->setIcon(KIcon("document-open"));
    m_actionopen->setShortcut(KStandardShortcut::open());
    m_actionopen->setWhatsThis(i18n("Open manual page."));

    m_actionquit = actionCollection()->addAction("file_quit", this, SLOT(slotQuitAction()));
    m_actionquit->setText(i18n("Quit"));
    m_actionquit->setIcon(KIcon("application-exit"));
    m_actionquit->setShortcut(KStandardShortcut::quit());
    m_actionquit->setWhatsThis(i18n("Close the application."));

    m_actionnext = actionCollection()->addAction("view_next", this, SLOT(slotNextAction()));
    m_actionnext->setText(i18n("Next"));
    m_actionnext->setIcon(KIcon("go-next"));
    m_actionnext->setShortcut(KStandardShortcut::forward());
    m_actionnext->setWhatsThis(i18n("Switch to the next manual page in the list."));

    m_actionprevious = actionCollection()->addAction("view_previous", this, SLOT(slotPreviousAction()));
    m_actionprevious->setText(i18n("Previous"));
    m_actionprevious->setIcon(KIcon("go-previous"));
    m_actionprevious->setShortcut(KStandardShortcut::back());
    m_actionprevious->setWhatsThis(i18n("Switch to the previous manual page in the list."));

    m_actionfind = actionCollection()->addAction("find_find", this, SLOT(slotFindAction()));
    m_actionfind->setText(i18n("Find"));
    m_actionfind->setIcon(KIcon("edit-find"));
    m_actionfind->setShortcut(KStandardShortcut::find());
    m_actionfind->setWhatsThis(i18n("Find in the manual page."));

    m_actionfindnext = actionCollection()->addAction("find_next", this, SLOT(slotFindNextAction()));
    m_actionfindnext->setText(i18n("Find Next"));
    m_actionfindnext->setIcon(KIcon("go-next"));
    m_actionfindnext->setShortcut(KStandardShortcut::findNext());
    m_actionfindnext->setWhatsThis(i18n("Find next match in the manual page."));

    m_actionfindprevious = actionCollection()->addAction("find_previous", this, SLOT(slotFindPreviousAction()));
    m_actionfindprevious->setText(i18n("Find Previous"));
    m_actionfindprevious->setIcon(KIcon("go-previous"));
    m_actionfindprevious->setShortcut(KStandardShortcut::findPrev());
    m_actionfindprevious->setWhatsThis(i18n("Find previous match in the manual page."));

    setupGUI();
    setAutoSaveSettings();

    setWindowIcon(KIcon("help-browser"));

    connect(m_kmanui->searchEdit, SIGNAL(textChanged(const QString &)),
        this, SLOT(slotSearchChanged(const QString &)));
    connect(m_kmanui->listWidget, SIGNAL(currentTextChanged(const QString &)),
        this, SLOT(slotListChanged(const QString &)));

    connect(m_lister, SIGNAL(started()), this, SLOT(slotBusyStart()));
    connect(m_lister, SIGNAL(resultRead(const QString, const QString)),
        this, SLOT(slotListResult(const QString, const QString)));
    connect(m_lister, SIGNAL(finished()), this, SLOT(slotBusyFinish()));

    listManPages();

    // set current directory to the working directory of groff so that QTextBrowser can load images
    // generated by it as resources
    QDir::setCurrent(groffWorkingDir);
}

KManMainWindow::~KManMainWindow()
{
    saveAutoSaveSettings();
    m_lister->terminate();
    delete m_lister;
    delete m_kmanui;
}

void KManMainWindow::changePath(const QString path) {
    if (path.isEmpty()) {
        return;
    }

    m_path = path;

    const QString content = manContent(path);
    m_kmanui->manView->setHtml(content);

    statusBar()->showMessage(path);

    // TODO: m_kmanui->menuFind->setEnabled(!content.isEmpty());

    const QString fancypage = s_mans.value(path);
    if (!fancypage.isEmpty()) {
        QList<QListWidgetItem*> items = m_kmanui->listWidget->findItems(fancypage, Qt::MatchExactly);
        if (items.count() > 0) {
            m_kmanui->listWidget->setCurrentItem(items.first());
        } else {
            QMessageBox::warning(this, windowTitle(), i18n("not in list %1", fancypage));
        }
    } else {
        kDebug() << "not in cache" << path;
    }

    m_actionnext->setEnabled(false);
    m_actionprevious->setEnabled(false);
    const int position = m_kmanui->listWidget->currentRow();
    if ((position + 1) < m_kmanui->listWidget->count()) {
        m_actionnext->setEnabled(true);
    }
    if (position >= 1) {
        m_actionprevious->setEnabled(true);
    }
}

void KManMainWindow::slotOpenAction() {
    const QString path = KFileDialog::getOpenFileName(KUrl(), QLatin1String("text/troff"), this, i18n("Manual page path"));
    if (!path.isEmpty()) {
        changePath(path);
    }
}

void KManMainWindow::slotQuitAction() {
    qApp->quit();
}

void KManMainWindow::slotNextAction() {
    const int position = m_kmanui->listWidget->currentRow();
    if ((position + 1) < m_kmanui->listWidget->count()) {
        m_kmanui->listWidget->setCurrentRow(position + 1);
    }
}

void KManMainWindow::slotPreviousAction() {
    const int position = m_kmanui->listWidget->currentRow();
    if (position >= 1) {
        m_kmanui->listWidget->setCurrentRow(position - 1);
    }
}

void KManMainWindow::slotFindAction() {
    const QString text = QInputDialog::getText(this, windowTitle(), i18n("Find:"));
    if (text.isEmpty()) {
        return;
    }
    m_search = text;
    m_actionfindnext->setEnabled(true);
    m_actionfindprevious->setEnabled(true);

    if (!m_kmanui->manView->find(text)) {
        QMessageBox::information(this, windowTitle(), i18n("No match was found"));
    }
}

void KManMainWindow::slotFindNextAction() {
    if (m_search.isEmpty()) {
        return;
    }

    if (!m_kmanui->manView->find(m_search)) {
        QMessageBox::information(this, windowTitle(), i18n("No match was found"));
    }
}

void KManMainWindow::slotFindPreviousAction() {
    if (m_search.isEmpty()) {
        return;
    }

    if (!m_kmanui->manView->find(m_search, QTextDocument::FindBackward)) {
        QMessageBox::information(this, windowTitle(), i18n("No match was found"));
    }
}

void KManMainWindow::slotSearchChanged(const QString &text) {
    m_kmanui->listWidget->clear();
    foreach (const QString &man, s_mans.values()) {
        if (text.isEmpty() || man.indexOf(text) >= 0) {
            QListWidgetItem* manitem = new QListWidgetItem(KIcon("application-x-troff-man"), man, m_kmanui->listWidget);
            m_kmanui->listWidget->addItem(manitem);
        }
    }
    m_kmanui->listWidget->sortItems();
}

void KManMainWindow::slotListChanged(const QString &text) {
    const QString manpath = s_mans.key(text);
    if (!manpath.isEmpty()) {
        changePath(manpath);
    } else {
        kDebug() << "not in cache" << text;
    }
}

void KManMainWindow::slotListResult(const QString path, const QString fancy) {
    QListWidgetItem* manitem = new QListWidgetItem(KIcon("application-x-troff-man"), fancy, m_kmanui->listWidget);
    m_kmanui->listWidget->addItem(manitem);
}

void KManMainWindow::slotBusyStart() {
    // TODO: m_kmanui->menuFile->setEnabled(false);
    // TODO: m_kmanui->menuAction->setEnabled(false);
    // TODO: m_kmanui->menuFind->setEnabled(false);
    m_kmanui->searchEdit->setEnabled(false);
    m_kmanui->listWidget->setEnabled(false);
    m_kmanui->progressBar->setRange(0, 0);
    m_kmanui->progressBar->setVisible(true);
}

void KManMainWindow::slotBusyFinish() {
    m_kmanui->listWidget->sortItems();


    m_kmanui->progressBar->setRange(0, 1);
    m_kmanui->progressBar->setVisible(false);
    m_kmanui->listWidget->setEnabled(true);
    m_kmanui->searchEdit->setEnabled(true);
    // TODO: m_kmanui->menuAction->setEnabled(true);
    // TODO: m_kmanui->menuFile->setEnabled(true);
}

void KManMainWindow::listManPages() {
    if (s_mans.isEmpty()) {
        m_lister->start();
    }
}

QString KManMainWindow::manContent(const QString path) {
    QString result;

    if (QStandardPaths::findExecutable("groff").isEmpty()) {
        QMessageBox::warning(this, windowTitle(), i18n("groff is not installed"));
        return result;
    }

    QFile pathfile(path);
    if (pathfile.size() > 100000) {
        QMessageBox::warning(this, windowTitle(), i18n("manual page has size greatar than 10Mb"));
        return result;
    }

    QByteArray content;
    if (path.endsWith(".gz") || path.endsWith(".bz2") || path.endsWith(".xz")) {
        QIODevice *archivedevice = KFilterDev::deviceForFile(path);
        if (archivedevice) {
            if (archivedevice->open(QFile::ReadOnly)) {
                content = archivedevice->readAll();
            }
            archivedevice->deleteLater();
        }
    } else {
        if (pathfile.open(QFile::ReadOnly)) {
            content = pathfile.readAll();
        }
        pathfile.close();
    }

    if (content.isEmpty()) {
        QMessageBox::warning(this, windowTitle(), i18n("could not read %1", path));
        return result;
    }

    slotBusyStart();

    QProcess groff;
    groff.setWorkingDirectory(groffWorkingDir);
    groff.start("groff", QStringList() << "-mandoc" << "-c" << "-Thtml");
    groff.waitForStarted();
    groff.write(content);
    groff.closeWriteChannel();
    while (groff.state() == QProcess::Running) {
        QCoreApplication::processEvents();
    }
    if (groff.exitCode() != 0 ) {
        QMessageBox::warning(this, windowTitle(), groff.readAllStandardError());
    } else {
        result = groff.readAll();
    }

    slotBusyFinish();

    return result;
}

#include "kmanmainwindow.moc"
