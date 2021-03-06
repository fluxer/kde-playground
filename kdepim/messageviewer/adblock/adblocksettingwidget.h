/* ============================================================
*
* This file is a part of the rekonq project
* Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>
* based on code from rekonq
* Copyright (C) 2010-2012 by Andrea Diamantini <adjam7 at gmail dot com>
*
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy
* defined in Section 14 of version 3 of the license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
* ============================================================ */


#ifndef AD_BLOCK_SETTINGS_WIDGET_H
#define AD_BLOCK_SETTINGS_WIDGET_H

#include "ui_settings_adblock.h"
#include "messageviewer_export.h"

// Qt Includes
#include <QWidget>

namespace MessageViewer {
class MESSAGEVIEWER_EXPORT AdBlockSettingWidget : public QWidget, private Ui::adblock
{
    Q_OBJECT

public:
    explicit AdBlockSettingWidget(QWidget *parent = 0);

    bool changed() const;

    void save();
    void doLoadFromGlobalSettings();
    void doResetToDefaultsOther();


Q_SIGNALS:
    void changed(bool);

private Q_SLOTS:
    void hasChanged();
    void slotInfoLinkActivated(const QString &);
    void insertRule();
    void removeRule();
    void slotAddFilter();
    void slotRemoveSubscription();
    void slotUpdateButtons();
    void slotShowList();
    void slotImportFilters();
    void slotExportFilters();
    void slotUpdateManualButtons();
    void slotEditFilter();
    void slotManualFilterLineEditTextChanged(const QString &);
    void slotAutomaticFilterDouble(QListWidgetItem *item);
    void slotDeleteList(const QString &listName);
private:
    void addManualFilter(const QString &text);
    void showAutomaticFilterList(QListWidgetItem *item);
    enum List {
        UrlList = Qt::UserRole + 1,
        PathList = Qt::UserRole + 2,
        LastUpdateList = Qt::UserRole + 3
    };

    bool mChanged;
};
}

#endif // AD_BLOCK_SETTINGS_WIDGET_H
