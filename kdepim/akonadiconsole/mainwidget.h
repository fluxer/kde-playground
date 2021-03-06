/*
    This file is part of Akonadi.

    Copyright (c) 2006 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#ifndef AKONADICONSOLE_MAINWIDGET_H
#define AKONADICONSOLE_MAINWIDGET_H

#include "browserwidget.h"

#include <QWidget>

class KXmlGuiWindow;

class MainWidget : public QWidget
{
  Q_OBJECT

  public:
    explicit MainWidget( KXmlGuiWindow *parent = 0 );
    ~MainWidget();

  private Q_SLOTS:
    void createSearch();
    void startServer();
    void stopServer();
    void restartServer();
    void configureServer();

  private:
    BrowserWidget *mBrowser;
};

#endif
