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

#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QTemporaryFile>
#include <QDirIterator>
#include <QThread>
#include <QStandardItem>
#include <QDBusMetaType>
#include <KDebug>
#include <KMimeType>
#include <KIconLoader>

#include "karchivemanager.hpp"

#include <string.h>
#include <sys/stat.h>
#include <archive.h>
#include <archive_entry.h>
#include <zlib.h>
#include <bzlib.h>

extern "C" { void strmode(mode_t mode, char *str); };

#define KARCHIVEMANAGER_BUFFSIZE 10240

static const QStringList s_readwrite = QStringList()
#if ARCHIVE_VERSION_NUMBER > 3000004
    << "application/x-lzop"
#endif
#if ARCHIVE_VERSION_NUMBER > 3001002
    << "application/x-lz4"
#endif
    << "application/x-tar"
    << "application/x-compressed-tar"
    << "application/x-bzip"
    << "application/x-gzip"
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
    << "application/x-apple-diskimage"
    << "application/x-cd-image"
    << "application/x-raw-disk-image";

KArchiveInfo::KArchiveInfo()
    : encrypted(false),
    size(0),
    gid(-1),
    uid(-1),
    mode(0),
    type(KArchiveInfo::None) {
}

KArchiveInfo::KArchiveInfo(const KArchiveInfo &info)
    : encrypted(info.encrypted),
    size(info.size),
    gid(info.gid),
    uid(info.uid),
    mode(info.mode),
    atime(info.atime),
    ctime(info.ctime),
    mtime(info.mtime),
    hardlink(info.hardlink),
    symlink(info.symlink),
    pathname(info.pathname),
    mimetype(info.mimetype),
    type(info.type) {
}

QString KArchiveInfo::fancyEncrypted() const {
    if (encrypted) {
        return QString("yes");
    }
    return QString("No");
}

// it does not make very accurate estimations but is good enough
QString KArchiveInfo::fancySize() const {
    QString result;

    const int64_t bsize = 1024;
    const int64_t kbsize = 1024 * 1024;
    const int64_t mbsize = 1024 * 1024 * 1024;

    if (size < bsize) {
        result = QString::number(size) + " bytes";
    } else if (size < kbsize) {
        result = QString::number(size / bsize) + " Kb";
    } else if (size < mbsize) {
        result = QString::number(size / kbsize) + " Mb";
    } else {
        result = QString::number(size / mbsize) + " Gb";
    }

    return result;
}

QString KArchiveInfo::fancyMode() const {
    if (mode == 0) {
        return QString("---------");
    }

    char buffer[20];
    ::strmode(mode, buffer);

    return QString::fromLatin1(buffer);
}

QString KArchiveInfo::fancyType() const {
    switch (type) {
        case KArchiveType::None: {
            return "None";
        }
        case KArchiveType::File: {
            return "File";
        }
        case KArchiveType::Directory: {
            return "Directory";
        }
        case KArchiveType::Link: {
            return "Link";
        }
        case KArchiveType::Character: {
            return "Character";
        }
        case KArchiveType::Block: {
            return "Block";
        }
        case KArchiveType::Fifo: {
            return "Fifo";
        }
        case KArchiveType::Socket: {
            return "Socket";
        }
    }

    Q_UNREACHABLE();
    return QString();
}

QIcon KArchiveInfo::fancyIcon() const {
    switch (type) {
        case KArchiveType::None: {
            return QIcon::fromTheme("unknown");
        }
        case KArchiveType::File: {
            const KMimeType::Ptr mime = KMimeType::mimeType(mimetype);
            if (mime) {
                return QIcon(KIconLoader::global()->loadMimeTypeIcon(mime->iconName(), KIconLoader::Small));
            }
            return QIcon::fromTheme("unknown");
        }
        case KArchiveType::Directory: {
            return QIcon::fromTheme("folder");
        }
        case KArchiveType::Link: {
            return QIcon::fromTheme("emblem-symbolic-link");
        }
        case KArchiveType::Character:
        case KArchiveType::Block: {
            return QIcon::fromTheme("drive-harddisk");
        }
        case KArchiveType::Fifo:
        case KArchiveType::Socket: {
            return QIcon::fromTheme("media-memory");
        }
    }

    Q_UNREACHABLE();
    return QIcon();
}

bool KArchiveInfo::isNull() const {
    if (type == KArchiveType::None) {
        return true;
    }
    return false;
}

bool KArchiveInfo::operator==(const KArchiveInfo &info) const {
    return pathname == info.pathname;
}

