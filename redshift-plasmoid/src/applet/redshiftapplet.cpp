/************************************************************************
* Copyright (C) 2012 by Simone Gaiarin <simgunz@gmail.com>              *
*                                                                       *
* This program is free software; you can redistribute it and/or modify  *
* it under the terms of the GNU General Public License as published by  *
* the Free Software Foundation; either version 3 of the License, or     *
* (at your option) any later version.                                   *
*                                                                       *
* This program is distributed in the hope that it will be useful,       *
* but WITHOUT ANY WARRANTY; without even the implied warranty of        *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
* GNU General Public License for more details.                          *
*                                                                       *
* You should have received a copy of the GNU General Public License     *
* along with this program; if not, see <http://www.gnu.org/licenses/>.  *
************************************************************************/

/*!
 * \file redshiftapplet.cpp
 *
 * Contains the implementation of the RedshiftApplet class.
 */

#include "redshiftapplet.h"

#include "redshiftsettings.h"
#include "redshiftosdwidget.h"

#include <QtGui/QDesktopWidget>

#include <KConfigDialog>
#include <KComboBox>
#include <KApplication>

#include <Plasma/DataEngine>
#include <Plasma/ToolTipManager>

RedshiftApplet::RedshiftApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_redshiftOSD(new RedshiftOSDWidget())
{
    setBackgroundHints(StandardBackground);
    setAspectRatioMode(Plasma::ConstrainedSquare);
    setHasConfigurationInterface(true);
}

RedshiftApplet::~RedshiftApplet()
{
    delete m_redshiftOSD;
}

void RedshiftApplet::init()
{
    //Initialize the plasmoid using an IconWidget
    m_button = new Plasma::IconWidget(this);
    m_button->setIcon(KIcon("redshift-status-off"));
    m_layout = new QGraphicsGridLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addItem(m_button, 0, 0);

    //Initialize to 3 seconds the wait timer used to wait before changing the applet status
    m_setStatusTimer = new QTimer();
    m_setStatusTimer->setInterval(3000);
    m_setStatusTimer->setSingleShot(true);

    //Set the default applet status
    setStatus(Plasma::PassiveStatus);

    //Connect to the data engine
    m_engine = dataEngine("redshift");
    m_engine->connectSource("Controller", this);

    //Connect signals and slots
    connect(m_button, SIGNAL(clicked()), this, SLOT(toggle()));
    connect(m_setStatusTimer, SIGNAL(timeout()), this, SLOT(setAppletStatus()));
    //The activate signal is emitted whenever the plasmoid get the focus,
    // i.e, when the user trigger the global shortcut
    connect(this, SIGNAL(activate()), this, SLOT(toggle()));
}

void RedshiftApplet::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
{
    if (sourceName == "Controller") {
        Plasma::ToolTipContent tooltip;
        if (data["Status"].toString().indexOf("Running") == 0) {
            m_button->setIcon(KIcon("redshift-status-on"));
            tooltip.setSubText(i18nc("Action the user can perform","Click to toggle off. "
                                       "Scroll the mouse wheel to set the color temperature manually."));
            tooltip.setImage(KIcon("redshift-status-on"));
            m_appletStatus = Plasma::PassiveStatus;
        } else {
            m_button->setIcon(KIcon("redshift-status-off"));
            tooltip.setSubText(i18nc("Action the user can perform","Click to toggle on. "
                                        "Scroll the mouse wheel to set the color temperature manually."));
            tooltip.setImage(KIcon("redshift-status-off"));
            m_appletStatus = Plasma::PassiveStatus;
        }
        if (data["Status"].toString() == "RunningManual") {
            tooltip.setSubText(i18nc("Action the user can perform","Click to switch to auto mode. "
                                       "Scroll the mouse wheel to change the color temperature."));
            m_button->setIcon(KIcon("redshift-status-manual"));
            m_appletStatus = Plasma::ActiveStatus;
        }
        //Start the timer to change the status, if the timer is already active this will restart it
        m_setStatusTimer->start();
        Plasma::ToolTipManager::self()->setContent(this, tooltip);
    }
    int temperature = data["Temperature"].toInt();
    //Show the OSD only if the temperature is non-zero, i.e., only when redshift is inn "Manual" mode
    if(temperature) {
        showRedshiftOSD(temperature);
    }
}

void RedshiftApplet::toggle()
{
    m_redshiftOSD->hide();
    Plasma::Service *service = m_engine->serviceForSource("Controller");
    service->startOperationCall(service->operationDescription("toggle"));
}

