/***************************************************************************
 *   Copyright (C) by Simon Persson                                        *
 *   simonop@spray.se                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef MPRIS2PLAYER_H
#define MPRIS2PLAYER_H
#include <QVariantMap>
#include <QDBusObjectPath>
#include <QStringList>

class Mpris2Player: public QObject
{
	Q_OBJECT

public:
	Mpris2Player(QObject *pParent);

public: // Properties for MediaPlayer2 interface.
	Q_PROPERTY(bool CanQuit READ canQuit)
	bool canQuit() const;

	Q_PROPERTY(bool CanRaise READ canRaise)
	bool canRaise() const;

	Q_PROPERTY(QString DesktopEntry READ desktopEntry)
	QString desktopEntry() const;

	Q_PROPERTY(bool HasTrackList READ hasTrackList)
	bool hasTrackList() const;

	Q_PROPERTY(QString Identity READ identity)
	QString identity() const;

	Q_PROPERTY(QStringList SupportedMimeTypes READ supportedMimeTypes)
	QStringList supportedMimeTypes() const;

	Q_PROPERTY(QStringList SupportedUriSchemes READ supportedUriSchemes)
	QStringList supportedUriSchemes() const;

public: // Properties for MediaPlayer2.Player interface.
	Q_PROPERTY(bool CanControl READ canControl)
	bool canControl() const;

	Q_PROPERTY(bool CanGoNext READ canGoNext)
	bool canGoNext() const;

	Q_PROPERTY(bool CanGoPrevious READ canGoPrevious)
	bool canGoPrevious() const;

	Q_PROPERTY(bool CanPause READ canPause)
	bool canPause() const;

	Q_PROPERTY(bool CanPlay READ canPlay)
	bool canPlay() const;

	Q_PROPERTY(bool CanSeek READ canSeek)
	bool canSeek() const;

	Q_PROPERTY(QString LoopStatus READ loopStatus WRITE setLoopStatus)
	QString loopStatus() const;
	void setLoopStatus(const QString &value);

	Q_PROPERTY(double MaximumRate READ maximumRate)
	double maximumRate() const;

	Q_PROPERTY(QVariantMap Metadata READ metadata)
	QVariantMap metadata() const;

	Q_PROPERTY(double MinimumRate READ minimumRate)
	double minimumRate() const;

	Q_PROPERTY(QString PlaybackStatus READ playbackStatus)
	QString playbackStatus() const;

	Q_PROPERTY(qlonglong Position READ position)
	qlonglong position() const;

	Q_PROPERTY(double Rate READ rate WRITE setRate)
	double rate() const;
	void setRate(double value);

	Q_PROPERTY(bool Shuffle READ shuffle WRITE setShuffle)
	bool shuffle() const;
	void setShuffle(bool value);

	Q_PROPERTY(double Volume READ volume WRITE setVolume)
	double volume() const;
	void setVolume(double value);

public slots: //Slots for MediaPlayer2 interface
	void Quit();
	void Raise();

public Q_SLOTS: // Slots for MediaPlayer2.Player interface.
	void Next();
	void OpenUri(const QString &Uri);
	void Pause();
	void Play();
	void PlayPause();
	void Previous();
	void Seek(qlonglong Offset);
	void SetPosition(const QDBusObjectPath &TrackId, qlonglong Position);
	void Stop();

Q_SIGNALS: //Signals for MediaPlayer2.Player interface.
	void Seeked(qlonglong Position);

public:
	void notifyChangedProperty(const QLatin1String &pPropertyName);

public slots:
	void openPLS(const QString &pPath);
	void openM3U(const QString &pPath);
	void notifySeeked(qlonglong pTimeInUs);

protected:
	uint extractIndex(const QString &pStr);
};

#endif // MPRIS2PLAYER_H