KArchiveInfo& KArchiveInfo::operator=(const KArchiveInfo &info) {
    encrypted = info.encrypted;
    size = info.size;
    gid = info.gid;
    uid = info.uid;
    mode = info.mode;
    atime = info.atime;
    ctime = info.ctime;
    mtime = info.mtime;
    hardlink = info.hardlink;
    symlink = info.symlink;
    pathname = info.pathname;
    mimetype = info.mimetype;
    type = info.type;
    return *this;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const KArchiveInfo &info)
{
    d << "KArchiveInfo( encrypted:" << info.fancyEncrypted()
        << ", size:" << info.fancySize()
        << ", gid:" << info.gid
        << ", uid:" << info.uid
        << ", mode:" << info.fancyMode()
        << ", atime:" << info.atime
        << ", ctime:" << info.ctime
        << ", mtime:" << info.mtime
        << ", hardlink:" << info.hardlink
        << ", symlink:" << info.symlink
        << ", pathname:" << info.pathname
        << ", mimetype:" << info.mimetype
        << ", type:" << info.fancyType()
        << ")";
    return d;
}
#endif

const QDBusArgument &operator<<(QDBusArgument &argument, const KArchiveInfo &info) {
    argument.beginStructure();
    argument << info.encrypted;
    argument << qlonglong(info.size);
    argument << qlonglong(info.gid);
    argument << qlonglong(info.uid);
    argument << info.mode;
    argument << uint(info.atime);
    argument << uint(info.ctime);
    argument << uint(info.mtime);
    argument << info.hardlink;
    argument << info.symlink;
    argument << info.pathname;
    argument << info.mimetype;
    argument << int(info.type);
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, KArchiveInfo &info) {
    int typebuf;
    uint atimebuff;
    uint ctimebuff;
    uint mtimebuff;
    qlonglong sizebuff;
    qlonglong gidbuff;
    qlonglong uidbuff;
    argument.beginStructure();
    argument >> info.encrypted;
    argument >> sizebuff;
    info.size = int64_t(sizebuff);
    argument >> gidbuff;
    info.gid = int64_t(gidbuff);
    argument >> uidbuff;
    info.uid = int64_t(uidbuff);
    argument >> info.mode;
    argument >> atimebuff;
    info.atime = time_t(atimebuff);
    argument >> ctimebuff;
    info.ctime = time_t(ctimebuff);
    argument >> mtimebuff;
    info.mtime = time_t(mtimebuff);
    argument >> info.hardlink;
    argument >> info.symlink;
    argument >> info.pathname;
    argument >> info.mimetype;
    argument >> typebuf;
    info.type = KArchiveInfo::KArchiveType(typebuf);
    argument.endStructure();

    return argument;
}

class KArchiveManagerPrivate {

    public:
        QString m_path;
        bool m_writable;

        static struct archive* openRead(const QByteArray &path);
        static struct archive* openWrite(const QByteArray &path);
        static struct archive* openDisk(const bool preserve);
        static bool closeRead(struct archive*);
        static bool closeWrite(struct archive*);

        static bool copyData(struct archive* aread, struct archive* awrite);
        static bool copyData(struct archive* aread, QByteArray &awrite);

        static QByteArray getMime(struct archive* aread);
        static KArchiveInfo::KArchiveType getType(const mode_t mode);
};

struct archive* KArchiveManagerPrivate::openRead(const QByteArray &path) {
    struct archive* m_archive = archive_read_new();

    if (m_archive) {
        archive_read_support_filter_all(m_archive);
        archive_read_support_format_all(m_archive);

        if (archive_read_open_filename(m_archive, path, KARCHIVEMANAGER_BUFFSIZE) != ARCHIVE_OK) {
            kWarning() << "archive_read_open_filename" << archive_error_string(m_archive);
        }
    }

    return m_archive;
}

struct archive* KArchiveManagerPrivate::openWrite(const QByteArray &path) {
    struct archive* m_archive = archive_write_new();

    if (m_archive) {
        if (archive_write_set_format_filter_by_ext(m_archive, path) != ARCHIVE_OK) {
            kWarning() << "archive_write_set_format_filter_by_ext" << archive_error_string(m_archive);
            archive_write_add_filter_none(m_archive);
        }

        archive_write_set_format_pax_restricted(m_archive);

        if (archive_write_open_filename(m_archive, path) != ARCHIVE_OK) {
            kWarning() << "archive_write_open_filename" << archive_error_string(m_archive);
        }
    }

    return m_archive;
}

struct archive* KArchiveManagerPrivate::openDisk(const bool preserve) {
    struct archive* m_archive = archive_write_disk_new();

    if (m_archive) {
        int extractFlags = ARCHIVE_EXTRACT_TIME;
        extractFlags |= ARCHIVE_EXTRACT_SECURE_SYMLINKS | ARCHIVE_EXTRACT_SECURE_NODOTDOT;
        if (preserve) {
            extractFlags |= ARCHIVE_EXTRACT_PERM;
            extractFlags |= ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_XATTR;
            extractFlags |= ARCHIVE_EXTRACT_FFLAGS | ARCHIVE_EXTRACT_MAC_METADATA;
        }

        archive_write_disk_set_options(m_archive, extractFlags);
        archive_write_disk_set_standard_lookup(m_archive);
    }

