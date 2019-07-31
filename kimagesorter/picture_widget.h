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

#ifndef PICTURE_WIDGET
#define PICTURE_WIDGET

#include <QWidget>

#include <QResizeEvent>
#include <QLabel>


class PictureWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PictureWidget(QWidget* parent = 0);

    void resizeEvent( QResizeEvent * );
    void setImage( const QString& fileName );
    void clear();
    
public slots:
    void imageLoaded( QImage im );
signals:
    void load( const QString& name );
    void changed();
    void cantChange();

private:
    void showImage();
    
private:
    QPixmap m_pixmap;
    QLabel *m_imageLabel;
};

#endif
