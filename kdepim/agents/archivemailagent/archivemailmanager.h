/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef ARCHIVEMAILMANAGER_H
#define ARCHIVEMAILMANAGER_H

#include <QObject>

#include <KSharedConfig>
#include <Akonadi/Collection>

class ArchiveMailKernel;
class ArchiveMailInfo;

class ArchiveMailManager : public QObject
{
    Q_OBJECT
public:
    explicit ArchiveMailManager(QObject *parent = 0);
    ~ArchiveMailManager();
    void removeCollection(const Akonadi::Collection &collection);
    void backupDone(ArchiveMailInfo *info);
    void pause();
    void resume();

    QString printArchiveListInfo();
    void collectionDoesntExist(ArchiveMailInfo *info);

    QString printCurrentListInfo();

    void archiveFolder(const QString &path, Akonadi::Collection::Id collectionId);

public Q_SLOTS:
    void load();
    void slotArchiveNow(ArchiveMailInfo *info);

Q_SIGNALS:
    void needUpdateConfigDialogBox();

private:
    QString infoToStr(ArchiveMailInfo *info) const;
    void removeCollectionId(Akonadi::Collection::Id id);
    KSharedConfig::Ptr mConfig;
    QList<ArchiveMailInfo *> mListArchiveInfo;
    ArchiveMailKernel *mArchiveMailKernel;
};


#endif /* ARCHIVEMAILMANAGER_H */

