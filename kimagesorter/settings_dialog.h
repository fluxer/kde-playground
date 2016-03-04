/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2011, Dmitry Chernov diman4ik.chernov@gmail.com
 */

#ifndef SORT_SETTINGS_DIALOG
#define SORT_SETTINGS_DIALOG


#include <QDialog>
#include "ui_sortsettings.h"
#include "binding.h"


class SortSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    SortSettingsDialog( const QString& mainComb,
                        QList<binding>& bindings, QWidget* parent = 0 );
    
public slots:
    void accept();
    void reject();
    void bindingChanged( int, int );
    void mainSeqChanged( const QKeySequence& );
    void plusClicked( bool );
    void minusClicked( bool );
    void illegalBinding( int row, const binding& bin, const binding& prev );
    
signals:
    void bindingChangedSignal( int row, const binding& changed );
    void enabledChanged( bool );
    void mainCombChanged( const QString& newComb );
    void bindingRemoved( int row );
    
    void saveSettings();
    
private:
    Ui::Dialog m_ui;
};

#endif
