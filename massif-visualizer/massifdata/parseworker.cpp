/*
   This file is part of Massif Visualizer

   Copyright 2012 Milian Wolff <mail@milianw.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "parseworker.h"
#include "parser.h"
#include "filedata.h"

#include <KUrl>
#include <KFilterDev>
#include <KIO/NetAccess>
#include <KLocalizedString>

namespace Massif {

ParseWorker::ParseWorker(QObject* parent)
: QObject(parent)
{

}

void ParseWorker::parse(const KUrl& url, const QStringList& allocators)
{
    // process in background thread
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(this, "parse", Q_ARG(KUrl, url), Q_ARG(QStringList, allocators));
        return;
    }

    m_shouldStop = 0;
    QString file;
    if (!url.isLocalFile()) {
        if (!KIO::NetAccess::download(url, file, 0)) {
            emit error(i18n("Download Failed"),
                       i18n("Failed to download remote massif data file <i>%1</i>.", url.pathOrUrl()));
            return;
        }
    } else {
        file = url.toLocalFile();
    }

    QScopedPointer<QIODevice> device(KFilterDev::deviceForFile(file));
    if (!device->open(QIODevice::ReadOnly)) {
        emit error(i18n("Read Failed"),
                   i18n("Could not open file <i>%1</i> for reading.", file));
        return;
    }

    Parser p;
    emit progressRange(0, 100);
    connect(&p, SIGNAL(progress(int)), this, SIGNAL(progress(int)));
    QScopedPointer<FileData> data(p.parse(device.data(), allocators, &m_shouldStop));

    if (!data) {
        if (!m_shouldStop) {
            emit error(i18n("Parser Failed"),
                       i18n("Could not parse file <i>%1</i>.<br>"
                            "Parse error in line %2:<br>%3",
                            url.pathOrUrl(), p.errorLine() + 1,
                            QString::fromLatin1(p.errorLineString())));
        }
        return;
    } else if (data->snapshots().isEmpty()) {
        emit error(i18n("Empty data file <i>%1</i>.", url.pathOrUrl()),
                   i18n("Empty Data File"));
        return;
    }

    emit progress(100);

    // success!
    emit finished(url, data.take());
}

void ParseWorker::stop()
{
    m_shouldStop = 1;
}

}

#include "parseworker.moc"
