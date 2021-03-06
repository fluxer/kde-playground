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

#ifndef IMPORTALARMJOB_H
#define IMPORTALARMJOB_H

#include "abstractimportexportjob.h"

class ArchiveStorage;
#include <QWidget>

class ImportAlarmJob : public AbstractImportExportJob
{
    Q_OBJECT
public:
    explicit ImportAlarmJob(QWidget *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep);
    ~ImportAlarmJob();

    void start();

protected:
    void nextStep();

private:
    void storeAlarmArchiveResource(const KArchiveDirectory *dir, const QString &prefix);
    void searchAllFiles(const KArchiveDirectory *dir,const QString &prefix);
    void importkalarmConfig(const KArchiveFile* kalarmFile, const QString &kalarmrc, const QString &filename,const QString &prefix);
    void restoreResources();
    void restoreConfig();
    void addSpecificResourceSettings(KSharedConfig::Ptr resourceConfig, const QString &resourceName, QMap<QString, QVariant> &settings);
};

#endif // IMPORTALARMJOB_H
