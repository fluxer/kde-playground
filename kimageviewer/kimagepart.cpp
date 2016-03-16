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

#include "kimagepart.h"

#include <KAboutData>
#include <KAction>
#include <KActionCollection>
#include <KLocale>
#include <KMessageBox>
#include <KStandardAction>
#include <KStatusBar>
#include <KPluginFactory>

#include <QtCore/QByteArray>
#include <QtCore/QDir>
#include <QtGui/QScrollArea>

namespace KImageViewer {

K_PLUGIN_FACTORY(KImageViewerPartFactory, registerPlugin<KImagePart>();)  // produce a factory
K_EXPORT_PLUGIN(KImageViewerPartFactory(KAboutData(
               "kimageviewerpart",
               0,
               ki18n("KImageViewer"),
               "1.0.0",
               ki18n("Simple image viewer for KDE."),
               KAboutData::License_GPL_V2,
               ki18n("(c) 2014 Ivailo Monev"),
               KLocalizedString(),
               "http://github.com/fluxer/kde-playground",
               "xakepa10@gmail.com").
               setProgramIconName(QLatin1String("KImageViewer")).
               setCatalogName("kimageviewer")))

BrowserExtension::BrowserExtension(KImagePart *parent)
        : KParts::BrowserExtension(parent)
{}


KImagePart::KImagePart(QWidget *parentWidget, QObject *parent, const QList<QVariant>&)
        : ReadOnlyPart(parent)
        , m_ext(new BrowserExtension(this))
        , m_statusbar(new StatusBarExtension(this))
        , m_viewer(0)
{
    setComponentData(KImageViewerPartFactory::componentData());

    m_viewer = new KImageWidget(parentWidget);
    setWidget(m_viewer);
}

void KImagePart::setApplication(bool application)
{
    m_viewer->setApplication(application);
}

bool KImagePart::openFile()
{
    KUrl url = KUrl::fromPath(localFilePath());
    if (url.isEmpty()) {
        //do nothing, chances are the user accidently pressed ENTER
    } else if (!url.isValid()) {
        KMessageBox::information(widget(), i18n("The entered URL is not invalid."));
    } else if (url.protocol() != QLatin1String("file")) {
        KMessageBox::information(widget(), i18n("Only file paths are accepted."));
    } else {
        m_viewer->setImage(url.path());
        m_statusbar->statusBar()->showMessage(i18n("Opened: %1", url.path()));
        return true;
    }

    return false;
}

bool KImagePart::closeUrl()
{
    m_statusbar->statusBar()->showMessage(i18n("Closing viewer..."));

    return ReadOnlyPart::closeUrl();
}

void KImagePart::updateURL(const KUrl &u)
{
    // update the interface
    emit m_ext->openUrlNotify(); //must be done first
    emit m_ext->setLocationBarUrl(u.prettyUrl());

    m_viewer->setImage(u.path());

    //do this last, or it breaks Konqi location bar
    setUrl(u);
}

} //namespace KImageViewer

#include "moc_kimagepart.cpp"
