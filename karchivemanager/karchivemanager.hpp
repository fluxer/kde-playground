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

#ifndef KARCHIVEMANAGER_H
#define KARCHIVEMANAGER_H

#include <QString>
#include <QStringList>
#include <QStandardItemModel>
#include <QDBusArgument>

#include <sys/types.h>

/*!
    Archive entry information holder, valid object is obtained via @p KArchiveManager::info

    @note It is up to the programmer to keep the integrity of the structure
    @note D-Bus signature for the type is <b>(bxxxussssi)</b>
    @warning The structure is not very portable (size, gid, uid)
    @ingroup Types

    @see KArchiveManager
    @see https://dbus.freedesktop.org/doc/dbus-specification.html#container-types
*/
class KArchiveInfo {

    public:
        enum KArchiveType {
            None = 0,
            File = 1,
            Directory = 2,
            Link = 3,
            Character = 4,
            Block = 5,
            Fifo = 6,
            Socket = 7,
        };

        KArchiveInfo();
        KArchiveInfo(const KArchiveInfo &info);

        bool encrypted;
        int64_t size;
        int64_t gid;
        int64_t uid;
        mode_t mode;
        QByteArray hardlink;
        QByteArray symlink;
        QByteArray pathname;
        QByteArray mimetype;
        KArchiveType type;

        //! @brief Fancy encrypted for the purpose of widgets
        QString fancyEncrypted() const;
        //! @brief Fancy size for the purpose of widgets
        QString fancySize() const;
        //! @brief Fancy mode for the purpose of widgets
        QString fancyMode() const;
        //! @brief Fancy type for the purpose of widgets
        QString fancyType() const;
        //! @brief Fancy icon for the purpose of widgets
        QIcon fancyIcon() const;

        //! @brief Returns if the info is valid or not
        bool isNull() const;

        bool operator==(const KArchiveInfo &i) const;
        KArchiveInfo &operator=(const KArchiveInfo &i);
};
#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug, const KArchiveInfo &i);
#endif
const QDBusArgument &operator<<(QDBusArgument &, const KArchiveInfo &i);
const QDBusArgument &operator>>(const QDBusArgument &, KArchiveInfo &i);

class KArchiveManagerPrivate;

/*!
    Archive manager with support for many formats

    Example:
    \code
    KArchiveManager archive("/home/joe/archive.tar.gz");
    kDebug() << archive.list();

    QDir::mkpath("/tmp/destination");
    archive.extract("dir/in/archive/", "/tmp/destination");
    archive.delete("file/in/archive.txt");
    \endcode

    @note Paths ending with "/" will be considered as directories
    @warning The operations are done on temporary file, copy of the orignal, which after
    successfull operation (add or remove) replaces the orignal thus if it is interrupted the
    source may get corrupted

    @see KArchiveInfo

    @todo encrypted paths handling
    @todo make listing consistent by faking dir info?
    @todo set permissions on file after copy, same as the original
    @todo error reporting
*/
class KArchiveManager {

    public:
        KArchiveManager(const QString &path);
        ~KArchiveManager();

        /*!
            @brief Add paths to the archive
            @param strip string to remove from the start of every path
            @param destination relative path where paths should be added to
        */
        bool add(const QStringList &paths, const QByteArray &strip = "/", const QByteArray &destination = "") const;
        //! @brief Remove paths from the archive
        bool remove(const QStringList &paths) const;
        /*!
            @brief Extract paths to destination
            @param destination existing directory, you can use @p QDir::mkpath(QString)
            @param preserve preserve advanced attributes (ACL/ATTR)
        */
        bool extract(const QStringList &paths, const QString &destination, bool preserve = true) const;

        /*!
            @brief List the content of the archive
            @note may return empty list on both failure and success
            @note some formats list directories, some do not
            @todo report failure somehow
        */
        QList<KArchiveInfo> list() const;

        //! @brief Get information for path in archive
        KArchiveInfo info(const QString &path) const;

        //! @brief Report if archive is writable
        bool writable() const;
        //! @brief Report if path is supported archive
        static bool supported(const QString &path);

    private:
        KArchiveManagerPrivate *d;
};

class KArchiveModelPrivate;

/*!
    Custom item model for displaying archives in tree views (@p QTreeView)

    Example:
    \code
    QTreeView view;
    KArchiveModel model;
    KArchiveManager archive("/home/joe/archive.tar.gz");

    view.setModel(model);
    model.loadArchive(&archive);
    \endcode

    @see KArchiveManager, KArchiveInfo

    @todo sorting by column values is borked
*/
class KArchiveModel : public QStandardItemModel {
    Q_OBJECT

    public:
        KArchiveModel(QObject *parent = Q_NULLPTR);
        ~KArchiveModel();

        //! @brief Load archive into the model
        bool loadArchive(const KArchiveManager *archive);
        //! @brief Get path for index, propagates to parents to retrieve the full path
        QString path(const QModelIndex &index) const;
        /*!
            @brief Get paths for index, propagates to childs to retrieve all sub-paths.
            Usefull for obtaining the selected item paths for add, remove and extract
            operations
        */
        QStringList paths(const QModelIndex &index) const;
        /*!
            @brief Get parent directory for index. Usefull for obtaining the current dir
            for add operations
        */
        QString dir(const QModelIndex &index) const;

    Q_SIGNALS:
        //! @brief Signals load was started
        void loadStarted();
        //! @brief Signals load was finished
        void loadFinished();

    private Q_SLOTS:
        void slotLoadFinished();

    private:
        KArchiveModelPrivate *d;
};

Q_DECLARE_METATYPE(KArchiveInfo);

#endif // KARCHIVEMANAGER_H
