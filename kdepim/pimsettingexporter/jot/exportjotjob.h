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

#ifndef EXPORTJOTJOB_H
#define EXPORTJOTJOB_H

#include "abstractimportexportjob.h"

#include <QWidget>
class ArchiveStorage;

class ExportJotJob : public AbstractImportExportJob
{
    Q_OBJECT
public:
    explicit ExportJotJob(QWidget *parent, Utils::StoredTypes typeSelected, ArchiveStorage *archiveStorage, int numberOfStep);
    ~ExportJotJob();

    void start();

private:
    void backupResources();
    void backupConfig();
};

#endif // EXPORTJOTJOB_H
