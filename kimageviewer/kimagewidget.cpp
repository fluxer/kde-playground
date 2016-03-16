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

#include "kimagewidget.h"
#include "ui_kimageui.h"

#include <KDebug>
#include <KFileDialog>
#include <KMessageBox>
#include <KLocale>
#include <KFileMetaDataWidget>

namespace KImageViewer {

#define mimefilter "image/gif image/jpeg image/png image/bmp image/x-eps image/x-ico image/x-portable-bitmap image/x-portable-graymap image/x-portable-pixmap image/x-xbitmap image/x-xpixmap image/tiff image/x-psd image/x-webp image/webp"

KImageWidget::KImageWidget(QWidget *parent)
    : QWidget(parent), m_ui(0)
{
    m_ui = new Ui_KImageUI();
    m_ui->setupUi(this);
    m_mode = Qt::KeepAspectRatio;
    setApplication(false);
 
    connect(m_ui->m_open, SIGNAL(clicked()), this, SLOT(openImage()));
    connect(m_ui->m_save, SIGNAL(clicked()), this, SLOT(saveImage()));
    connect(m_ui->m_rotateleft, SIGNAL(clicked()), this, SLOT(rotateLeft()));
    connect(m_ui->m_rotateright, SIGNAL(clicked()), this, SLOT(rotateRight()));
    connect(m_ui->m_view, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeMode(QString)));
    connect(m_ui->m_quit, SIGNAL(clicked()), qApp, SLOT(quit()));
}

bool KImageWidget::saveImage()
{
    if (m_ui->m_image->pixmap()->isNull()) {
        return false;
    }

    QString path = KFileDialog::getSaveFileName(KUrl(), QLatin1String(mimefilter), this);
    
    if (path.isEmpty()) {
        // probably canceled open request
        return false;
    } else if (!m_ui->m_image->pixmap()->save(path)) {
        KMessageBox::information(this, i18n("Could not save image to path: %1.", path));
        return false;
    }
    return true;
}

bool KImageWidget::openImage()
{
    QString path = KFileDialog::getOpenFileName(KUrl(), QLatin1String(mimefilter), this);
    
    if (path.isEmpty()) {
        // probably canceled open request
        return false;
    }
    if (!setImage(path)) {
        return false;
    }
    return true;
}

bool KImageWidget::rotateLeft()
{
    // TODO: implement
    return false;
}

bool KImageWidget::rotateRight()
{
    // TODO: implement
    return false;
}

bool KImageWidget::changeMode(QString mode)
{
    if (mode == QLatin1String("Ignore")) {
        m_ui->m_image->setPixmap(resizeIfNeeded(m_original, Qt::IgnoreAspectRatio));
        return true;
    } else if (mode == QLatin1String("Keep")) {
        m_ui->m_image->setPixmap(resizeIfNeeded(m_original, Qt::KeepAspectRatio));
        return true;
    } else if (mode == QLatin1String("Expanding")) {
        m_ui->m_image->setPixmap(resizeIfNeeded(m_original, Qt::KeepAspectRatioByExpanding));
        return true;
    }
    return false;
}

bool KImageWidget::setImage(QString path, Qt::AspectRatioMode mode)
{
    QPixmap p;
    p.load(path);
    if (p.isNull()) {
        return false;
    }
    m_original = p;
    m_ui->m_image->setPixmap(resizeIfNeeded(p, mode));
    setWrite(true); // TODO: make that conditional
    return true;
}

bool KImageWidget::setImage(QImage image, Qt::AspectRatioMode mode)
{
    QPixmap p = QPixmap::fromImage(image);
    if (p.isNull()) {
        return false;
    }
    m_original = p;
    m_ui->m_image->setPixmap(resizeIfNeeded(p, mode));
    setWrite(true); // TODO: make that conditional
    return true;
}

bool KImageWidget::setImage(QPixmap pixmap, Qt::AspectRatioMode mode)
{
    if (pixmap.isNull()) {
        return false;
    }
    m_original = pixmap;
    m_ui->m_image->setPixmap(resizeIfNeeded(pixmap, mode));
    setWrite(true); // TODO: make that conditional
    return true;
}

void KImageWidget::setMode(Qt::AspectRatioMode mode)
{
    m_mode = mode;
    if (!m_original.isNull()) {
        m_ui->m_image->setPixmap(resizeIfNeeded(m_original, mode));
    }
}

Qt::AspectRatioMode KImageWidget::mode()
{
    return m_mode;
}

void KImageWidget::setApplication(bool application)
{
    m_application = application;
    if (application) {
        m_ui->m_open->setEnabled(true);
        m_ui->m_quit->show();
    } else {
        m_ui->m_open->setEnabled(false);
        m_ui->m_quit->hide();
    }
}

bool KImageWidget::application()
{
    return m_application;
}

void KImageWidget::setWrite(bool write)
{
    m_write = write;
    if (write) {
        m_ui->m_save->setEnabled(true);
        m_ui->m_rotateleft->setEnabled(true);
        m_ui->m_rotateright->setEnabled(true);
    } else {
        m_ui->m_save->setEnabled(false);
        m_ui->m_rotateleft->setEnabled(false);
        m_ui->m_rotateright->setEnabled(false);
    }
}

bool KImageWidget::write()
{
    return m_write;
}

QSize KImageWidget::sizeHint() const
{
    if (m_original.isNull()) {
       // since no image has been set yet the size is useless
       return QSize(640, 480);
    }
    return m_ui->m_image->size();
}

QPixmap KImageWidget::resizeIfNeeded(QPixmap pixmap, Qt::AspectRatioMode mode)
{
    if (pixmap.height() > m_ui->m_image->height()) {
        return pixmap.scaled(m_ui->m_image->size(), mode);
    }
    return pixmap;
}

void KImageWidget::resizeEvent(QResizeEvent *event)
{
    if(m_original.isNull()) {
        // can happend upon application startup before setImage()
        return;
    }
    m_ui->m_image->setPixmap(resizeIfNeeded(m_original, m_mode));
}

} //namespace KImageViewer

#include "moc_kimagewidget.cpp"
