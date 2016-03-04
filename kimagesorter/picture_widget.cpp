/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2011, Dmitry Chernov diman4ik.chernov@gmail.com
 */

#include "picture_widget.h"
#include "image_loader.h"

#include <QLabel>
#include <QBoxLayout>


PictureWidget::PictureWidget( QWidget* parent ) : QWidget(parent)
{
    setBackgroundRole( QPalette::Dark );
    setAutoFillBackground(true);
    
    m_imageLabel = new QLabel(this);
    m_imageLabel->setBackgroundRole( QPalette::Dark );
    m_imageLabel->setAlignment( Qt::AlignCenter );

    QBoxLayout *mainLay = new QHBoxLayout( this );
    mainLay->setMargin( 0 );
    mainLay->addWidget( m_imageLabel );
    
    ImageLoaderThread* ilt = new ImageLoaderThread( this );
}

void PictureWidget::setImage( const QString& fileName )
{
    //m_pixmap = QPixmap::fromImage( QImage(fileName) );
    //showImage();
    emit( load( fileName ) );
}

void PictureWidget::imageLoaded( QImage im )
{
    if( !im.isNull() )
    {
        m_pixmap = QPixmap::fromImage( im );
        showImage();
        emit( changed() );
    }
    else
    {
        clear();
        emit( cantChange() );
    }
}

void PictureWidget::showImage()
{
    int im_w = m_pixmap.width();
    int im_h = m_pixmap.height();

    QRect rect;

    int w_width = width();
    int w_height = height();
    
    if( ( im_w > w_width ) || ( im_h > w_height ) )
    {
        QPixmap pixmap_scalled = m_pixmap.scaled( w_width, w_height, Qt::KeepAspectRatio );
        m_imageLabel->setPixmap( pixmap_scalled );
    }
    else
    {
        m_imageLabel->setPixmap( m_pixmap );
    }
}

void PictureWidget::resizeEvent( QResizeEvent * )
{
    showImage();
}

void PictureWidget::clear()
{
    m_imageLabel->clear();
}