    return m_archive;
}

bool KArchiveManagerPrivate::closeRead(struct archive* m_archive) {
    if (m_archive) {
        if (archive_read_close(m_archive) != ARCHIVE_OK) {
            kWarning() << "archive_read_close" << archive_error_string(m_archive);
            return false;
        }

        if (archive_read_free(m_archive) != ARCHIVE_OK) {
            kWarning() << "archive_read_free" << archive_error_string(m_archive);
            return false;
        }
    }

    return true;
}

bool KArchiveManagerPrivate::closeWrite(struct archive* m_archive) {
    if (m_archive) {
        if (archive_write_close(m_archive) != ARCHIVE_OK) {
            kWarning() << "archive_write_close" << archive_error_string(m_archive);
            return false;
        }

        if (archive_write_free(m_archive) != ARCHIVE_OK) {
            kWarning() << "archive_write_free" << archive_error_string(m_archive);
            return false;
        }
    }

    return true;
}

bool KArchiveManagerPrivate::copyData(struct archive* aread, struct archive* awrite) {
    char buffer[KARCHIVEMANAGER_BUFFSIZE];
    ssize_t readsize = archive_read_data(aread, buffer, sizeof(buffer));
    while (readsize > 0) {
        const int result = archive_errno(aread);
        if (result != ARCHIVE_OK) {
            kWarning() << "archive_read_data" << archive_error_string(aread);
            return false;
        }

        if (archive_write_data(awrite, buffer, readsize) != readsize) {
            kWarning() << "archive_write_data" << archive_error_string(awrite);
            return false;
        }

        readsize = archive_read_data(aread, buffer, sizeof(buffer));
    }

    return true;
}


bool KArchiveManagerPrivate::copyData(struct archive* aread, QByteArray &awrite) {
    char buffer[KARCHIVEMANAGER_BUFFSIZE];
    ssize_t readsize = archive_read_data(aread, buffer, sizeof(buffer));
    while (readsize > 0) {
        const int result = archive_errno(aread);
        if (result != ARCHIVE_OK) {
            kWarning() << "archive_read_data" << archive_error_string(aread);
            return false;
        }

        awrite.append(buffer, readsize);

        readsize = archive_read_data(aread, buffer, sizeof(buffer));
    }

    return true;
}

QByteArray KArchiveManagerPrivate::getMime(struct archive* aread) {
    QByteArray buffer;
    copyData(aread, buffer);
    const KMimeType::Ptr mime = KMimeType::findByContent(buffer);
    if (mime) {
        return mime->name().toUtf8();
    }
    return QByteArray("application/octet-stream");
}

KArchiveInfo::KArchiveType KArchiveManagerPrivate::getType(const mode_t mode) {
    if (S_ISREG(mode)) {
        return KArchiveInfo::KArchiveType::File;
    } else if (S_ISDIR(mode)) {
        return KArchiveInfo::KArchiveType::Directory;
    } else if (S_ISLNK(mode)) {
        return KArchiveInfo::KArchiveType::Link;
    } else if (S_ISCHR(mode)) {
        return KArchiveInfo::KArchiveType::Character;
    } else if (S_ISBLK(mode)) {
        return KArchiveInfo::KArchiveType::Block;
    } else if (S_ISFIFO(mode)) {
        return KArchiveInfo::KArchiveType::Fifo;
    } else if (S_ISSOCK(mode)) {
        return KArchiveInfo::KArchiveType::Socket;
    } else {
        return KArchiveInfo::KArchiveType::None;
    }
}

KArchiveManager::KArchiveManager(const QString &path)
    : d(new KArchiveManagerPrivate()) {
    qRegisterMetaType<KArchiveInfo>();
    qDBusRegisterMetaType<KArchiveInfo>();

    d->m_path = path;

    if (path.isEmpty()) {
        kWarning() << "empty path";
        return;
    }

    if (QFile::exists(path) && !supported(path)) {
        kWarning() << "unsupported" << path;
        d->m_writable = false;
        return;
    }

    const KMimeType::Ptr mime = KMimeType::findByPath(path);
    if (mime) {
        foreach (const QString &parentmime, mime->allParentMimeTypes()) {
            if (s_readwrite.contains(parentmime)) {
                d->m_writable = true;
                break;
            }
        }
    } else {
        d->m_writable = false;
    }
}

KArchiveManager::~KArchiveManager() {
    if (d) {
        delete d;
    }
}

