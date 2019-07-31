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

#ifndef MAIN_WINDOW
#define MAIN_WINDOW

#include <KXmlGuiWindow>
#include <KUrl>

#include "binding.h"

class PictureWidget;
class ImageSorter;
class SortSettingsDialog;
#include <QActionGroup>


class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT
public:
    explicit MainWindow( QWidget *parent = 0 );

    void openFile(KUrl url);

public slots:
    void changed();
    void cantChange();
    void noNext();
    void noPrevious();
    void gotNext();
    void gotPrevious();
    
    void configureSort();
    void addNewAction( int row );
    void setCustomActions();

private:
    void setupActions();
    
    PictureWidget* m_picWidget;    
    
    SortSettingsDialog* m_settingsDialog;
    QActionGroup* m_sortActions;
    
    ImageSorter* m_imSorter;
    QString m_curFileName;
    bool m_canAct;

private slots:
    void openFile();
    void nextFile();
    void prevFile();
    
    void actionTriggered( QAction* act );
};

#endif
