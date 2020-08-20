/***********************************************************************
* Copyright 2016 Ivailo Monev <xakepa10@gmail.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy
* defined in Section 14 of version 3 of the license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/

#ifndef KIMAGEWIDGET_H
#define KIMAGEWIDGET_H

#include <QWidget>
#include <QEvent>

QT_BEGIN_NAMESPACE
class Ui_KImageUI;
QT_END_NAMESPACE

namespace KImageViewer
{

class KImageWidget : public QWidget
{
    Q_OBJECT

public:
    KImageWidget(QWidget *parent = 0);

    bool setImage(QString path, Qt::AspectRatioMode mode = Qt::KeepAspectRatio);
    bool setImage(QImage image, Qt::AspectRatioMode mode = Qt::KeepAspectRatio);
    bool setImage(QPixmap pixmap, Qt::AspectRatioMode mode = Qt::KeepAspectRatio);

    void setMode(Qt::AspectRatioMode mode);
    Qt::AspectRatioMode mode();
    void setApplication(bool application);
    bool application();
    void setWrite(bool write);
    bool write();

    QSize sizeHint() const;

public slots:
    bool saveImage();
    bool openImage();
    bool rotateLeft();
    bool rotateRight();
    bool changeMode(QString mode);
    bool changeFormat(QString mode);

protected:
    // reimplementation
    void resizeEvent(QResizeEvent *event);

private:
    Ui_KImageUI *m_ui;
    QPixmap resizeIfNeeded(QPixmap pixmap, Qt::AspectRatioMode mode);
    int m_resizehits;
    QPixmap m_original;
    Qt::AspectRatioMode m_mode;
    bool m_application;
    bool m_write;
};

} // namespace KImageViewer

#endif // KIMAGEWIDGET_H
