/*  This file is part of the KDE project
    Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>

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

#include "kgreeterconfig.h"

#include <QSettings>
#include <QStyleFactory>
#include <QProcess>
#include <kdebug.h>
#include <klocale.h>
#include <kimageio.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kaboutdata.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>

#include "config-kgreeter.h"

K_PLUGIN_FACTORY(KCMGreeterFactory, registerPlugin<KCMGreeter>();)
K_EXPORT_PLUGIN(KCMGreeterFactory("kcmgreeterconfig", "kcm_greeterconfig"))

KCMGreeter::KCMGreeter(QWidget* parent, const QVariantList& args)
    : KCModule(KCMGreeterFactory::componentData(), parent)
{
    Q_UNUSED(args);

    setQuickHelp(i18n("<h1>KGreeter</h1>"
            "This module allows you to change KDE greeter options."));

    setupUi(this);

    KAboutData *about =
        new KAboutData(I18N_NOOP("KCMGreeter"), 0,
                       ki18n("KDE Greeter Module"),
                       0, KLocalizedString(), KAboutData::License_GPL,
                       ki18n("Copyright 2022, Ivailo Monev <email>xakepa10@gmail.com</email>"
                       ));

    about->addAuthor(ki18n("Ivailo Monev"), KLocalizedString(), "xakepa10@gmail.com");
    setAboutData(about);

    load();

    stylesbox->addItems(QStyleFactory::keys());
    connect(stylesbox, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotStyleChanged(QString)));

    // TODO: load name from General/Name
    const QStringList kcolorschemes = KGlobal::dirs()->findAllResources("data", "color-schemes/*.colors", KStandardDirs::NoDuplicates);
    foreach (const QString &kcolorscheme, kcolorschemes) {
        colorsbox->addItem(QFileInfo(kcolorscheme).baseName());
    }
    connect(colorsbox, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotColorChanged(QString)));

    backgroundrequester->setFilter(KImageIO::pattern(KImageIO::Reading));
    connect(backgroundrequester, SIGNAL(textChanged(QString)), this, SLOT(slotURLChanged(QString)));
    connect(backgroundrequester, SIGNAL(urlSelected(KUrl)), this, SLOT(slotURLChanged(KUrl)));

    rectanglerequester->setFilter(KImageIO::pattern(KImageIO::Reading));
    connect(rectanglerequester, SIGNAL(textChanged(QString)), this, SLOT(slotURLChanged(QString)));
    connect(rectanglerequester, SIGNAL(urlSelected(KUrl)), this, SLOT(slotURLChanged(KUrl)));

    m_lightdmexe = KStandardDirs::findRootExe("lightdm");
    testbutton->setEnabled(!m_lightdmexe.isEmpty());
    connect(testbutton, SIGNAL(pressed()), this, SLOT(slotTest()));
}

KCMGreeter::~KCMGreeter()
{
}

void KCMGreeter::load()
{
    QSettings kgreetersettings(SYSCONF_INSTALL_DIR "/lightdm/lightdm-kgreeter-greeter.conf", QSettings::IniFormat);

    const QString kgreeterstyle = kgreetersettings.value("greeter/style").toString();
    if (!kgreeterstyle.isEmpty()) {
        for (int i = 0; i < stylesbox->count(); i++) {
            if (stylesbox->itemText(i) == kgreeterstyle) {
                stylesbox->setCurrentIndex(i);
                break;
            }
        }
    }

    const QString kgreetercolor = kgreetersettings.value("greeter/colorscheme").toString();
    if (!kgreetercolor.isEmpty()) {
        for (int i = 0; i < colorsbox->count(); i++) {
            if (colorsbox->itemText(i) == kgreetercolor) {
                colorsbox->setCurrentIndex(i);
                break;
            }
        }
    }

    const QString kgreeterbackground = kgreetersettings.value("greeter/background").toString();
    backgroundrequester->setUrl(KUrl(kgreeterbackground));

    const QString kgreeterrectangle = kgreetersettings.value("greeter/rectangle").toString();
    rectanglerequester->setUrl(KUrl(kgreeterrectangle));

    emit changed(false);
}

void KCMGreeter::save()
{
    emit changed(false);
}

void KCMGreeter::slotStyleChanged(const QString &style)
{
    Q_UNUSED(style);
    emit changed(true);
}

void KCMGreeter::slotColorChanged(const QString &style)
{
    Q_UNUSED(style);
    emit changed(true);
}

void KCMGreeter::slotURLChanged(const QString &url)
{
    Q_UNUSED(url);
    emit changed(true);
}

void KCMGreeter::slotURLChanged(const KUrl &url)
{
    Q_UNUSED(url);
    emit changed(true);
}

void KCMGreeter::slotTest()
{
    if (!QProcess::startDetached(m_lightdmexe, QStringList() << QString::fromLatin1("--test-mode"))) {
        KMessageBox::error(this, i18n("Could not start LightDM"));
    }
}

#include "moc_kgreeterconfig.cpp"
