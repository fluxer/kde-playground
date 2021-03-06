/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "sieveeditorconfiguredialog.h"
#include "serversievelistwidget.h"
#include "sieveeditorconfigureserverwidget.h"
#include "pimcommon/widgets/configureimmutablewidgetutils.h"
#include "sieveeditorglobalconfig.h"

#include <KLocalizedString>
#include <KSharedConfig>

#include <QTabWidget>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QCheckBox>

SieveEditorConfigureDialog::SieveEditorConfigureDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Configure" ) );
    setButtons( Cancel | Ok  );
    mTabWidget = new QTabWidget;
    setMainWidget(mTabWidget);
    initializeServerSieveSettings();
    readConfig();
}

SieveEditorConfigureDialog::~SieveEditorConfigureDialog()
{
    writeConfig();
}

void SieveEditorConfigureDialog::initializeServerSieveSettings()
{
    QWidget *w = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout;
    w->setLayout(vbox);
    mTabWidget->addTab(w, i18n("Server Sieve"));
    mServerWidget = new SieveEditorConfigureServerWidget;
    vbox->addWidget(mServerWidget);

    mCloseWallet = new QCheckBox(i18n("Close wallet when close application"));
    vbox->addWidget(mCloseWallet);
    loadServerSieveConfig();
}

void SieveEditorConfigureDialog::loadServerSieveConfig()
{
    mServerWidget->readConfig();
    PimCommon::ConfigureImmutableWidgetUtils::loadWidget(mCloseWallet, SieveEditorGlobalConfig::self()->closeWalletItem());
}

void SieveEditorConfigureDialog::saveServerSieveConfig()
{
    mServerWidget->writeConfig();
    PimCommon::ConfigureImmutableWidgetUtils::saveCheckBox(mCloseWallet, SieveEditorGlobalConfig::self()->closeWalletItem());
    SieveEditorGlobalConfig::self()->writeConfig();
}

void SieveEditorConfigureDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "SieveEditorConfigureDialog" );
    const QSize size = group.readEntry( "Size", QSize(600, 400) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void SieveEditorConfigureDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "SieveEditorConfigureDialog" );
    group.writeEntry( "Size", size() );
    group.sync();
}