bool KArchiveManager::add(const QStringList &paths, const QByteArray &strip, const QByteArray &destination) const {
    bool result = false;

    if (d->m_path.isEmpty()) {
        kWarning() << "no path is set";
        return result;
    } else if (!d->m_writable) {
        kWarning() << "path is not writable";
        return result;
    }

    QFileInfo fileinfo(d->m_path);
    QTemporaryFile tmpfile("XXXXXX." + fileinfo.completeSuffix());
    if (!tmpfile.open()) {
        kWarning() << "could not open temporary file";
        return result;
    }

    const QByteArray tmppath = tmpfile.fileName().toUtf8();
    struct archive* m_write = d->openWrite(tmppath);
    if (!m_write) {
        kWarning() << "could not open temporary archive" << tmppath;
        return result;
    }

    QStringList recursivepaths;
    foreach (const QString &path, paths) {
        if (QDir(path).exists()) {
            QDirIterator iterator(path, QDir::AllEntries | QDir::System, QDirIterator::Subdirectories);
            while (iterator.hasNext()) {
                recursivepaths << QDir::cleanPath(iterator.next());
            }
        } else {
            recursivepaths << QDir::cleanPath(path);
        }
    }
    recursivepaths.removeDuplicates();

    bool replace = false;
    if (QFile::exists(d->m_path)) {

        replace = true;

        struct archive* m_read = d->openRead(d->m_path.toUtf8());
        if (!m_read) {
            kWarning() << "could not open archive" << d->m_path;
            return result;
        }

        struct archive_entry* entry = archive_entry_new();
        int ret = archive_read_next_header(m_read, &entry);
        while (ret != ARCHIVE_EOF) {
            if (ret < ARCHIVE_OK) {
                kWarning() << "archive_read_next_header" << archive_error_string(m_read);
                result = false;
                break;
            }

            const QByteArray pathname = archive_entry_pathname(entry);
            if (recursivepaths.contains(strip + pathname)) {
                kDebug() << "removing (update)" << pathname;
                archive_read_data_skip(m_read);
                ret = archive_read_next_header(m_read, &entry);
                continue;
            }

            if (archive_write_header(m_write, entry) != ARCHIVE_OK) {
                kWarning() << "archive_write_header" << archive_error_string(m_write);
                result = false;
                break;
            }

            if (!d->copyData(m_read, m_write)) {
                result = false;
                break;
            }

            if (archive_write_finish_entry(m_write) != ARCHIVE_OK) {
                kWarning() << "archive_write_finish_entry" << archive_error_string(m_write);
                result = false;
                break;
            }

            ret = archive_read_next_header(m_read, &entry);
        }

        d->closeRead(m_read);
    }

    foreach (const QString &path, recursivepaths) {
        const QByteArray utfpath = path.toUtf8();

        struct stat statistic;
        if (::lstat(utfpath, &statistic) != 0) {
            kWarning() << "stat error" << ::strerror(errno);
            result = false;
            break;
        }

        QByteArray pathname = utfpath;
        if (pathname.startsWith(strip)) {
            pathname.remove(0, strip.size());
        }
        pathname.prepend(destination);
        kDebug() << "adding" << path << "as" << pathname;

        // NOTE: archive_entry_copy_stat doesn't work
        // http://linux.die.net/man/2/stat
        struct archive_entry* newentry = archive_entry_new();
        archive_entry_set_pathname(newentry, pathname);
        archive_entry_set_size(newentry, statistic.st_size);
        archive_entry_set_gid(newentry, statistic.st_gid);
        archive_entry_set_uid(newentry, statistic.st_uid);
        archive_entry_set_atime(newentry, statistic.st_atim.tv_sec, statistic.st_atim.tv_nsec);
        archive_entry_set_ctime(newentry, statistic.st_ctim.tv_sec, statistic.st_ctim.tv_nsec);
        archive_entry_set_mtime(newentry, statistic.st_mtim.tv_sec, statistic.st_mtim.tv_nsec);

        // filetype and mode are supposedly the same
        // permissions are set when mode is set, same for filetyp?
        archive_entry_set_mode(newentry, statistic.st_mode);

        if (statistic.st_nlink > 1) {
            // TODO: archive_entry_set_hardlink(newentry, pathname);
        }

        if (S_ISLNK(statistic.st_mode)) {
            QByteArray linkbuffer(PATH_MAX + 1, Qt::Uninitialized);
            if (::readlink(utfpath, linkbuffer.data(), PATH_MAX) == -1) {
                kWarning() << "readlink error" << ::strerror(errno);
                result = false;
                break;
            }

            if (linkbuffer.startsWith(strip)) {
                linkbuffer.remove(0, strip.size());
            }

            archive_entry_set_symlink(newentry, linkbuffer.constData());
        }

        if (archive_write_header(m_write, newentry) != ARCHIVE_OK) {
            kWarning() << "archive_write_header" << archive_error_string(m_write);
            archive_entry_free(newentry);
            result = false;
            break;
        }
        archive_entry_free(newentry);

        if (S_ISREG(statistic.st_mode)) {
            QFile file(path);
            if (!file.open(QFile::ReadOnly)) {
                kWarning() << "could not open" << path;
                result = false;
                break;
            }

            const QByteArray data = file.readAll();
            if (data.isEmpty() && statistic.st_size > 0) {
                kWarning() << "could not read" << path;
                result = false;
                break;
            }

            if (statistic.st_size > 0 && data.size() != statistic.st_size) {
                kWarning() << "read and stat size are different" << path;
                result = false;
                break;
            }

            if (archive_write_data(m_write, data, data.size()) != statistic.st_size) {
                kWarning() << "archive_write_data" << archive_error_string(m_write);
                result = false;
                break;
            }
        }

        if (archive_write_finish_entry(m_write) != ARCHIVE_OK) {
            kWarning() << "archive_write_finish_entry" << archive_error_string(m_write);
            result = false;
            break;
        }

        result = true;
    }

    d->closeWrite(m_write);

    if (result && replace) {
        result = QFile::remove(d->m_path);
        if (!result) {
            kWarning() << "could not remove original" << d->m_path;
        }
    }

    if (result) {
        result = QFile::rename(tmppath, d->m_path);
        if (!result) {
            kWarning() << "could not move" << tmppath << "to" << d->m_path;
        }
    }

    return result;
}

