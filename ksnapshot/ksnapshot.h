/*  This file is part of the KDE project
    Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>

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

#ifndef KSNAPSHOT_H
#define KSNAPSHOT_H

#include <QResizeEvent>
#include <kmainwindow.h>

#include "snapshottimer.h"
#include "ui_ksnapshotwidget.h"

class KSnapshot : public KMainWindow
{
    Q_OBJECT
public:
    enum KSnapshotMode {
        CaptureDesktop = 0,
        CaptureActiveWindow = 1
    };

    explicit KSnapshot(QWidget *parent = 0);
    ~KSnapshot();

public:
    void snapshot(const KSnapshotMode mode, const int delay);

private Q_SLOTS:
    void slotGrabSnapshot();
    void slotSaveSnapshot();

    void slotGrabPixmap();

private:
    QPixmap scaledSnapshot() const;

    SnapshotTimer m_countdown;
    QPixmap m_snapshot;
    Ui_KSnapshotWindow m_ui;
};

#endif // KSNAPSHOT_H
