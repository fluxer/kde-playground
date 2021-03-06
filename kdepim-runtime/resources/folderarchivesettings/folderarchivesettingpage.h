/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#ifndef FOLDERARCHIVESETTINGPAGE_H
#define FOLDERARCHIVESETTINGPAGE_H

#include "folderarchiveaccountinfo.h"
#include "folderarchivesettings_export.h"
#include <QWidget>
#include <QComboBox>

#include <QCheckBox>
namespace Akonadi {
class CollectionRequester;
}

class FolderArchiveComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit FolderArchiveComboBox(QWidget *parent = 0);
    ~FolderArchiveComboBox();

    void setType(FolderArchiveAccountInfo::FolderArchiveType type);
    FolderArchiveAccountInfo::FolderArchiveType type() const;

private:
    void initialize();
};

class FolderArchiveAccountInfo;
class FOLDERARCHIVESETTINGS_EXPORT FolderArchiveSettingPage : public QWidget
{
    Q_OBJECT
public:
    explicit FolderArchiveSettingPage(const QString &instanceName, QWidget *parent=0);
    ~FolderArchiveSettingPage();

    void loadSettings();
    void writeSettings();

private Q_SLOTS:
    void slotEnableChanged(bool enabled);

private:
    QString mInstanceName;
    QCheckBox *mEnabled;
    FolderArchiveComboBox *mArchiveNamed;
    Akonadi::CollectionRequester *mArchiveFolder;
    FolderArchiveAccountInfo *mInfo;
};

#endif // FOLDERARCHIVESETTINGPAGE_H
