/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#ifndef BALOODEBUGDIALOG_H
#define BALOODEBUGDIALOG_H

#include <KDialog>
#include "pimcommon_export.h"
#include "baloodebugsearchpathcombobox.h"
#include <Akonadi/Item>

namespace PimCommon {
class BalooDebugWidget;
class PIMCOMMON_EXPORT BalooDebugDialog : public KDialog
{
    Q_OBJECT
public:
    explicit BalooDebugDialog(QWidget *parent=0);
    ~BalooDebugDialog();

    void setAkonadiId(Akonadi::Item::Id akonadiId);
    void setSearchType(BalooDebugSearchPathComboBox::SearchType type);
    void doSearch();
private:
    void readConfig();
    void writeConfig();
    BalooDebugWidget *mBalooDebugWidget;
};
}


#endif // BALOODEBUGDIALOG_H

