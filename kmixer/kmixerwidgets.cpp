/*  This file is part of KMixer
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

#include "kmixer.h"
#include "kmixerwidgets.h"

#include <KDebug>

KVolumeWidget::KVolumeWidget(QWidget *parent)
    : QWidget(parent)
{
}

KVolumeWidget::~KVolumeWidget()
{
}

KSoundDeviceWidget::KSoundDeviceWidget(QWidget *parent)
    : QWidget(parent)
{
}

KSoundDeviceWidget::~KSoundDeviceWidget()
{
}

KSoundCardWidget::KSoundCardWidget(QTabWidget *parent)
    : QTabWidget(parent)
{
}

KSoundCardWidget::~KSoundCardWidget()
{
}

KMixerWindow::KMixerWindow(QWidget *parent, const KMixer *mixer)
    : QMainWindow(parent),
    m_mixer(mixer)
{
#if 1
    foreach (const KSoundCard &kcard, m_mixer->soundCards()) {
        foreach (KSoundChannel kchannel, kcard.channels()) {
            qDebug() << kcard.name() << kcard.description() << kchannel.name() << kchannel.description();
            qDebug() << "Channel volume is" << kchannel.playbackVolume() << kchannel.captureVolume();
            kchannel.setPlaybackVolume(10);
            kchannel.setCaptureVolume(10);
            qDebug() << "Channel volume is" << kchannel.playbackVolume() << kchannel.captureVolume();
            qDebug() << "Channel volume range is" << kchannel.playbackRange().minvolume << kchannel.playbackRange().maxvolume;
        }
    }
#endif
}

KMixerWindow::~KMixerWindow()
{
}

#include "moc_kmixerwidgets.cpp"
