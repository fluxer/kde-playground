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

#ifndef ANIPAPER_H
#define ANIPAPER_H

#include <QTimer>
#include <QImageReader>
#include <Plasma/Wallpaper>
#include <Plasma/DataEngine>

class AniPaper : public Plasma::Wallpaper
{
    Q_OBJECT

public:
    AniPaper(QObject *parent, const QVariantList &args);
    ~AniPaper();

    void paint(QPainter* painter, const QRectF &exposedRect);

private Q_SLOTS:
    void slotTimeout();

private:
    QTimer* m_timer;
    QImageReader m_imagereader;
};

K_EXPORT_PLASMA_WALLPAPER(anipaper, AniPaper)

#endif // ANIPAPER_H
