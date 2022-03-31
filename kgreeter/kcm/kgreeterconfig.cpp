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

#include <kdebug.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>

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

    layout()->setContentsMargins(0, 0, 0, 0);

    load();
}

KCMGreeter::~KCMGreeter()
{
}

void KCMGreeter::load()
{
    emit changed(false);
}

void KCMGreeter::save()
{
    emit changed(false);
}

#include "moc_kgreeterconfig.cpp"