void RedshiftApplet::showRedshiftOSD(int temperature)
{
    m_redshiftOSD->setCurrentTemperature(temperature);
    m_redshiftOSD->activateOSD(); //Show the OSD and enable the hide timer

    //Center the OSD
    QRect rect = KApplication::kApplication()->desktop()->screenGeometry(QCursor::pos());
    QSize size = m_redshiftOSD->sizeHint();
    int posX = rect.x() + (rect.width() - size.width()) / 2;
    int posY = rect.y() + 4 * rect.height() / 5;
    m_redshiftOSD->setGeometry(posX, posY, size.width(), size.height());
}

void RedshiftApplet::createConfigurationInterface(KConfigDialog *parent)
{
    //Create the redshift parameters configuration page
    QWidget *redshiftInterface = new QWidget(parent);
    m_redshiftUi = new Ui::RedshiftConfig();
    m_redshiftUi->setupUi(redshiftInterface);
    parent->addPage(redshiftInterface, RedshiftSettings::self(),
                    i18nc("Redshift main configuration page. Title Capitalization.","General"), "redshift");

    //Create the activities configuration page
    QWidget *activitiesInterface = new QWidget(parent);
    m_activitiesUi = new Ui::ActivitiesConfig();
    m_activitiesUi->setupUi(activitiesInterface);

    //Get the list of KDE activities
    Plasma::DataEngine *activities_engine = dataEngine("org.kde.activities");
    QStringList activities = activities_engine->sources();
    //The last source is not an activity, but is the Status source
    activities.removeLast();

    //Get the redshift-plasmoid activities configuration
    RedshiftSettings::self()->readConfig();
    const QStringList alwaysOnActivities = RedshiftSettings::alwaysOnActivities();
    const QStringList alwaysOffActivities = RedshiftSettings::alwaysOffActivities();

    QString act;
    foreach(act, activities) {
        Plasma::DataEngine::Data data = activities_engine->query(act);
        QTreeWidgetItem *listItem = new QTreeWidgetItem(m_activitiesUi->activities);
        KComboBox *itemCombo = new KComboBox(m_activitiesUi->activities);
        listItem->setText(0, data["Name"].toString());
        listItem->setIcon(0, KIcon(data["Icon"].toString()));
        listItem->setFlags(Qt::ItemIsEnabled);
        listItem->setData(0, Qt::UserRole, act);

        itemCombo->addItem(i18nc("Redshift state is set to «Auto» mode in this activity. Title Capitalization.", "Auto"));
        itemCombo->addItem(i18nc("Redshift is forced to be enabled in this activity. Title Capitalization.", "Always Enabled"));
        itemCombo->addItem(i18nc("Redshift is forced to be disabled in this activity. Title Capitalization.", "Always Disabled"));

        if (alwaysOnActivities.contains(act)) {
            itemCombo->setCurrentIndex(1);
        } else if (alwaysOffActivities.contains(act)) {
            itemCombo->setCurrentIndex(2);
        } else {
            itemCombo->setCurrentIndex(0);
        }

        m_activitiesUi->activities->setItemWidget(listItem, 1, itemCombo);
        connect(itemCombo, SIGNAL(currentIndexChanged(int)), parent, SLOT(settingsModified()));
    }
    m_activitiesUi->activities->resizeColumnToContents(0);
    parent->addPage(activitiesInterface, i18nc("Redshift activities behaviour configuration page. Title Capitalization.", "Activities"),
                    "preferences-activities");

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void RedshiftApplet::configChanged()
{
    Plasma::Service *service = m_engine->serviceForSource("Controller");
    service->startOperationCall(service->operationDescription("restart"));
}

void RedshiftApplet::wheelEvent(QGraphicsSceneWheelEvent *e)
{
    Plasma::ToolTipManager::self()->hide(this);
    Plasma::Service *service = m_engine->serviceForSource("Controller");

    if(e->delta() < 0) {
        service->startOperationCall(service->operationDescription("decrease"));
    } else {
        service->startOperationCall(service->operationDescription("increase"));
    }
}

void RedshiftApplet::configAccepted()
{
    QStringList alwaysOnActivities;
    QStringList alwaysOffActivities;

    QTreeWidget *activitiesList = m_activitiesUi->activities;
    for (int i = 0; i < activitiesList->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = activitiesList->topLevelItem(i);
        KComboBox *itemCombo = static_cast<KComboBox *>(activitiesList->itemWidget(item, 1));
        const QString act = item->data(0, Qt::UserRole).toString();
        if (itemCombo->currentIndex() == 1) {
            alwaysOnActivities << act;
        } else if (itemCombo->currentIndex() == 2) {
            alwaysOffActivities << act;
        }
    }
    RedshiftSettings::setAlwaysOnActivities(alwaysOnActivities);
    RedshiftSettings::setAlwaysOffActivities(alwaysOffActivities);
    RedshiftSettings::self()->writeConfig();
}

void RedshiftApplet::setAppletStatus()
{
    setStatus(m_appletStatus);
}

K_EXPORT_PLASMA_APPLET(redshift, RedshiftApplet)

#include "redshiftapplet.moc"
