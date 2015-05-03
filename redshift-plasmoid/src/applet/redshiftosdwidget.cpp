/************************************************************************
* Copyright (C) 2012 by Simone Gaiarin <simgunz@gmail.com>              *
* Adapted from kdemultimedia/kmix/osdwidget.cpp                         *
*                                                                       *
* This program is free software; you can redistribute it and/or modify  *
* it under the terms of the GNU General Public License as published by  *
* the Free Software Foundation; either version 3 of the License, or     *
* (at your option) any later version.                                   *
*                                                                       *
* This program is distributed in the hope that it will be useful,       *
* but WITHOUT ANY WARRANTY; without even the implied warranty of        *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
* GNU General Public License for more details.                          *
*                                                                       *
* You should have received a copy of the GNU General Public License     *
* along with this program; if not, see <http://www.gnu.org/licenses/>.  *
************************************************************************/

/*!
 * \file redshiftosdwidget.cpp
 *
 * Contains the implementations of the RedshiftOSDWidget class.
 */

#include "redshiftosdwidget.h"

#include <QFont>
#include <QGraphicsLinearLayout>
#include <QLabel>
#include <QTimer>

#include <KIcon>
#include <KIconLoader>
#include <KWindowSystem>

#include <Plasma/Label>
#include <Plasma/Theme>

RedshiftOSDWidget::RedshiftOSDWidget(QWidget * parent)
    : Plasma::Dialog(parent, Qt::ToolTip),
      m_iconLabel(new Plasma::Label),
      m_temperatureLabel(new Plasma::Label),
      m_autoHideTimer(new QTimer(this))
{
    //Setup the window properties
    KWindowSystem::setState(winId(), NET::KeepAbove);
    KWindowSystem::setType(winId(), NET::Tooltip);
    setAttribute(Qt::WA_X11NetWmWindowTypeToolTip, true);

    //Setup the auto-hide timer
    m_autoHideTimer->setInterval(2000);
    m_autoHideTimer->setSingleShot(true);
    connect(m_autoHideTimer, SIGNAL(timeout()), this, SLOT(hide()));

    //Cache the icon pixmaps
    QSize iconSize = QSize(KIconLoader::SizeLarge, KIconLoader::SizeLarge);
    m_redshiftPixmap = KIcon("redshift").pixmap(iconSize);

    //Setup the widgets
    m_iconLabel->nativeWidget()->setPixmap(m_redshiftPixmap);
    m_iconLabel->nativeWidget()->setFixedSize(iconSize);

    QFont font = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    font.setPixelSize(iconSize.height()-iconSize.height()*0.15);

    m_temperatureLabel->setFont(font);
    m_temperatureLabel->setText(QString::fromUtf8("9999 K"));
    m_temperatureLabel->setWordWrap(false);

    //Setup the OSD layout
    QGraphicsWidget *widget = new QGraphicsWidget();
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(widget);
    layout->addItem(m_iconLabel);
    layout->addItem(m_temperatureLabel);

    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->addItem(widget);

    setGraphicsWidget(widget);
}

QSize RedshiftOSDWidget::sizeHint() const
{
    int iconSize = m_iconLabel->nativeWidget()->pixmap()->height();
    return QSize(iconSize + m_temperatureLabel->nativeWidget()->sizeHint().width(), iconSize);
}

void RedshiftOSDWidget::activateOSD()
{
    show();
    m_autoHideTimer->start();
}

void RedshiftOSDWidget::setCurrentTemperature(int temperature)
{
    m_temperatureLabel->setText(QString::fromUtf8("%1 K").arg(temperature));
}

#include "redshiftosdwidget.moc"
