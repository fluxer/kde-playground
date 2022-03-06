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

#ifndef KMIXER_H
#define KMIXER_H

#include <QString>
#include <QList>
#include <QAction>
#include <KStatusNotifierItem>

#include <alsa/asoundlib.h>

class KMixerBackend;

struct KVolumeRange {
    int minvolume;
    int maxvolume;
};

class KSoundChannel {
public:
    enum KSoundChannelType {
        Unknown = 0,
        FrontLeft = 1,
        FrontRight = 2,
        RearLeft = 3,
        RearRight = 4,
        FrontCenter = 5,
        Woofer = 6,
        SideLeft = 7,
        SideRight = 8,
        RearCenter = 9
    };

    KSoundChannel(KMixerBackend *backend);

    KSoundChannel(const KSoundChannel &other);
    KSoundChannel &operator=(const KSoundChannel &other);

    KSoundChannelType type() const;
    int cardID() const;
    QString id() const;
    QString name() const;
    QString description() const;

    bool hasPlayback() const;
    bool hasCapture() const;

    int playbackVolume() const;
    KVolumeRange playbackRange() const;
    bool setPlaybackVolume(const int volume);

    int captureVolume() const;
    KVolumeRange captureRange() const;
    bool setCaptureVolume(const int volume);

private:
    KMixerBackend* m_backend;
    QString m_id;
    QString m_name;
    KSoundChannelType m_type;
    bool m_playback;
    bool m_capture;
    int m_cardid;

    friend class KALSABackend;
};

class KSoundCard {
public:
    KSoundCard();

    KSoundCard(const KSoundCard &other);
    KSoundCard &operator=(const KSoundCard &other);

    QString name() const;
    QString description() const;

    QList<KSoundChannel> channels() const;

private:
    QString m_id;
    QString m_name;
    QString m_description;
    QList<KSoundChannel> m_channels;

    friend class KALSABackend;
};

class KMixerBackend
{
public:
    virtual QList<KSoundCard> soundCards() = 0;

    virtual int playbackVolume(const KSoundChannel *channel) const = 0;
    virtual KVolumeRange playbackRange(const KSoundChannel *channel) const = 0;
    virtual bool setPlaybackVolume(const KSoundChannel *channel, const int volume) = 0;

    virtual int captureVolume(const KSoundChannel *channel) const = 0;
    virtual KVolumeRange captureRange(const KSoundChannel *channel) const = 0;
    virtual bool setCaptureVolume(const KSoundChannel *channel, const int volume) = 0;

    virtual bool mute(const KSoundChannel *channel) const = 0;
    virtual bool setMute(const KSoundChannel *channel, const bool mute) = 0;
};

class KALSABackend : public QObject, public KMixerBackend
{
    Q_OBJECT
public:
    KALSABackend(QObject *parent);

    QList<KSoundCard> soundCards() final;
    
    int playbackVolume(const KSoundChannel *channel) const final;
    KVolumeRange playbackRange(const KSoundChannel *channel) const final;
    bool setPlaybackVolume(const KSoundChannel *channel, const int volume) final;

    int captureVolume(const KSoundChannel *channel) const final;
    KVolumeRange captureRange(const KSoundChannel *channel) const final;
    bool setCaptureVolume(const KSoundChannel *channel, const int volume) final;

    bool mute(const KSoundChannel *channel) const final;
    bool setMute(const KSoundChannel *channel, const bool mute) final;

    static bool isAvailable();
    
private:
    static snd_mixer_t* mixerForCard(const int card);
    static snd_mixer_selem_channel_id_t channelType(const KSoundChannel *channel);
};

class KMixer : public QObject
{
    Q_OBJECT
public:
    KMixer(QObject *parent = nullptr);
    ~KMixer();

    bool start(const QString &backend);
    QString errorString() const;

public Q_SLOTS:
    void slotBackend();

private:
    KMixerBackend *m_backend;
    KStatusNotifierItem *m_statusitem;
    QAction *m_backendaction;
};

#endif // KMIXER_H