bool KArchiveManager::remove(const QStringList &paths) const {
    bool result = false;

    if (d->m_path.isEmpty()) {
        kWarning() << "no path is set";
        return result;
    } else if (!d->m_writable) {
        kWarning() << "path is not writable";
        return result;
    } else if (!QFile::exists(d->m_path)) {
        kWarning() << "path does not exists" << d->m_path;
        return result;
    }

    struct archive* m_read = d->openRead(d->m_path.toUtf8());
    if (!m_read) {
        kWarning() << "could not open archive" << d->m_path;
        return result;
    }

    QFileInfo fileinfo(d->m_path);
    QTemporaryFile tmpfile("XXXXXX." + fileinfo.completeSuffix());
    if (!tmpfile.open()) {
        kWarning() << "could not open temporary file";
        return result;
    }

    const QByteArray tmppath = tmpfile.fileName().toUtf8();
    struct archive* m_write = d->openWrite(tmppath);
    if (!m_write) {
        kWarning() << "could not open temporary archive" << tmppath;
        return result;
    }

    QStringList notfound = paths;

    struct archive_entry* entry = archive_entry_new();
    int ret = archive_read_next_header(m_read, &entry);
    while (ret != ARCHIVE_EOF) {
        if (ret < ARCHIVE_OK) {
            kWarning() << "archive_read_next_header" << archive_error_string(m_read);
            result = false;
            break;
        }

        const QByteArray pathname = archive_entry_pathname(entry);
        if (paths.contains(pathname)) {
            kDebug() << "removing" << pathname;
            notfound.removeAll(pathname);
            archive_read_data_skip(m_read);
            ret = archive_read_next_header(m_read, &entry);
            result = true;
            continue;
        }

        if (archive_write_header(m_write, entry) != ARCHIVE_OK) {
            kWarning() << "archive_write_header" << archive_error_string(m_write);
            result = false;
            break;
        }

        if (!d->copyData(m_read, m_write)) {
            result = false;
            break;
        }

        if (archive_write_finish_entry(m_write) != ARCHIVE_OK) {
            kWarning() << "archive_write_finish_entry" << archive_error_string(m_write);
            result = false;
            break;
        }

        ret = archive_read_next_header(m_read, &entry);
    }

    d->closeWrite(m_write);
    d->closeRead(m_read);

    if (result) {
        kDebug() << "replacing" << d->m_path << "with" << tmppath;

        result = QFile::remove(d->m_path);
        if (result) {
            result = QFile::rename(tmppath, d->m_path);
            if (!result) {
                kWarning() << "could not move" << tmppath << "to" << d->m_path;
            }
        } else {
            kWarning() << "could not remove original" << d->m_path;
        }
    }

    if (!notfound.isEmpty()) {
        kWarning() << "entries not in archive" << notfound;
    }

    return result;
}

