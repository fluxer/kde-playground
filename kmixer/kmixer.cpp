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

#include <KAboutData>
#include <KCmdLineArgs>
#include <KApplication>
#include <KUrl>
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

bool KSoundChannel::setPlaybackVolume(const int volume)
{
    return m_backend->setPlaybackVolume(this, volume);
}

int KSoundChannel::captureVolume() const
{
    return m_backend->captureVolume(this);
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

KAlsaBackend::KAlsaBackend(QObject *parent)
    : QObject(parent)
{
}

QList<KSoundCard> KAlsaBackend::soundCards()
{
    QList<KSoundCard> result;

    int alsacard = -1;
    int alsaresult = 0;
    while (true) {
        alsaresult = snd_card_next(&alsacard);
        if (alsaresult != 0) {
            kWarning() << "Could not get card" << snd_strerror(alsaresult);
            break;
        }
        if (alsacard < 0) {
            break;
        }

        QByteArray alsacardname = "hw:" + QByteArray::number(alsacard);
        snd_ctl_t *alsactl = nullptr;
        alsaresult = snd_ctl_open(&alsactl, alsacardname.constData(), SND_CTL_NONBLOCK);
        if (alsaresult != 0) {
            kWarning() << "Could not open card" << snd_strerror(alsaresult);
            break;
        }
        snd_ctl_card_info_t *alsacardinfo = nullptr;
        snd_ctl_card_info_alloca(&alsacardinfo);
        alsaresult = snd_ctl_card_info(alsactl, alsacardinfo);
        if (alsaresult != 0) {
            kWarning() << "Could not open card" << snd_strerror(alsaresult);
            break;
        }
        KSoundCard kcard;
        kcard.m_id = QString::fromLocal8Bit(snd_ctl_card_info_get_id(alsacardinfo));
        kcard.m_name = QString::fromLocal8Bit(snd_ctl_card_info_get_name(alsacardinfo));
        kcard.m_description = QString::fromLocal8Bit(snd_ctl_card_info_get_mixername(alsacardinfo));
        kDebug() << "Card" << kcard.m_id << kcard.m_name << kcard.m_description;
        snd_ctl_close(alsactl);

        snd_mixer_t *alsamixer = KAlsaBackend::mixerForCard(alsacard);
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
                    alsaresult = snd_mixer_selem_has_playback_channel(alsaelement, SND_MIXER_SCHN_FRONT_LEFT);
                    if (alsaresult == 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::FrontLeft;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_playback_channel(alsaelement, SND_MIXER_SCHN_FRONT_RIGHT);
                    if (alsaresult == 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::FrontRight;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_playback_channel(alsaelement, SND_MIXER_SCHN_REAR_LEFT);
                    if (alsaresult == 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::RearLeft;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_playback_channel(alsaelement, SND_MIXER_SCHN_REAR_RIGHT);
                    if (alsaresult == 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::RearRight;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_playback_channel(alsaelement, SND_MIXER_SCHN_FRONT_CENTER);
                    if (alsaresult == 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::FrontCenter;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_playback_channel(alsaelement, SND_MIXER_SCHN_WOOFER);
                    if (alsaresult == 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::Woofer;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_playback_channel(alsaelement, SND_MIXER_SCHN_SIDE_LEFT);
                    if (alsaresult == 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::SideLeft;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_playback_channel(alsaelement, SND_MIXER_SCHN_SIDE_RIGHT);
                    if (alsaresult == 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::SideRight;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_playback_channel(alsaelement, SND_MIXER_SCHN_REAR_CENTER);
                    if (alsaresult == 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::RearCenter;
                        kcard.m_channels.append(kchannel);
                    }
                } else if (hascapture) {
                    alsaresult = snd_mixer_selem_has_capture_channel(alsaelement, SND_MIXER_SCHN_FRONT_LEFT);
                    if (alsaresult == 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::FrontLeft;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_capture_channel(alsaelement, SND_MIXER_SCHN_FRONT_RIGHT);
                    if (alsaresult == 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::FrontRight;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_capture_channel(alsaelement, SND_MIXER_SCHN_REAR_LEFT);
                    if (alsaresult == 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::RearLeft;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_capture_channel(alsaelement, SND_MIXER_SCHN_REAR_RIGHT);
                    if (alsaresult == 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::RearRight;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_capture_channel(alsaelement, SND_MIXER_SCHN_FRONT_CENTER);
                    if (alsaresult == 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::FrontCenter;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_capture_channel(alsaelement, SND_MIXER_SCHN_WOOFER);
                    if (alsaresult == 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::Woofer;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_capture_channel(alsaelement, SND_MIXER_SCHN_SIDE_LEFT);
                    if (alsaresult == 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::SideLeft;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_capture_channel(alsaelement, SND_MIXER_SCHN_SIDE_RIGHT);
                    if (alsaresult == 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::SideRight;
                        kcard.m_channels.append(kchannel);
                    }
                    alsaresult = snd_mixer_selem_has_capture_channel(alsaelement, SND_MIXER_SCHN_REAR_CENTER);
                    if (alsaresult == 0) {
                        kchannel.m_type = KSoundChannel::KSoundChannelType::RearCenter;
                        kcard.m_channels.append(kchannel);
                    }
                } else {
                    kDebug() << "Neither playback nor capture element";
                    continue;
                }

                if (kchannel.m_type == KSoundChannel::KSoundChannelType::Unknown) {
                    kWarning() << "Unknown channel element";
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

int KAlsaBackend::playbackVolume(const KSoundChannel *channel) const
{
    if (!channel->hasPlayback()) {
        kDebug() << "Not playback channel";
        return 0;
    }

    snd_mixer_t* alsamixer = KAlsaBackend::mixerForCard(channel->cardID());
    if (!alsamixer) {
        return 0;
    }

    snd_mixer_elem_t *alsaelement = snd_mixer_first_elem(alsamixer);
    for (; alsaelement; alsaelement = snd_mixer_elem_next(alsaelement)) {
        if (!snd_mixer_elem_empty(alsaelement)) {
            const QString alsaid = QString::number(snd_mixer_selem_get_index(alsaelement));
            const QString alsaname = QString::fromLocal8Bit(snd_mixer_selem_get_name(alsaelement));
            if (alsaid == channel->id() && alsaname == channel->name()) {
                kDebug() << "Device" << channel->id() << channel->name();
                long alsaplaybackvolume = 0;
                const int alsaresult = snd_mixer_selem_get_playback_volume(alsaelement, SND_MIXER_SCHN_FRONT_LEFT, &alsaplaybackvolume);
                if (alsaresult != 0) {
                    kWarning() << "Could not get playback channel volume" << channel->id() << channel->name();
                    return 0;
                }
                return int(alsaplaybackvolume);
            }
        }
    }

    kWarning() << "Could not find playback channel" << channel->id() << channel->name();
    return 0;
}

bool KAlsaBackend::setPlaybackVolume(const KSoundChannel *channel, const int volume)
{
    snd_mixer_t* alsamixer = KAlsaBackend::mixerForCard(channel->cardID());
    if (!alsamixer) {
        return 0;
    }

    int alsaresult = 0;
    if (channel->type() == KSoundChannel::KSoundChannelType::FrontLeft) {
        snd_mixer_elem_t *alsaelement = snd_mixer_first_elem(alsamixer);
        for (; alsaelement; alsaelement = snd_mixer_elem_next(alsaelement)) {
            if (!snd_mixer_elem_empty(alsaelement)) {
                alsaresult = snd_mixer_selem_set_playback_volume(alsaelement, SND_MIXER_SCHN_FRONT_LEFT, long(volume));
                if (alsaresult != 0) {
                    kWarning() << "Could not set playback volume" << snd_strerror(alsaresult);;
                }
            }
        }
    } else if (channel->type() == KSoundChannel::KSoundChannelType::FrontRight) {
        snd_mixer_elem_t *alsaelement = snd_mixer_first_elem(alsamixer);
        for (; alsaelement; alsaelement = snd_mixer_elem_next(alsaelement)) {
            if (!snd_mixer_elem_empty(alsaelement)) {
                alsaresult = snd_mixer_selem_set_playback_volume(alsaelement, SND_MIXER_SCHN_FRONT_RIGHT, long(volume));
                if (alsaresult != 0) {
                    kWarning() << "Could not set playback volume" << snd_strerror(alsaresult);;
                }
            }
        }
    } else {
        // TODO: more channels
    }
    snd_mixer_close(alsamixer);

    return true;
}

int KAlsaBackend::captureVolume(const KSoundChannel *channel) const
{
    if (!channel->hasCapture()) {
        kDebug() << "Not capture channel";
        return 0;
    }

    snd_mixer_t* alsamixer = KAlsaBackend::mixerForCard(channel->cardID());
    if (!alsamixer) {
        return 0;
    }

    snd_mixer_elem_t *alsaelement = snd_mixer_first_elem(alsamixer);
    for (; alsaelement; alsaelement = snd_mixer_elem_next(alsaelement)) {
        if (!snd_mixer_elem_empty(alsaelement)) {
            const QString alsaid = QString::number(snd_mixer_selem_get_index(alsaelement));
            const QString alsaname = QString::fromLocal8Bit(snd_mixer_selem_get_name(alsaelement));
            if (alsaid == channel->id() && alsaname == channel->name()) {
                kDebug() << "Device" << channel->id() << channel->name();
                long alsacapturevolume = 0;
                const int alsaresult = snd_mixer_selem_get_capture_volume(alsaelement, SND_MIXER_SCHN_FRONT_LEFT, &alsacapturevolume);
                if (alsaresult != 0) {
                    kWarning() << "Could not get capture channel volume" << channel->id() << channel->name();
                    return 0;
                }
                return int(alsacapturevolume);
            }
        }
    }

    kWarning() << "Could not find capture channel" << channel->id() << channel->name();
    return 0;
}

bool KAlsaBackend::setCaptureVolume(const KSoundChannel *channel, const int volume)
{
    snd_mixer_t* alsamixer = KAlsaBackend::mixerForCard(channel->cardID());
    if (!alsamixer) {
        return 0;
    }

    int alsaresult = 0;
    if (channel->type() == KSoundChannel::KSoundChannelType::FrontLeft) {
        snd_mixer_elem_t *alsaelement = snd_mixer_first_elem(alsamixer);
        for (; alsaelement; alsaelement = snd_mixer_elem_next(alsaelement)) {
            if (!snd_mixer_elem_empty(alsaelement)) {
                alsaresult = snd_mixer_selem_set_capture_volume(alsaelement, SND_MIXER_SCHN_FRONT_LEFT, long(volume));
                if (alsaresult != 0) {
                    kWarning() << "Could not set playback volume" << snd_strerror(alsaresult);;
                }
            }
        }
    } else if (channel->type() == KSoundChannel::KSoundChannelType::FrontRight) {
        snd_mixer_elem_t *alsaelement = snd_mixer_first_elem(alsamixer);
        for (; alsaelement; alsaelement = snd_mixer_elem_next(alsaelement)) {
            if (!snd_mixer_elem_empty(alsaelement)) {
                alsaresult = snd_mixer_selem_set_capture_volume(alsaelement, SND_MIXER_SCHN_FRONT_RIGHT, long(volume));
                if (alsaresult != 0) {
                    kWarning() << "Could not set playback volume" << snd_strerror(alsaresult);;
                }
            }
        }
    } else {
        // TODO: more channels
    }
    snd_mixer_close(alsamixer);

    return true;
}

bool KAlsaBackend::mute(const KSoundChannel *channel) const
{
    // TODO: save volume and set it to 0
    return false;
}

bool KAlsaBackend::setMute(const KSoundChannel *channel, const bool mute)
{
    // TODO: restore volume
    return false;
}

bool KAlsaBackend::isAvailable()
{
    snd_mixer_t *alsamixer = nullptr;
    const int alsaresult = snd_mixer_open(&alsamixer, 0);
    if (alsaresult != 0) {
        kWarning() << "ALSA is not available" << snd_strerror(alsaresult);
        return false;
    }
    kDebug() << "ALSA is available";
    snd_mixer_close(alsamixer);
    return true;
}

snd_mixer_t* KAlsaBackend::mixerForCard(const int card)
{
    QByteArray alsacardname = "hw:" + QByteArray::number(card);
    snd_mixer_t *alsamixer = nullptr;
    int alsaresult = snd_mixer_open(&alsamixer, 0);
    if (alsaresult != 0) {
        kWarning() << "Could not open mixer" << snd_strerror(alsaresult);
        return nullptr;
    }
    alsaresult = snd_mixer_attach(alsamixer, alsacardname.constData());
    if (alsaresult != 0) {
        kWarning() << "Could not attach mixer" << snd_strerror(alsaresult);
        return nullptr;
    }
    alsaresult = snd_mixer_selem_register(alsamixer, nullptr, nullptr);
    if (alsaresult != 0) {
        kWarning() << "Could not register mixer" << snd_strerror(alsaresult);
        return nullptr;
    }
    alsaresult = snd_mixer_load(alsamixer);
    if (alsaresult != 0) {
        kWarning() << "Could not load mixer" << snd_strerror(alsaresult);
        return nullptr;
    }
    return alsamixer;
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
    delete m_backend;
}

bool KMixer::start(const QString &backend)
{
    if (backend == "alsa") {
        m_backend = new KAlsaBackend(this);
#if 1
        foreach (const KSoundCard &kcard, m_backend->soundCards()) {
            foreach (const KSoundChannel &kchannel, kcard.channels()) {
                qDebug() << kcard.description() << kcard.name() << kchannel.description();
                qDebug() << "Channel volume is" << kchannel.playbackVolume() << kchannel.captureVolume();
            }
        }
#endif
        return true;
    } else if (backend == "auto") {
        if (KAlsaBackend::isAvailable()) {
            m_backend = new KAlsaBackend(this);
#if 1
            foreach (const KSoundCard &kcard, m_backend->soundCards()) {
                foreach (const KSoundChannel &kchannel, kcard.channels()) {
                    qDebug() << kcard.description() << kcard.name() << kchannel.description();
                    qDebug() << "Channel volume is" << kchannel.playbackVolume() << kchannel.captureVolume();
                }
            }
#endif
            return true;
        }
        return false;
    }
    return false;
}

QString KMixer::errorString() const
{
    // TODO: error from backend
    return QString();
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
    qDebug() << "kmixer running, backend is" << args->getOption("backend");

    return kmixerapp->exec();
}

#include "moc_kmixer.cpp"
