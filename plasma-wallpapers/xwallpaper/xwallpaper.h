/*  This file is part of the KDE project
    Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

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

#ifndef XWALLPAPER_H
#define XWALLPAPER_H

#include <QWidget>
#include <QProcess>
#include <QTimer>
#include <Plasma/Wallpaper>
#include <Plasma/DataEngine>

class XWallpaper : public Plasma::Wallpaper
{
    Q_OBJECT

public:
    XWallpaper(QObject* parent, const QVariantList& args);
    ~XWallpaper();

    void paint(QPainter* painter, const QRectF &exposedRect);

private Q_SLOTS:
    void slotTimeout();

private:
    QWidget* m_widget;
    QProcess* m_proc;
    QTimer* m_timer;
};

K_EXPORT_PLASMA_WALLPAPER(xwallpaper, XWallpaper)

#endif // XWALLPAPER_H
