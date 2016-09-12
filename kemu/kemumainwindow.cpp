/*  This file is part of the KDE libraries
    Copyright (C) 2016 Ivailo Monev <xakepa10@gmail.com>

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

#include <KAction>
#include <KActionCollection>
#include <KLocale>
#include <KIcon>
#include <KInputDialog>
#include <KStandardDirs>
#include <KDebug>
#include <QApplication>
#include <QMessageBox>

#include "kemumainwindow.h"
#include "ui_kemu.h"

KEmuMainWindow::KEmuMainWindow(QWidget *parent, Qt::WindowFlags flags)
    : KXmlGuiWindow(parent, flags), m_kemuui(new Ui_KEmuWindow)
{
    m_kemuui->setupUi(this);
    m_kemuui->startStopButton->setText(i18n("Start"));
    m_kemuui->startStopButton->setIcon(KIcon("system-run"));

    KAction *a = actionCollection()->addAction("harddisk_create", this, SLOT(createHardDrive()));
    a->setText(i18n("Create Hard Disk image"));
    a->setIcon(KIcon("hard-drive"));
    a->setShortcut(KStandardShortcut::openNew());
    a->setWhatsThis(i18n("Create a new Hard Disk image for later use."));

    KAction *b = actionCollection()->addAction("file_quit", this, SLOT(quit()));
    b->setText(i18n("Quit"));
    b->setIcon(KIcon("application-exit"));
    b->setShortcut(KStandardShortcut::quit());
    b->setWhatsThis(i18n("Close the application."));

    setupGUI();
    setAutoSaveSettings();

    setWindowIcon(KIcon("applications-engineering"));
    m_kemuui->groupBox->setEnabled(false);
    connect(m_kemuui->machinesList->listView()->selectionModel(),
        SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
        this, SLOT(machineChanged(QItemSelection,QItemSelection)));
    connect(m_kemuui->machinesList, SIGNAL(added(QString)), this, SLOT(addMachine(QString)));
    connect(m_kemuui->startStopButton, SIGNAL(clicked()), this, SLOT(startStopMachine()));
    connect(m_kemuui->machinesList, SIGNAL(removed(QString)), this, SLOT(removeMachine(QString)));

    static const QStringList qemuBins = QStringList() << "emu-system-aarch64"
        << "qemu-system-alpha"
        << "qemu-system-arm"
        << "qemu-system-cris"
        << "qemu-system-i386"
        << "qemu-system-lm32"
        << "qemu-system-m68k"
        << "qemu-system-microblaze"
        << "qemu-system-microblazeel"
        << "qemu-system-mips"
        << "qemu-system-mips64"
        << "qemu-system-mips64el"
        << "qemu-system-mipsel"
        << "qemu-system-moxie"
        << "qemu-system-or32"
        << "qemu-system-ppc"
        << "qemu-system-ppc64"
        << "qemu-system-ppcemb"
        << "qemu-system-s390x"
        << "qemu-system-sh4"
        << "qemu-system-sh4eb"
        << "qemu-system-sparc"
        << "qemu-system-sparc64"
        << "qemu-system-tricore"
        << "qemu-system-unicore32"
        << "qemu-system-xtensa"
        << "qemu-system-xtensaeb";
    foreach (const QString bin, qemuBins) {
        if(!KStandardDirs::findExe(bin).isEmpty()) {
            m_kemuui->systemComboBox->addItem(bin);
        }
    }

    m_settings = new QSettings("KEmu", "kemu");
    foreach(const QString machine, m_settings->childGroups()) {
        if (m_settings->value(machine + "/enable") == true) {
            m_kemuui->machinesList->insertItem(machine);
            machineLoad(machine);
        } else {
            kDebug() << "garbage machine" << machine;
        }
    }

    QFile kvmdev("/dev/kvm");
    if (!kvmdev.exists()) {
        const QString modprobeBin = KStandardDirs::findExe("modprobe");
        if (!modprobeBin.isEmpty()) {
            QProcess modprobe(this);
            modprobe.start(modprobeBin, QStringList() << "-b" << "kvm");
            modprobe.waitForFinished();
            if (!kvmdev.exists()) {
                QMessageBox::warning(this, i18n("KVM not available"), i18n("KVM not available"));
            }
        } else {
            kDebug() << "modprobe not found";
        }
    }

    connect(m_kemuui->CDROMInput, SIGNAL(textChanged(QString)), this, SLOT(machineSave(QString)));
    connect(m_kemuui->HardDriveInput, SIGNAL(textChanged(QString)), this, SLOT(machineSave(QString)));
    connect(m_kemuui->systemComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(machineSave(QString)));
    connect(m_kemuui->videoComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(machineSave(QString)));
    connect(m_kemuui->RAMInput, SIGNAL(valueChanged(int)), this, SLOT(machineSave(int)));
    connect(m_kemuui->KVMCheckBox, SIGNAL(stateChanged(int)), this, SLOT(machineSave(int)));
    connect(m_kemuui->argumentsLineEdit, SIGNAL(textChanged(QString)), this, SLOT(machineSave(QString)));
}

KEmuMainWindow::~KEmuMainWindow()
{
    saveAutoSaveSettings();
    m_settings->sync();
    delete m_settings;
    foreach(QProcess* machineProcess, m_machines) {
        const QString machine = m_machines.key(machineProcess);
        if (machineProcess->state() == QProcess::Running) {
            kDebug() << "stopping machine" << machine;
            machineProcess->terminate();
        }
        machineProcess->deleteLater();
        m_machines.remove(machine);
    }
}

void KEmuMainWindow::createHardDrive()
{
    // TODO: implement
    QMessageBox::warning(this, "NOT IMPLEMENTED", "NOT IMPLEMENTED");
}

void KEmuMainWindow::quit()
{
    qApp->quit();
}

void KEmuMainWindow::machineSave(const QString ignored)
{
    QString machine = m_kemuui->machinesList->currentText();
    kDebug() << "saving machine" << machine;
    m_settings->setValue(machine + "/cdrom", m_kemuui->CDROMInput->url().prettyUrl());
    m_settings->setValue(machine + "/harddrive", m_kemuui->HardDriveInput->url().prettyUrl());
    m_settings->setValue(machine + "/system", m_kemuui->systemComboBox->currentText());
    m_settings->setValue(machine + "/video", m_kemuui->videoComboBox->currentText());
    m_settings->setValue(machine + "/ram", m_kemuui->RAMInput->value());
    m_settings->setValue(machine + "/kvm", m_kemuui->KVMCheckBox->isChecked());
    m_settings->setValue(machine + "/args", m_kemuui->argumentsLineEdit->text());
    m_settings->sync();
}

void KEmuMainWindow::machineSave(int ignored)
{
    machineSave(QString());
}


void KEmuMainWindow::machineLoad(const QString machine)
{
    kDebug() << "loading machine" << machine;
    m_kemuui->CDROMInput->setUrl(m_settings->value(machine + "/cdrom").toUrl());
    m_kemuui->HardDriveInput->setUrl(m_settings->value(machine + "/harddrive").toUrl());
    const QString system = m_settings->value(machine + "/system").toString();
    const int systemIndex = m_kemuui->systemComboBox->findText(system);
    m_kemuui->systemComboBox->setCurrentIndex(systemIndex);
    const QString video = m_settings->value(machine + "/video", "virtio").toString();
    const int videoIndex = m_kemuui->videoComboBox->findText(video);
    m_kemuui->videoComboBox->setCurrentIndex(videoIndex);
    m_kemuui->RAMInput->setValue(m_settings->value(machine + "/ram", 32).toInt());
    m_kemuui->KVMCheckBox->setChecked(m_settings->value(machine + "/kvm", false).toBool());
    m_kemuui->argumentsLineEdit->setText(m_settings->value(machine + "/args").toString());
}

void KEmuMainWindow::machineChanged(QItemSelection ignored, QItemSelection ignored2)
{
    const QString machine = m_kemuui->machinesList->currentText();
    if (!machine.isEmpty()) {
        QFile kvmdev("/dev/kvm");
        m_kemuui->KVMCheckBox->setEnabled(kvmdev.exists());

        m_kemuui->startStopButton->setEnabled(true);
        m_kemuui->groupBox->setEnabled(true);
        if (m_machines.contains(machine)) {
            kDebug() << "machine is running" << machine;
            m_kemuui->startStopButton->setText(i18n("Stop"));
            m_kemuui->startStopButton->setIcon(KIcon("system-shutdown"));
        } else {
            m_kemuui->startStopButton->setText(i18n("Start"));
            m_kemuui->startStopButton->setIcon(KIcon("system-run"));
        }
        machineLoad(machine);
    } else {
        m_kemuui->startStopButton->setEnabled(false);
        m_kemuui->groupBox->setEnabled(false);
    }
}

void KEmuMainWindow::machineFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *machineProcess = qobject_cast<QProcess*>(sender());
    disconnect(machineProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
        this, SLOT(machineFinished(int,QProcess::ExitStatus)));
    if (exitCode != 0) {
        QMessageBox::warning(this, i18n("QEMU error"),
            i18n("An error occured:\n%1", QString(machineProcess->readAll())));
    }
    m_kemuui->startStopButton->setText(i18n("Start"));
    m_kemuui->startStopButton->setIcon(KIcon("system-run"));
    const QString machine = m_machines.key(machineProcess);
    m_machines.remove(machine);
    machineProcess->deleteLater();
}

void KEmuMainWindow::addMachine(const QString machine)
{
    m_settings->setValue(machine + "/enable", true);
    // TODO: set defaults?
    m_settings->sync();
}

void KEmuMainWindow::startStopMachine()
{
    const QString machine = m_kemuui->machinesList->currentText();
    if (!machine.isEmpty()) {
        if (m_machines.contains(machine)) {
            kDebug() << "stopping machine" << machine;
            QProcess* machineProcess = m_machines.take(machine);
            machineProcess->terminate();
            machineProcess->deleteLater();
            m_kemuui->startStopButton->setText(i18n("Start"));
            m_kemuui->startStopButton->setIcon(KIcon("system-run"));
        } else {
            kDebug() << "starting machine" << machine;
            QStringList machineArgs;
            const QString CDRom = m_kemuui->CDROMInput->url().prettyUrl();
            if (!CDRom.isEmpty()) {
                machineArgs << "-cdrom" << CDRom;
            }
            const QString HDrive = m_kemuui->HardDriveInput->url().prettyUrl();
            if (!HDrive.isEmpty()) {
                machineArgs << "-hda" << HDrive;
            }
            if (CDRom.isEmpty() && HDrive.isEmpty()) {
                QMessageBox::warning(this, i18n("Requirements not met"),
                    i18n("Either CD-ROM or Hard-Drive image must be set"));
                return;
            }
            machineArgs << "-vga" << m_kemuui->videoComboBox->currentText();
            machineArgs << "-m" << QByteArray::number(m_kemuui->RAMInput->value());
            if (m_kemuui->KVMCheckBox->isChecked()) {
                machineArgs << "-enable-kvm";
            }
            const QString extraArgs = m_kemuui->argumentsLineEdit->text();
            if (!extraArgs.isEmpty()) {
                foreach (const QString argument, extraArgs.split(" ")) {
                    machineArgs << argument;
                }
            }
            QProcess* machineProcess = new QProcess(this);
            machineProcess->setProcessChannelMode(QProcess::MergedChannels);
            m_kemuui->startStopButton->setText(i18n("Stop"));
            m_kemuui->startStopButton->setIcon(KIcon("system-shutdown"));
            m_machines.insert(machine, machineProcess);
            connect(machineProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
                    this, SLOT(machineFinished(int,QProcess::ExitStatus)));
            machineProcess->start(m_kemuui->systemComboBox->currentText(), machineArgs);
        }
    }
}

void KEmuMainWindow::removeMachine(const QString machine)
{
    if (m_machines.contains(machine)) {
        kDebug() << "stopping machine" << machine;
        QProcess* machineProcess = m_machines.take(machine);
        machineProcess->terminate();
        machineProcess->deleteLater();
    }
    kDebug() << "removing machine" << machine;
    m_settings->setValue(machine + "/enable", false);
    m_settings->sync();
}
