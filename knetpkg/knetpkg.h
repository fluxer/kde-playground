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

#ifndef KNETPKG_H
#define KNETPKG_H

#include <kmainwindow.h>

#include "ui_knetpkg.h"

struct KNetPkgInfo {
    QByteArray name;
    QByteArray description;
    QByteArray requiredby;
    QByteArray size;
};

class KNetPkg : public KMainWindow
{
    Q_OBJECT
public:
    explicit KNetPkg(QWidget *parent = 0);
    ~KNetPkg();

public Q_SLOTS:
    void slotItemChanged(QListWidgetItem *knetpkgitem);

private:
    Ui_KNetPkgWindow m_ui;
    QList<KNetPkgInfo> m_packages;
};

#endif // KNETPKG_H
