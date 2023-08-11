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

#include "xwallpaper.h"

#include <QX11Info>
#include <QApplication>
#include <QDesktopWidget>
#include <KDebug>

static const QString s_xscreensaver = QString::fromLatin1("/usr/libexec/xscreensaver/binaryring");
static const int s_updateinterval = 100; // increase to decrease CPU usage

XWallpaper::XWallpaper(QObject *parent, const QVariantList &args)
    : Plasma::Wallpaper(parent, args),
    m_widget(nullptr),
    m_proc(nullptr),
    m_timer(nullptr)
{
    m_widget = new QWidget(nullptr, Qt::X11BypassWindowManagerHint);
    // move it outside the screen space, should the resolution change the widget may be visible but
    // neither rendering nor grabbing the window works without the winodow being visible
    const QRect trect = QApplication::desktop()->geometry();
    m_widget->move(trect.bottom(), trect.right());
    m_widget->show();

    m_proc = new QProcess(this);
    m_proc->start(
        s_xscreensaver,
        QStringList() << "--window-id" << QString::number(qlonglong(m_widget->winId()))
    );
    if (!m_proc->waitForStarted()) {
        kWarning() << "Could not start" << s_xscreensaver;
        return;
    } else {
        kDebug() << "Started" << s_xscreensaver << QString::number(qlonglong(m_widget->winId()));
    }

    m_timer = new QTimer(this);
    m_timer->setInterval(s_updateinterval);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
    m_timer->start();
}

XWallpaper::~XWallpaper()
{
    m_timer->stop();
    m_proc->terminate();
    m_proc->waitForFinished();
    delete m_widget;
}

void XWallpaper::paint(QPainter *painter, const QRectF &exposedRect)
{
    kDebug() << "Rendering" << s_xscreensaver << targetSizeHint().toSize();
    const QSize tsize = targetSizeHint().toSize();
    m_widget->resize(tsize.width(), tsize.height());
    painter->drawPixmap(QPoint(), QPixmap::grabWindow(m_widget->winId()));
}

void XWallpaper::slotTimeout()
{
    emit update(QRectF());
}

#include "moc_xwallpaper.cpp"
