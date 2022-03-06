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

#include <KAboutData>
#include <KCmdLineArgs>
#include <KApplication>
#include <KDebug>
#include <KAction>
#include <KActionCollection>
#include <KMenu>
#include <KIcon>
#include <KLocale>

KSoundChannel::KSoundChannel(KMixerBackend *backend)
    : m_backend(backend),
    m_type(KSoundChannelType::Unknown),
    m_playback(false),
    m_capture(false),
    m_cardid(-1)
{
}

KSoundChannel::KSoundChannel(const KSoundChannel &other)
{
    m_backend = other.m_backend;
    m_id = other.m_id;
    m_name = other.m_name;
    m_type = other.m_type;
    m_playback = other.m_playback;
    m_capture = other.m_capture;
    m_cardid = other.m_cardid;
}

KSoundChannel& KSoundChannel::operator=(const KSoundChannel &other)
{
    m_backend = other.m_backend;
    m_id = other.m_id;
    m_name = other.m_name;
    m_type = other.m_type;
    m_playback = other.m_playback;
    m_capture = other.m_capture;
    m_cardid = other.m_cardid;
    return *this;
}

KSoundChannel::KSoundChannelType KSoundChannel::type() const
{
    return m_type;
}

int KSoundChannel::cardID() const
{
    return m_cardid;
}

QString KSoundChannel::id() const
{
    return m_id;
}

QString KSoundChannel::name() const
{
    return m_name;
}

QString KSoundChannel::description() const
{
    switch (m_type) {
        case KSoundChannel::KSoundChannelType::Unknown: {
            return i18n("Unknown");
        }
        case KSoundChannel::KSoundChannelType::FrontLeft: {
            return i18n("Front Left");
        }
        case KSoundChannel::KSoundChannelType::FrontRight: {
            return i18n("Front Right");
        }
        case KSoundChannel::KSoundChannelType::RearLeft: {
            return i18n("Rear Left");
        }
        case KSoundChannel::KSoundChannelType::RearRight: {
            return i18n("Rear Right");
        }
        case KSoundChannel::KSoundChannelType::FrontCenter: {
            return i18n("Front Center");
        }
        case KSoundChannel::KSoundChannelType::Woofer: {
            return i18n("Woofer");
        }
        case KSoundChannel::KSoundChannelType::SideLeft: {
            return i18n("Side Left");
        }
        case KSoundChannel::KSoundChannelType::SideRight: {
            return i18n("Side Right");
        }
        case KSoundChannel::KSoundChannelType::RearCenter: {
            return i18n("Rear Center");
        }
    }
    return QString();
}

bool KSoundChannel::hasPlayback() const
{
    return m_playback;
}

bool KSoundChannel::hasCapture() const
{
    return m_capture;
}

int KSoundChannel::playbackVolume() const
{
    return m_backend->playbackVolume(this);
}

KVolumeRange KSoundChannel::playbackRange() const
{
    return m_backend->playbackRange(this);
}

bool KSoundChannel::setPlaybackVolume(const int volume)
{
    return m_backend->setPlaybackVolume(this, volume);
}

int KSoundChannel::captureVolume() const
{
    return m_backend->captureVolume(this);
}

KVolumeRange KSoundChannel::captureRange() const
{
    return m_backend->captureRange(this);
}

bool KSoundChannel::setCaptureVolume(const int volume)
{
    return m_backend->setCaptureVolume(this, volume);
}

KSoundCard::KSoundCard()
{
}

KSoundCard::KSoundCard(const KSoundCard &other)
{
    m_id = other.m_id;
    m_name = other.m_name;
    m_description = other.m_description;
    m_channels = other.m_channels;
}

KSoundCard& KSoundCard::operator=(const KSoundCard &other)
{
    m_id = other.m_id;
    m_name = other.m_name;
    m_description = other.m_description;
    m_channels = other.m_channels;
    return *this;
}