bool KArchiveManager::extract(const QStringList &paths, const QString &destination, bool preserve) const {
    bool result = false;

    if (d->m_path.isEmpty()) {
        kWarning() << "no path is set";
        return result;
    } else if (!QFile::exists(d->m_path)) {
        kWarning() << "path does not exists" << d->m_path;
        return result;
    }

    struct archive* m_read = d->openRead(d->m_path.toUtf8());
    if (!m_read) {
        kWarning() << "could not open archive" << d->m_path;
        return result;
    }

    const QString currendir = QDir::currentPath();
    if (!QDir::setCurrent(destination)) {
        kWarning() << "could not change to destination directory" << destination;
        return result;
    }

    struct archive* m_write = d->openDisk(preserve);
    if (!m_write) {
        kWarning() << "could not open write archive";
        return result;
    }

    QStringList notfound = paths;

    struct archive_entry* entry = archive_entry_new();
    int ret = archive_read_next_header(m_read, &entry);
    while (ret != ARCHIVE_EOF) {
        if (ret < ARCHIVE_OK) {
            kWarning() << "archive_read_next_header" << archive_error_string(m_read);
            result = false;
            break;
        }

        const QByteArray pathname = archive_entry_pathname(entry);;
        if (!paths.contains(pathname)) {
            archive_read_data_skip(m_read);
            ret = archive_read_next_header(m_read, &entry);
            continue;
        }
        notfound.removeAll(pathname);
        result = true;

        if (archive_write_header(m_write, entry) != ARCHIVE_OK) {
            kWarning() << "archive_write_header" << archive_error_string(m_write);
            result = false;
            break;
        }

        if (archive_read_extract2(m_read, entry, m_write) != ARCHIVE_OK) {
            kWarning() << "archive_read_extract2" << archive_error_string(m_write);
            result = false;
            break;
        }

        if (archive_write_finish_entry(m_write) != ARCHIVE_OK) {
            kWarning() << "archive_write_finish_entry" << archive_error_string(m_write);
            result = false;
            break;
        }

        ret = archive_read_next_header(m_read, &entry);
    }

    d->closeWrite(m_write);
    d->closeRead(m_read);

    if (!QDir::setCurrent(currendir)) {
        kWarning() << "could not change to orignal directory" << currendir;
    }

    if (!notfound.isEmpty()) {
        kWarning() << "entries not in archive" << notfound;
    }

    return result;
}

QList<KArchiveInfo> KArchiveManager::list() const {
    QList<KArchiveInfo> result;

    if (d->m_path.isEmpty()) {
        kWarning() << "no path is set";
        return result;
    } else if (!QFile::exists(d->m_path)) {
        kWarning() << "path does not exists" << d->m_path;
        return result;
    }

    struct archive* m_read = d->openRead(d->m_path.toUtf8());
    if (!m_read) {
        kWarning() << "could not open archive" << d->m_path;
        return result;
    }

    struct archive_entry* entry = archive_entry_new();
    int ret = archive_read_next_header(m_read, &entry);
    while (ret != ARCHIVE_EOF) {
        if (ret < ARCHIVE_OK) {
            kWarning() << "archive_read_next_header" << archive_error_string(m_read);
            result.clear();
            break;
        }

        KArchiveInfo info;
        info.encrypted = bool(archive_entry_is_encrypted(entry));
        info.size = archive_entry_size(entry);
        info.gid = archive_entry_gid(entry);
        info.uid = archive_entry_uid(entry);
        info.mode = archive_entry_mode(entry);
        info.atime = archive_entry_atime(entry);
        info.ctime = archive_entry_ctime(entry);
        info.mtime = archive_entry_mtime(entry);
        info.hardlink =  QByteArray(archive_entry_hardlink(entry));
        info.symlink = QByteArray(archive_entry_symlink(entry));
        info.pathname = archive_entry_pathname(entry);
        info.mimetype = d->getMime(m_read);
        info.type = d->getType(info.mode);

        result << info;

        ret = archive_read_next_header(m_read, &entry);
    }

    d->closeRead(m_read);

    return result;
}

KArchiveInfo KArchiveManager::info(const QString &path) const {
    KArchiveInfo result;

    if (d->m_path.isEmpty()) {
        kWarning() << "no path is set";
        return result;
    }

    struct archive* m_read = d->openRead(d->m_path.toUtf8());
    if (!m_read) {
        kWarning() << "could not open archive" << d->m_path;
        return result;
    }

    bool found = false;
    struct archive_entry* entry = archive_entry_new();
    int ret = archive_read_next_header(m_read, &entry);
    while (ret != ARCHIVE_EOF) {
        if (ret < ARCHIVE_OK) {
            kWarning() << "archive_read_next_header" << archive_error_string(m_read);
            break;
        }

        const QByteArray pathname = archive_entry_pathname(entry);
        if (pathname == path) {
            result.encrypted = bool(archive_entry_is_encrypted(entry));
            result.size = archive_entry_size(entry);
            result.gid = archive_entry_gid(entry);
            result.uid = archive_entry_uid(entry);
            result.mode = archive_entry_mode(entry);
            result.atime = archive_entry_atime(entry);
            result.ctime = archive_entry_ctime(entry);
            result.mtime = archive_entry_mtime(entry);
            result.hardlink =  QByteArray(archive_entry_hardlink(entry));
            result.symlink = QByteArray(archive_entry_symlink(entry));
            result.pathname = pathname;
            result.mimetype = d->getMime(m_read);
            result.type = d->getType(result.mode);

            found = true;
            break;
        }

        ret = archive_read_next_header(m_read, &entry);
    }

    d->closeRead(m_read);

    if (!found) {
        kWarning() << "entry not in archive" << path;
    }

    return result;
}

