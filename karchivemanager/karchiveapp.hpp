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

#ifndef KARCHIVEAPP_H
#define KARCHIVEAPP_H

#include <QMainWindow>
#include <QItemSelection>

class KArchiveAppPrivate;

class KArchiveApp : public QMainWindow {
    Q_OBJECT

    public:
        KArchiveApp();
        ~KArchiveApp();

        void changePath(const QString path);

    public Q_SLOTS:
        void slotOpenAction();
        void slotQuitAction();
        void slotAddAction();
        void slotRemoveAction();
        void slotExtractAction();

        void slotSelectionChanged(const QItemSelection &current, const QItemSelection &previous);

        void slotLoadStarted();
        void slotLoadFinished();

    private:
        KArchiveAppPrivate *d;
};

#endif // KARCHIVEAPP_H
