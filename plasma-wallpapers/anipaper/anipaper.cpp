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

#include "anipaper.h"

#include <KDebug>

static const QString s_image = QString::fromLatin1("/home/smil3y/katana/kde-samples/image/3.webp");
static const Qt::AspectRatioMode s_aspect = Qt::KeepAspectRatio;

AniPaper::AniPaper(QObject *parent, const QVariantList &args)
    : Plasma::Wallpaper(parent, args),
    m_timer(nullptr),
    m_imagereader(s_image)
{
    if (m_imagereader.imageCount() <= 0) {
        kWarning() << "No images in" << s_image;
        return;
    } else {
        kDebug() << "Started" << s_image << m_imagereader.nextImageDelay();
    }

    m_timer = new QTimer(this);
    m_timer->setInterval(m_imagereader.nextImageDelay());
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
    m_timer->start();
}

AniPaper::~AniPaper()
{
    m_timer->stop();
}

void AniPaper::paint(QPainter *painter, const QRectF &exposedRect)
{
    const QSize tsize = targetSizeHint().toSize();
    QImage frame = m_imagereader.read().scaled(tsize, s_aspect, Qt::SmoothTransformation);
    const QSize fsize = frame.size();
    const QPoint offset = (QPoint(tsize.width() - fsize.width(), tsize.height() - fsize.height()) / 2);
    kDebug() << "Rendering" << s_image << tsize << offset;
    painter->drawImage(offset, frame);
}

void AniPaper::slotTimeout()
{
    m_imagereader.jumpToNextImage();
    emit update(QRectF());
}

#include "moc_anipaper.cpp"
