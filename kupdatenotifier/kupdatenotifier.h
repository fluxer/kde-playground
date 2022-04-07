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

#ifndef KUPDATENOTIFIER_H
#define KUPDATENOTIFIER_H

#include <QDBusInterface>
#include <kstatusnotifieritem.h>
#include <kaction.h>
#include <kmenu.h>
#include <khelpmenu.h>

class KUpdateNotifier : public KStatusNotifierItem
{
    Q_OBJECT
public:
    KUpdateNotifier(QObject* parent = nullptr);
    ~KUpdateNotifier();

private Q_SLOTS:
    void slotGotIt();
    void slotUpdatesChanged();

private:
    KAction* m_gotitaction;
    KMenu* m_menu;
    KHelpMenu* m_helpmenu;
    QDBusInterface m_interface;
};

#endif // KUPDATENOTIFIER_H