bool KArchiveManager::writable() const {
    return d->m_writable;
}

bool KArchiveManager::supported(const QString &path) {
    bool result = false;

    struct archive* m_read = KArchiveManagerPrivate::openRead(path.toUtf8());
    if (m_read) {
        result = true;
    }

    KArchiveManagerPrivate::closeRead(m_read);

    return result;
}

bool KArchiveManager::gzipRead(const QString &path, QByteArray &buffer) {
    bool result = true;

    gzFile gzip = gzopen(path.toUtf8(), "r");

    if (gzip) {
        buffer.clear();
        char gzbuffer[KARCHIVEMANAGER_BUFFSIZE];
        int readbytes = gzread(gzip, gzbuffer, sizeof(gzbuffer));
        while (readbytes > 0) {
            buffer += gzbuffer;

            readbytes = gzread(gzip, gzbuffer, sizeof(gzbuffer));
        }

        if (readbytes == -1) {
            kWarning() << "gzip read error" << path;
            buffer.clear();
            result = false;
        }
    } else {
        kWarning() << "could not open gzip" << path;
        result = false;
    }

    gzclose(gzip);

    return result;
}

bool KArchiveManager::gzipWrite(const QString &path, const QByteArray &data) {
    bool result = true;

    gzFile gzip = gzopen(path.toUtf8(), "w");

    if (gzip) {
        const int writebytes = gzwrite(gzip, data.constData(), data.size());
        if (writebytes != data.size()) {
            kWarning() << "gzip write error" << path;
            result = false;
        }
    } else {
        kWarning() << "could not open gzip" << path;
        result = false;
    }

    gzclose(gzip);

    return result;
}

bool KArchiveManager::bzipRead(const QString &path, QByteArray &buffer) {
    bool result = true;

    BZFILE* bzip = BZ2_bzopen(path.toUtf8(), "r");

    if (bzip) {
        buffer.clear();
        char bzbuffer[KARCHIVEMANAGER_BUFFSIZE];
        int readbytes = BZ2_bzread(bzip, bzbuffer, sizeof(bzbuffer));
        while (readbytes > 0) {
            buffer += bzbuffer;

            readbytes = BZ2_bzread(bzip, bzbuffer, sizeof(bzbuffer));
        }

        if (readbytes == -1) {
            kWarning() << "bzip read error" << path;
            buffer.clear();
            result = false;
        }
    } else {
        kWarning() << "could not open bzip" << path;
        result = false;
    }

    BZ2_bzclose(bzip);

    return result;
}

bool KArchiveManager::bzipWrite(const QString &path, QByteArray data) {
    bool result = true;

    BZFILE* bzip = BZ2_bzopen(path.toUtf8(), "w");

    if (bzip) {
        const int writebytes = BZ2_bzwrite(bzip, data.data(), data.size());
        if (writebytes != data.size()) {
            kWarning() << "bzip write error" << path;
            result = false;
        }
    } else {
        kWarning() << "could not open bzip" << path;
        result = false;
    }

        BZ2_bzclose(bzip);

    return result;
}

class KArchiveModelPrivate : public QThread {
    Q_OBJECT

    public:
        KArchiveModelPrivate(QObject *parent = Q_NULLPTR);

        QString joinDir(const QString &dir1, const QString &dir2) const;
        QStandardItem* makeRow(const QString &string) const;

        void appendDirectory(const QString &path);
        void appendSpecial(const KArchiveInfo &info);

        QStandardItem* m_root;
        QMap<QString,QStandardItem*> m_directories;
        QList<KArchiveInfo> m_list;
        const KArchiveManager *m_archive;
        bool m_interrupt;

        void requestInterruption();

    protected:
        virtual void run();
};

KArchiveModelPrivate::KArchiveModelPrivate(QObject *parent)
    : QThread(parent),
    m_interrupt(false) {
}

// because altering paths is not easy
QString KArchiveModelPrivate::joinDir(const QString &dir1, const QString &dir2) const {
    if (dir1.isEmpty()) {
        return dir2;
    }
    return dir1 + "/" + dir2;
}

QStandardItem* KArchiveModelPrivate::makeRow(const QString &string) const {
    QStandardItem* item = new QStandardItem(string);
    item->setSelectable(false);
    item->setTextAlignment(Qt::AlignRight);
    return item;
}

