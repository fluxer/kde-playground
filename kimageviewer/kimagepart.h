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

#ifndef KIMAGEPART_H
#define KIMAGEPART_H

#include "kimagewidget.h"

#include <KParts/BrowserExtension>
#include <KParts/StatusBarExtension>
#include <KParts/Part>
#include <KUrl>

#include <QtGui/QGridLayout>

using KParts::StatusBarExtension;

namespace KImageViewer
{

class KImagePart;
class BrowserExtension : public KParts::BrowserExtension
{
public:
    explicit BrowserExtension(KImagePart*);
};

class KImagePart : public KParts::ReadOnlyPart
{
    Q_OBJECT

public:
    KImagePart(QWidget *, QObject *, const QList<QVariant>&);

    virtual bool openFile();
    virtual bool closeUrl();
    void setApplication(bool application);

private slots:
    void updateURL(const KUrl &);

private:
    BrowserExtension   *m_ext;
    StatusBarExtension *m_statusbar;
    QGridLayout *m_layout;
    KImageWidget *m_viewer;
};

} // namespace KImageViewer

#endif // KIMAGEPART_H
