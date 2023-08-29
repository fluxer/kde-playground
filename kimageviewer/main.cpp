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
#include <KCmdLineArgs>
#include <KApplication>
#include <KPluginLoader>
#include <KPluginFactory>
#include <KParts/Part>
#include <KLocale>
#include <KMessageBox>
#include <KDebug>
#include <KMainWindow>

int main(int argc, char **argv) {
    KAboutData aboutData(
        "kimageviewer", 0, ki18n("KImageViewer"),
        "1.0.0", ki18n("Simple image viewer for KDE."),
        KAboutData::License_GPL_V2,
        ki18n("(c) 2016 Ivailo Monev")
    );

    aboutData.addAuthor(ki18n("Ivailo Monev"),
                        ki18n("Maintainer"),
                        "xakepa10@gmail.com");
    aboutData.setProgramIconName(QLatin1String("view-preview"));

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineOptions option;
    option.add("+[url]", ki18n("URL to be opened"));
    KCmdLineArgs::addCmdLineOptions(option);

    KApplication *app = new KApplication();

    KMainWindow *window = new KMainWindow();

    KPluginLoader loader(QLatin1String("kimageviewerpart"));
    KPluginFactory *factory = loader.factory();
    KParts::ReadOnlyPart *part;
    if (factory) {
        part = factory->create<KParts::ReadOnlyPart>(window);
    }
    if (!factory || !part) {
        KMessageBox::error(window, i18n("Unable to load KImageViwer's KPart component, please check your installation."));
        kWarning() << "Error loading KImageViwer KPart: " << loader.errorString();
        return false;
    }

    part->setObjectName(QLatin1String("KImageViwer"));
    static_cast<KImageViewer::KImageWidget*>(part->widget())->setApplication(true);
    window->setCentralWidget(part->widget());
    // window->setAutoSaveSettings();
    window->show();

    app->connect(app, SIGNAL(saveYourself()), part, SLOT(closeUrl()));

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    for (int pos = 0; pos < args->count(); ++pos) {
        part->openUrl(args->url(pos));
    }

    return app->exec();
}