QString KSoundCard::name() const
{
    return m_name;
}

QString KSoundCard::description() const
{
    return m_description;
}

QList<KSoundChannel> KSoundCard::channels() const
{
    return m_channels;
}

KALSABackend::KALSABackend(QObject *parent)
    : QObject(parent),
    m_alsaresult(0)
{
}

QList<KSoundCard> KALSABackend::soundCards()
{
    QList<KSoundCard> result;

    int alsacard = -1;
    while (true) {
        m_alsaresult = snd_card_next(&alsacard);
        if (m_alsaresult != 0) {
            kWarning() << "Could not get card" << snd_strerror(m_alsaresult);
            break;
        }
        if (alsacard < 0) {
            break;
        }

        const QByteArray alsacardname = "hw:" + QByteArray::number(alsacard);
        snd_ctl_t *alsactl = nullptr;
        m_alsaresult = snd_ctl_open(&alsactl, alsacardname.constData(), SND_CTL_NONBLOCK);
        if (m_alsaresult != 0) {
            kWarning() << "Could not open card" << snd_strerror(m_alsaresult);
            break;
        }
        snd_ctl_card_info_t *alsacardinfo = nullptr;
        snd_ctl_card_info_alloca(&alsacardinfo);
        m_alsaresult = snd_ctl_card_info(alsactl, alsacardinfo);
        if (m_alsaresult != 0) {
            kWarning() << "Could not open card" << snd_strerror(m_alsaresult);
            break;
        }
        KSoundCard kcard;
        kcard.m_id = QString::fromLocal8Bit(snd_ctl_card_info_get_id(alsacardinfo));
        kcard.m_name = QString::fromLocal8Bit(snd_ctl_card_info_get_name(alsacardinfo));
        kcard.m_description = QString::fromLocal8Bit(snd_ctl_card_info_get_mixername(alsacardinfo));
        kDebug() << "Card" << kcard.m_id << kcard.m_name << kcard.m_description;
        snd_ctl_close(alsactl);

        snd_mixer_t *alsamixer = KALSABackend::mixerForCard(alsacard);
        if (!alsamixer) {
            break;
        }
    
        snd_mixer_elem_t *alsaelement = snd_mixer_first_elem(alsamixer);
        for (; alsaelement; alsaelement = snd_mixer_elem_next(alsaelement)) {
            if (!snd_mixer_elem_empty(alsaelement)) {
                const bool hasplayback = snd_mixer_selem_has_playback_volume(alsaelement);
                const bool hascapture = snd_mixer_selem_has_capture_volume(alsaelement);
                KSoundChannel kchannel(this);
                kchannel.m_id = QString::number(snd_mixer_selem_get_index(alsaelement));
                kchannel.m_name = QString::fromLocal8Bit(snd_mixer_selem_get_name(alsaelement));
                kchannel.m_type = KSoundChannel::KSoundChannelType::Unknown;
                kchannel.m_playback = hasplayback;
                kchannel.m_capture = hascapture;
                kchannel.m_cardid = alsacard;
                if (kchannel.m_playback) {
                    int alsaresult = snd_mixer_selem_has_playback_channel(alsaelement, SND_MIXER_SCHN_FRONT_LEFT);
                    if (alsaresult != 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::FrontLeft;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_playback_channel(alsaelement, SND_MIXER_SCHN_FRONT_RIGHT);
                    if (alsaresult != 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::FrontRight;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_playback_channel(alsaelement, SND_MIXER_SCHN_REAR_LEFT);
                    if (alsaresult != 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::RearLeft;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_playback_channel(alsaelement, SND_MIXER_SCHN_REAR_RIGHT);
                    if (alsaresult != 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::RearRight;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_playback_channel(alsaelement, SND_MIXER_SCHN_FRONT_CENTER);
                    if (alsaresult != 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::FrontCenter;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_playback_channel(alsaelement, SND_MIXER_SCHN_WOOFER);
                    if (alsaresult != 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::Woofer;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_playback_channel(alsaelement, SND_MIXER_SCHN_SIDE_LEFT);
                    if (alsaresult != 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::SideLeft;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_playback_channel(alsaelement, SND_MIXER_SCHN_SIDE_RIGHT);
                    if (alsaresult != 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::SideRight;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_playback_channel(alsaelement, SND_MIXER_SCHN_REAR_CENTER);
                    if (alsaresult != 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::RearCenter;
                        kcard.m_channels.append(kchannel);
                    }
                } else if (hascapture) {
                    int alsaresult = snd_mixer_selem_has_capture_channel(alsaelement, SND_MIXER_SCHN_FRONT_LEFT);
                    if (alsaresult != 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::FrontLeft;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_capture_channel(alsaelement, SND_MIXER_SCHN_FRONT_RIGHT);
                    if (alsaresult != 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::FrontRight;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_capture_channel(alsaelement, SND_MIXER_SCHN_REAR_LEFT);
                    if (alsaresult != 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::RearLeft;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_capture_channel(alsaelement, SND_MIXER_SCHN_REAR_RIGHT);
                    if (alsaresult != 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::RearRight;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_capture_channel(alsaelement, SND_MIXER_SCHN_FRONT_CENTER);
                    if (alsaresult != 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::FrontCenter;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_capture_channel(alsaelement, SND_MIXER_SCHN_WOOFER);
                    if (alsaresult != 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::Woofer;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_capture_channel(alsaelement, SND_MIXER_SCHN_SIDE_LEFT);
                    if (alsaresult != 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::SideLeft;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_capture_channel(alsaelement, SND_MIXER_SCHN_SIDE_RIGHT);
                    if (alsaresult != 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::SideRight;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_capture_channel(alsaelement, SND_MIXER_SCHN_REAR_CENTER);
                    if (alsaresult != 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::RearCenter;
                        kcard.m_channels.append(kchannel);
                    }
                } else {
                    kDebug() << "Neither playback nor capture element";
                    continue;
                }

                if (kchannel.m_type == KSoundChannel::KSoundChannelType::Unknown) {
                    kWarning() << "Unknown channel element type";
                    continue;
                }
                
                // qDebug() << "Channel" << kchannel.m_id << kchannel.m_name; << kchannel.m_type << kchannel.m_playback << kchannel.m_capture;
            }
        }
        snd_mixer_close(alsamixer);

        result.append(kcard);

        alsacard++;
    }
    
    return result;
}

int KALSABackend::playbackVolume(const KSoundChannel *channel) const
{
    if (!channel->hasPlayback()) {
        kDebug() << "Not playback channel";
        return 0;
    }

    snd_mixer_t* alsamixer = KALSABackend::mixerForCard(channel->cardID());
    if (!alsamixer) {
        return 0;
    }

    const snd_mixer_selem_channel_id_t alsachanneltype = KALSABackend::channelType(channel);
    if (alsachanneltype == SND_MIXER_SCHN_UNKNOWN) {
        kWarning() << "Could not determine channel type" << channel->id() << channel->name();
        snd_mixer_close(alsamixer);
        return 0;
    }

    snd_mixer_elem_t *alsaelement = snd_mixer_first_elem(alsamixer);
    for (; alsaelement; alsaelement = snd_mixer_elem_next(alsaelement)) {
        if (!snd_mixer_elem_empty(alsaelement)) {
            const QString alsaid = QString::number(snd_mixer_selem_get_index(alsaelement));
            const QString alsaname = QString::fromLocal8Bit(snd_mixer_selem_get_name(alsaelement));
            if (alsaid == channel->id() && alsaname == channel->name()) {
                kDebug() << "Channel" << channel->id() << channel->name();
                long alsaplaybackvolume = 0;
                m_alsaresult = snd_mixer_selem_get_playback_volume(alsaelement, alsachanneltype, &alsaplaybackvolume);
                if (m_alsaresult != 0) {
                    kWarning() << "Could not get playback channel volume" << snd_strerror(m_alsaresult);
                    snd_mixer_close(alsamixer);
                    return 0;
                }
                snd_mixer_close(alsamixer);
                return int(alsaplaybackvolume);
            }
        }
    }

    kWarning() << "Could not find playback channel" << channel->id() << channel->name();
    snd_mixer_close(alsamixer);
    return 0;
}

KVolumeRange KALSABackend::playbackRange(const KSoundChannel *channel) const
{
    KVolumeRange result = { 0, 0 };
    if (!channel->hasPlayback()) {
        kDebug() << "Not playback channel";
        return result;
    }

    snd_mixer_t* alsamixer = mixerForCard(channel->cardID());
    if (!alsamixer) {
        return result;
    }

    snd_mixer_elem_t *alsaelement = snd_mixer_first_elem(alsamixer);
    for (; alsaelement; alsaelement = snd_mixer_elem_next(alsaelement)) {
        if (!snd_mixer_elem_empty(alsaelement)) {
            const QString alsaid = QString::number(snd_mixer_selem_get_index(alsaelement));
            const QString alsaname = QString::fromLocal8Bit(snd_mixer_selem_get_name(alsaelement));
            if (alsaid == channel->id() && alsaname == channel->name()) {
                kDebug() << "Channel" << channel->id() << channel->name();
                long alsaplaybackvolumemin = 0;
                long alsaplaybackvolumemax = 0;
                m_alsaresult = snd_mixer_selem_get_playback_volume_range(alsaelement, &alsaplaybackvolumemin, &alsaplaybackvolumemax);
                if (m_alsaresult != 0) {
                    kWarning() << "Could not get playback channel volume range" << snd_strerror(m_alsaresult);
                    snd_mixer_close(alsamixer);
                    return result;
                }
                snd_mixer_close(alsamixer);
                result.minvolume = int(alsaplaybackvolumemin);
                result.maxvolume = int(alsaplaybackvolumemax);
                return result;
            }
        }
    }

    kWarning() << "Could not find playback channel" << channel->id() << channel->name();
    snd_mixer_close(alsamixer);
    return result;
}

bool KALSABackend::setPlaybackVolume(const KSoundChannel *channel, const int volume)
{
    if (!channel->hasPlayback()) {
        kDebug() << "Not playback channel";
        return false;
    }

    snd_mixer_t* alsamixer = mixerForCard(channel->cardID());
    if (!alsamixer) {
        return false;
    }

    const snd_mixer_selem_channel_id_t alsachanneltype = KALSABackend::channelType(channel);
    if (alsachanneltype == SND_MIXER_SCHN_UNKNOWN) {
        kWarning() << "Could not determine channel type" << channel->id() << channel->name();
        snd_mixer_close(alsamixer);
        return false;
    }

    snd_mixer_elem_t *alsaelement = snd_mixer_first_elem(alsamixer);
    for (; alsaelement; alsaelement = snd_mixer_elem_next(alsaelement)) {
        if (!snd_mixer_elem_empty(alsaelement)) {
            const QString alsaid = QString::number(snd_mixer_selem_get_index(alsaelement));
            const QString alsaname = QString::fromLocal8Bit(snd_mixer_selem_get_name(alsaelement));
            if (alsaid == channel->id() && alsaname == channel->name()) {
                m_alsaresult = snd_mixer_selem_set_playback_volume(alsaelement, alsachanneltype, long(volume));
                if (m_alsaresult != 0) {
                    kWarning() << "Could not set playback volume" << snd_strerror(m_alsaresult);
                    snd_mixer_close(alsamixer);
                    return false;
                }
                snd_mixer_close(alsamixer);
                return true;
            }
        }
    }

    kWarning() << "Could not find playback channel" << channel->id() << channel->name();
    snd_mixer_close(alsamixer);
    return false;
}

int KALSABackend::captureVolume(const KSoundChannel *channel) const
{
    if (!channel->hasCapture()) {
        kDebug() << "Not capture channel";
        return 0;
    }

    snd_mixer_t* alsamixer = mixerForCard(channel->cardID());
    if (!alsamixer) {
        return 0;
    }

    const snd_mixer_selem_channel_id_t alsachanneltype = KALSABackend::channelType(channel);
    if (alsachanneltype == SND_MIXER_SCHN_UNKNOWN) {
        kWarning() << "Could not determine channel type" << channel->id() << channel->name();
        snd_mixer_close(alsamixer);
        return 0;
    }
    
    snd_mixer_elem_t *alsaelement = snd_mixer_first_elem(alsamixer);
    for (; alsaelement; alsaelement = snd_mixer_elem_next(alsaelement)) {
        if (!snd_mixer_elem_empty(alsaelement)) {
            const QString alsaid = QString::number(snd_mixer_selem_get_index(alsaelement));
            const QString alsaname = QString::fromLocal8Bit(snd_mixer_selem_get_name(alsaelement));
            if (alsaid == channel->id() && alsaname == channel->name()) {
                kDebug() << "Channel" << channel->id() << channel->name();
                long alsacapturevolume = 0;
                m_alsaresult = snd_mixer_selem_get_capture_volume(alsaelement, alsachanneltype, &alsacapturevolume);
                if (m_alsaresult != 0) {
                    kWarning() << "Could not get capture channel volume" << snd_strerror(m_alsaresult);
                    snd_mixer_close(alsamixer);
                    return 0;
                }
                snd_mixer_close(alsamixer);
                return int(alsacapturevolume);
            }
        }
    }

    kWarning() << "Could not find capture channel" << channel->id() << channel->name();
    snd_mixer_close(alsamixer);
    return 0;
}

KVolumeRange KALSABackend::captureRange(const KSoundChannel *channel) const
{
    KVolumeRange result = { 0, 0 };
    if (!channel->hasCapture()) {
        kDebug() << "Not capture channel";
        return result;
    }

    snd_mixer_t* alsamixer = mixerForCard(channel->cardID());
    if (!alsamixer) {
        return result;
    }

    snd_mixer_elem_t *alsaelement = snd_mixer_first_elem(alsamixer);
    for (; alsaelement; alsaelement = snd_mixer_elem_next(alsaelement)) {
        if (!snd_mixer_elem_empty(alsaelement)) {
            const QString alsaid = QString::number(snd_mixer_selem_get_index(alsaelement));
            const QString alsaname = QString::fromLocal8Bit(snd_mixer_selem_get_name(alsaelement));
            if (alsaid == channel->id() && alsaname == channel->name()) {
                kDebug() << "Channel" << channel->id() << channel->name();
                long alsacapturevolumemin = 0;
                long alsacapturevolumemax = 0;
                m_alsaresult = snd_mixer_selem_get_capture_volume_range(alsaelement, &alsacapturevolumemin, &alsacapturevolumemax);
                if (m_alsaresult != 0) {
                    kWarning() << "Could not get capture channel volume range" << snd_strerror(m_alsaresult);
                    snd_mixer_close(alsamixer);
                    return result;
                }
                snd_mixer_close(alsamixer);
                result.minvolume = int(alsacapturevolumemin);
                result.maxvolume = int(alsacapturevolumemax);
                return result;
            }
        }
    }

    kWarning() << "Could not find capture channel" << channel->id() << channel->name();
    snd_mixer_close(alsamixer);
    return result;
}

bool KALSABackend::setCaptureVolume(const KSoundChannel *channel, const int volume)
{
    if (!channel->hasCapture()) {
        kDebug() << "Not capture channel";
        return false;
    }

    snd_mixer_t* alsamixer = mixerForCard(channel->cardID());
    if (!alsamixer) {
        return false;
    }

    const snd_mixer_selem_channel_id_t alsachanneltype = KALSABackend::channelType(channel);
    if (alsachanneltype == SND_MIXER_SCHN_UNKNOWN) {
        kWarning() << "Could not determine channel type" << channel->id() << channel->name();
        snd_mixer_close(alsamixer);
        return false;
    }

    snd_mixer_elem_t *alsaelement = snd_mixer_first_elem(alsamixer);
    for (; alsaelement; alsaelement = snd_mixer_elem_next(alsaelement)) {
        if (!snd_mixer_elem_empty(alsaelement)) {
            const QString alsaid = QString::number(snd_mixer_selem_get_index(alsaelement));
            const QString alsaname = QString::fromLocal8Bit(snd_mixer_selem_get_name(alsaelement));
            if (alsaid == channel->id() && alsaname == channel->name()) {
                m_alsaresult = snd_mixer_selem_set_capture_volume(alsaelement, alsachanneltype, long(volume));
                if (m_alsaresult != 0) {
                    kWarning() << "Could not set capture volume" << snd_strerror(m_alsaresult);
                    snd_mixer_close(alsamixer);
                    return false;
                }
                snd_mixer_close(alsamixer);
                return true;
            }
        }
    }

    kWarning() << "Could not find capture channel" << channel->id() << channel->name();
    snd_mixer_close(alsamixer);
    return false;
}

bool KALSABackend::isAvailable() const
{
    snd_mixer_t *alsamixer = nullptr;
    m_alsaresult = snd_mixer_open(&alsamixer, 0);
    if (m_alsaresult != 0) {
        kWarning() << "ALSA is not available" << snd_strerror(m_alsaresult);
        return false;
    }
    kDebug() << "ALSA is available";
    snd_mixer_close(alsamixer);
    return true;
}

QString KALSABackend::errorString() const
{
    // TODO: translate errors
    return QString::fromLocal8Bit(snd_strerror(m_alsaresult));
}

snd_mixer_t* KALSABackend::mixerForCard(const int card) const
{
    const QByteArray alsacardname = "hw:" + QByteArray::number(card);
    snd_mixer_t *alsamixer = nullptr;
    m_alsaresult = snd_mixer_open(&alsamixer, 0);
    if (m_alsaresult != 0) {
        kWarning() << "Could not open mixer" << snd_strerror(m_alsaresult);
        return nullptr;
    }
    m_alsaresult = snd_mixer_attach(alsamixer, alsacardname.constData());
    if (m_alsaresult != 0) {
        kWarning() << "Could not attach mixer" << snd_strerror(m_alsaresult);
        snd_mixer_close(alsamixer);
        return nullptr;
    }
    m_alsaresult = snd_mixer_selem_register(alsamixer, nullptr, nullptr);
    if (m_alsaresult != 0) {
        kWarning() << "Could not register mixer" << snd_strerror(m_alsaresult);
        snd_mixer_close(alsamixer);
        return nullptr;
    }
    m_alsaresult = snd_mixer_load(alsamixer);
    if (m_alsaresult != 0) {
        kWarning() << "Could not load mixer" << snd_strerror(m_alsaresult);
        snd_mixer_close(alsamixer);
        return nullptr;
    }
    return alsamixer;
}

snd_mixer_selem_channel_id_t KALSABackend::channelType(const KSoundChannel *channel)
{
    switch (channel->type()) {
        case KSoundChannel::KSoundChannelType::Unknown: {
            return SND_MIXER_SCHN_UNKNOWN;
        }
        case KSoundChannel::KSoundChannelType::FrontLeft: {
            return SND_MIXER_SCHN_FRONT_LEFT;
        }
        case KSoundChannel::KSoundChannelType::FrontRight: {
            return SND_MIXER_SCHN_FRONT_RIGHT;
        }
        case KSoundChannel::KSoundChannelType::RearLeft: {
            return SND_MIXER_SCHN_REAR_LEFT;
        }
        case KSoundChannel::KSoundChannelType::RearRight: {
            return SND_MIXER_SCHN_REAR_RIGHT;
        }
        case KSoundChannel::KSoundChannelType::FrontCenter: {
            return SND_MIXER_SCHN_FRONT_CENTER;
        }
        case KSoundChannel::KSoundChannelType::Woofer: {
            return SND_MIXER_SCHN_WOOFER;
        }
        case KSoundChannel::KSoundChannelType::SideLeft: {
            return SND_MIXER_SCHN_SIDE_LEFT;
        }
        case KSoundChannel::KSoundChannelType::SideRight: {
            return SND_MIXER_SCHN_SIDE_RIGHT;
        }
        case KSoundChannel::KSoundChannelType::RearCenter: {
            return SND_MIXER_SCHN_REAR_CENTER;
        }
    }
    return SND_MIXER_SCHN_UNKNOWN;
}

KMixer::KMixer(QObject *parent)
    : QObject(parent),
    m_backend(nullptr),
    m_statusitem(nullptr),
    m_backendaction(nullptr)
{
    m_statusitem = new KStatusNotifierItem(this);
    m_statusitem->setObjectName(QLatin1String("kmixer"));
    m_statusitem->setCategory(KStatusNotifierItem::SystemServices);
    m_statusitem->setStatus(KStatusNotifierItem::Passive);
    m_statusitem->setIconByName(QLatin1String("multimedia-volume-control"));
    m_statusitem->setToolTip(QLatin1String("tooltip"), i18n("KMixer"), i18n("Test."));

    m_backendaction = m_statusitem->actionCollection()->addAction(QLatin1String("select_backend"));
    m_backendaction->setText(i18n("Select &backend"));
    m_backendaction->setIcon(KIcon(QLatin1String("configure")));
    connect(m_backendaction, SIGNAL(triggered()), this, SLOT(slotBackend()));
    m_statusitem->contextMenu()->addAction(m_backendaction);
}

KMixer::~KMixer()
{
    // backend parented to this
}

bool KMixer::start(const QString &backend)
{
    if (backend == "alsa") {
        m_backend = new KALSABackend(this);
        return m_backend->isAvailable();
    } else if (backend == "auto") {
        m_backend = new KALSABackend(this);
        return m_backend->isAvailable();
    }
    return false;
}

QString KMixer::errorString() const
{
    if (!m_backend) {
        return i18n("No backend");
    }
    return m_backend->errorString();
}

QList<KSoundCard> KMixer::soundCards() const
{
    if (!m_backend) {
        return QList<KSoundCard>();
    }
    return m_backend->soundCards();
}

void KMixer::slotBackend()
{
    qApp->quit();
}

int main(int argc, char** argv)
{
    KAboutData aboutData("kmixer", 0, ki18n("KMixer"),
                         "1.0.0", ki18n("Control the system audio devices volume."),
                         KAboutData::License_GPL_V2,
                         ki18n("(c) 2022 Ivailo Monev"),
                         KLocalizedString(),
                        "http://github.com/fluxer/katana"
                        );

    aboutData.addAuthor(ki18n("Ivailo Monev"),
                        ki18n("Maintainer"),
                        "xakepa10@gmail.com");
    aboutData.setProgramIconName(QLatin1String("multimedia-volume-control"));

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineOptions option;
    option.add("backend <auto>", ki18n("Backend to use, default value"), "auto");
    KCmdLineArgs::addCmdLineOptions(option);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KApplication *kmixerapp = new KApplication();

    KMixer kmixer(kmixerapp);
    if (!kmixer.start(args->getOption("backend"))) {
        kWarning() << kmixer.errorString();
        return 1;
    }
    KMixerWindow kmixerwin(nullptr, &kmixer);
    qDebug() << "kmixer running, backend is" << args->getOption("backend");

    return kmixerapp->exec();
}

#include "moc_kmixer.cpp"