void KArchiveModelPrivate::appendDirectory(const QString &path) {
    QStandardItem* lastitem = m_root;
    QString dirsofar;
    foreach (const QString &dir, path.split("/")) {
        if (dir.isEmpty()) {
            continue;
        }

        dirsofar = joinDir(dirsofar, dir);
        if (m_directories.contains(dirsofar)) {
            lastitem = m_directories.value(dirsofar);
            continue;
        }

        // NOTE: directory entries are not consistent
        KArchiveInfo info;
        foreach (const KArchiveInfo &listinfo, m_list) {
            if (listinfo.pathname == dirsofar || listinfo.pathname == dirsofar + "/") {
                info = listinfo;
                break;
            }
        }
        if (info.isNull()) {
            // fake it for now
            info.type = KArchiveInfo::Directory;
        }

        QList<QStandardItem*> diritems;
        QStandardItem* diritem = new QStandardItem(dir);
        diritem->setIcon(info.fancyIcon());
        diritem->setWhatsThis("Directory");
        diritem->setStatusTip(dirsofar);
        diritems << diritem;
        diritems << makeRow(info.fancyType());
        diritems << makeRow(info.fancySize());
        diritems << makeRow(info.fancyMode());
        diritems << makeRow(info.fancyEncrypted());

        lastitem->appendRow(diritems);

        m_directories.insert(dirsofar, diritem);
        lastitem = diritem;
    }
}

void KArchiveModelPrivate::appendSpecial(const KArchiveInfo &info) {
    const QFileInfo fileinfo(info.pathname);
    const QString infopath = fileinfo.path();
    const QString infoname = fileinfo.fileName();

    appendDirectory(infopath);

    QList<QStandardItem*> specialitems;
    QStandardItem* specialitem = new QStandardItem(infoname);
    specialitem->setIcon(info.fancyIcon());
    specialitem->setStatusTip(info.pathname);
    specialitems << specialitem;
    specialitems << makeRow(info.fancyType());
    specialitems << makeRow(info.fancySize());
    specialitems << makeRow(info.fancyMode());
    specialitems << makeRow(info.fancyEncrypted());

    QStandardItem* diritem = m_directories.value(infopath);
    if (diritem) {
        diritem->appendRow(specialitems);
    } else {
        m_root->appendRow(specialitems);
    }
}

void KArchiveModelPrivate::run() {
    m_list = m_archive->list();
    m_interrupt = false;

    foreach (const KArchiveInfo &info, m_list) {
        if(m_interrupt) {
            return;
        }

        if (info.type == KArchiveInfo::KArchiveType::Directory) {
            appendDirectory(info.pathname);
        } else {
            appendSpecial(info);
        }
    }
}

void KArchiveModelPrivate::requestInterruption() {
    m_interrupt = true;
}

KArchiveModel::KArchiveModel(QObject *parent)
    : QStandardItemModel(parent),
    d(new KArchiveModelPrivate(this)) {
    connect(d, SIGNAL(finished()), this, SLOT(slotLoadFinished()));
}

KArchiveModel::~KArchiveModel() {
    if (d) {
        delete d;
    }
}

bool KArchiveModel::loadArchive(const KArchiveManager *archive) {
    bool result = false;

    if (!archive) {
        return result;
    }

    result = true;

    if (d->isRunning()) {
        d->requestInterruption();
        d->wait();
    }
    QCoreApplication::processEvents();
    clear();

    d->m_archive = archive;
    d->m_root = invisibleRootItem();
    d->m_directories.clear();

    emit loadStarted();
    d->start();

    return result;
}

QString KArchiveModel::path(const QModelIndex &index) const {
    QString result;

    if (!index.isValid()) {
        return result;
    }

    const QStandardItem *item = itemFromIndex(index);
    result = item->statusTip();

    if (item->whatsThis() == ("Directory")) {
        result += "/";
    }

    return result;
}

QStringList KArchiveModel::paths(const QModelIndex &index) const {
    QStringList result;

    const QByteArray indexpath = path(index).toUtf8();
    if (!indexpath.isEmpty() && index.isValid()) {
        const QStandardItem *item = itemFromIndex(index);
        if (item->whatsThis() == ("Directory")) {
            foreach (const KArchiveInfo &info, d->m_list) {
                if (info.pathname.startsWith(indexpath)) {
                    result << info.pathname;
                }
            }
        } else {
            result << indexpath;
        }
    }

    return result;
}

QString KArchiveModel::dir(const QModelIndex &index) const {
    QString result;

    if (!index.isValid()) {
        return result;
    }

    const QStandardItem *item = itemFromIndex(index);
    if (item->whatsThis() == ("Directory")) {
        result = item->statusTip();
    } else {
        QFileInfo iteminfo(item->statusTip());
        result = iteminfo.path();
    }

    return result;
}

void KArchiveModel::slotLoadFinished() {
    setHeaderData(0, Qt::Horizontal, QVariant("Name"));
    setHeaderData(1, Qt::Horizontal, QVariant("Type"));
    setHeaderData(2, Qt::Horizontal, QVariant("Size"));
    setHeaderData(3, Qt::Horizontal, QVariant("Mode"));
    setHeaderData(4, Qt::Horizontal, QVariant("Encrypted"));

    emit loadFinished();
}

#include "karchivemanager.moc"
