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
#include "mpris2player.h"

#include "bbmainwindow.h"
#include "bbstreamstab.h"
#include "bbfilesystemtab.h"

#include <phonon/audiooutput.h>
#include <phonon/seekslider.h>
#include <phonon/mediaobject.h>
#include <phonon/volumeslider.h>
#include <KUrl>
#include <KMessageBox>
#include <KMimeType>
#include <KIO/NetAccess>
#include <QFile>

Mpris2Player::Mpris2Player(QObject *pParent)
   : QObject(pParent)
{
}

bool Mpris2Player::canQuit() const
{
	return true;
}

bool Mpris2Player::canRaise() const
{
	return true;
}

QString Mpris2Player::desktopEntry() const
{
	return QString::fromAscii("boombox");
}

bool Mpris2Player::hasTrackList() const
{
	return false;
}

QString Mpris2Player::identity() const
{
	return QString::fromAscii("BoomBox");
}

QStringList Mpris2Player::supportedMimeTypes() const
{
	QString lList = QString::fromAscii("application/x-ogg;audio/basic;audio/vnd.rn-realaudio;"
	                                   "audio/x-aiff;audio/x-flac;audio/x-matroska;audio/x-mp3;"
	                                   "audio/mpeg;audio/ogg;audio/x-flac+ogg;audio/x-vorbis+ogg;"
	                                   "audio/x-mpegurl;audio/x-ms-wma;audio/x-pn-realaudio;"
	                                   "audio/x-scpls;audio/x-wav");
	return lList.split(';');
}

QStringList Mpris2Player::supportedUriSchemes() const
{
	QStringList lList;
	lList << "http://" << "file://";
	return lList;
}

void Mpris2Player::Quit()
{
	gMainWindow->close();
}

void Mpris2Player::Raise()
{
	gMainWindow->activateWindow();
	gMainWindow->raise();
}





bool Mpris2Player::canControl() const
{
	return true;
}

bool Mpris2Player::canGoNext() const
{
	return true;
}

bool Mpris2Player::canGoPrevious() const
{
	return true;
}

bool Mpris2Player::canPause() const
{
	return true;
}

bool Mpris2Player::canPlay() const
{
	return true;
}

bool Mpris2Player::canSeek() const
{
	return gMainWindow->mMediaObject->isSeekable();
}

QString Mpris2Player::loopStatus() const
{
	return QString::fromAscii("Playlist");
}

void Mpris2Player::setLoopStatus(const QString &value)
{
	Q_UNUSED(value)
}

double Mpris2Player::maximumRate() const
{
	return 1.0;
}

QVariantMap Mpris2Player::metadata() const
{
	QMultiMap<QString, QString> lMap = gMainWindow->mMediaObject->metaData();
	QVariantMap lMap2;
	QString lUrl = gMainWindow->mMediaObject->currentSource().url().toString();
	if(lUrl.isEmpty()) {
		lUrl = "org/mpris/MediaPlayer2/TrackList/NoTrack";
	} else {
		lUrl.prepend("boombox/");
	}
	lMap2.insert("mpris:trackid", lUrl);
	lMap2.insert("mpris:length", gMainWindow->mMediaObject->totalTime() * 1000);
	lMap2.insert("xesam:artist", QStringList(lMap.value("ARTIST")));
	lMap2.insert("xesam:album", lMap.value("ALBUM"));
	lMap2.insert("xesam:title", lMap.value("TITLE"));
	lMap2.insert("xesam:genre", QStringList(lMap.value("GENRE")));
	return lMap2;
}

double Mpris2Player::minimumRate() const
{
	return 1.0;
}

QString Mpris2Player::playbackStatus() const
{
	Phonon::State lState = gMainWindow->mMediaObject->state();
	if(lState == Phonon::PlayingState || lState == Phonon::BufferingState)
		return "Playing";
	else if(lState == Phonon::PausedState)
		return "Paused";
	else
		return "Stopped";
}

qlonglong Mpris2Player::position() const
{
	return gMainWindow->mMediaObject->currentTime() * 1000; // return position in microseconds
}

double Mpris2Player::rate() const
{
	return 1.0;
}

void Mpris2Player::setRate(double value)
{
	Q_UNUSED(value)
}

bool Mpris2Player::shuffle() const
{
	return gMainWindow->shuffleActive();
}

void Mpris2Player::setShuffle(bool value)
{
	gMainWindow->setShuffle(value);
}

double Mpris2Player::volume() const
{
	return gMainWindow->mAudioOutput->volume();
}

void Mpris2Player::setVolume(double value)
{
	gMainWindow->mAudioOutput->setVolume(value);
}

void Mpris2Player::Next()
{
	gMainWindow->jumpToNextSong();
}

void Mpris2Player::OpenUri(const QString &Uri)
{
	KUrl lUrl(Uri);
	KMimeType::Ptr lMimeType = KMimeType::findByUrl(lUrl);
	if(lMimeType->is("application/octet-stream"))
	{
		lMimeType = KMimeType::mimeType(KIO::NetAccess::mimetype(lUrl, gMainWindow));
	}
	if(lMimeType.isNull())
		return;

	if(lMimeType->is("audio/x-mpegurl") || lMimeType->is("audio/x-scpls"))
	{
		QString lTmpFile;
		if(KIO::NetAccess::download(lUrl, lTmpFile, gMainWindow))
		{
			if(lMimeType->is("audio/x-mpegurl"))
				openM3U(lTmpFile);
			else if(lMimeType->is("audio/x-scpls"))
				openPLS(lTmpFile);
			KIO::NetAccess::removeTempFile(lTmpFile);
		}
		else
			KMessageBox::error(gMainWindow, KIO::NetAccess::lastErrorString());
		return;
	}

	if(lUrl.protocol() == "http")
		gMainWindow->mStreamsTab->addManualUrl(lUrl, lUrl.prettyUrl());
	else
		gMainWindow->mFileSystemTab->addManualUrl(lUrl);
}

void Mpris2Player::Pause()
{
	Phonon::State lState = gMainWindow->mMediaObject->state();
	if(lState == Phonon::PlayingState || lState == Phonon::BufferingState)
		gMainWindow->togglePlayback();
}

void Mpris2Player::Play()
{
	Phonon::State lState = gMainWindow->mMediaObject->state();
	if(lState != Phonon::PlayingState && lState != Phonon::BufferingState)
		gMainWindow->togglePlayback();
}

void Mpris2Player::PlayPause()
{
	gMainWindow->togglePlayback();
}

void Mpris2Player::Previous()
{
	gMainWindow->jumpToPreviousSong();
}

void Mpris2Player::Seek(qlonglong Offset)
{
	gMainWindow->mMediaObject->seek(gMainWindow->mMediaObject->currentTime() + Offset * 1000);
}

void Mpris2Player::SetPosition(const QDBusObjectPath &TrackId, qlonglong Position)
{
	if(!canSeek() || TrackId == QDBusObjectPath("org/mpris/MediaPlayer2/TrackList/NoTrack") ||
	      Position < 0 || Position > gMainWindow->mMediaObject->totalTime() * 1000) {
		return;
	}
	gMainWindow->mMediaObject->seek(Position/1000);
}

void Mpris2Player::Stop()
{
	Phonon::State lState = gMainWindow->mMediaObject->state();
	if(lState == Phonon::PlayingState || lState == Phonon::BufferingState)
		gMainWindow->togglePlayback();
	gMainWindow->mMediaObject->seek(0);
}

void Mpris2Player::openM3U(const QString &pPath)
{
	QFile lFile(pPath);
	QString lReadPath;
	QFileInfo lInfo(pPath);

	QString lCurrentPath = lInfo.dir().absolutePath();
	if(!lFile.open(QIODevice::ReadOnly | QIODevice::Text))
		return;
	QTextStream lTextStream(&lFile);

	while(!lTextStream.atEnd())
	{
		lReadPath = lTextStream.readLine(4096).trimmed();
		if(!lReadPath.startsWith('#'))
		{
			KUrl lUrl(lReadPath);
			if(lUrl.protocol() == "http")
				gMainWindow->mStreamsTab->addManualUrl(lUrl, lUrl.prettyUrl());
			else
			{
				if(lUrl.isRelative())
					lUrl = KUrl(lCurrentPath, lReadPath);
				gMainWindow->mFileSystemTab->addManualUrl(lUrl);
			}
		}
	}
}

void Mpris2Player::notifySeeked(qlonglong pTimeInUs) {
	emit Seeked(pTimeInUs);
}

void Mpris2Player::openPLS(const QString &pPath)
{
	QFile file(pPath);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	QTextStream stream(&file);

	// Better Handling of pls playlists - Taken from amaroK - amarok.kde.org
	// Counted number of "File#=" lines.
	uint entryCnt = 0;
	// Value of the "NumberOfEntries=#" line.
	uint numberOfEntries = 0;
	bool havePlaylistSection = false;
	QString tmp;
	QStringList lines;

	const QRegExp regExp_NumberOfEntries("^NumberOfEntries\\s*=\\s*\\d+$", Qt::CaseInsensitive);
	const QRegExp regExp_File("^File\\d+\\s*=", Qt::CaseInsensitive);
	const QRegExp regExp_Title("^Title\\d+\\s*=", Qt::CaseInsensitive);
	const QRegExp regExp_Length("^Length\\d+\\s*=\\s*-?\\d+$", Qt::CaseInsensitive);
	const QRegExp regExp_Version("^Version\\s*=\\s*\\d+$", Qt::CaseInsensitive);
	const QString section_playlist("[playlist]");

	/* Preprocess the input data.
	* Read the lines into a buffer; Cleanup the line strings;
	* Count the entries manually and read "NumberOfEntries".
	*/
	while (!stream.atEnd()) {
		tmp = stream.readLine(4096).simplified();
		if (tmp.isEmpty())
			continue;
		lines.append(tmp);

		if (tmp == section_playlist) {
			havePlaylistSection = true;
			continue;
		}

		if (tmp.contains(regExp_File)) {
			entryCnt++;
			continue;
		}

		if (tmp.contains(regExp_NumberOfEntries)) {
			numberOfEntries = tmp.section('=', -1).simplified().toUInt();
			continue;
		}
	}
	if (numberOfEntries != entryCnt)
		numberOfEntries = entryCnt;

	if(numberOfEntries == 0)
		return;

	uint index;
	bool inPlaylistSection = false;
	QString* files = new QString[entryCnt];
	QString *titles = new QString[entryCnt];

	QStringList::const_iterator i = lines.begin(), end = lines.end();
	for( ; i != end; ++i)
	{
		if (!inPlaylistSection && havePlaylistSection) {
			/* The playlist begins with the "[playlist]" tag.
			 * Skip everything before this.
			 */
			if ((*i) == section_playlist)
				inPlaylistSection = true;
			continue;
		}
		if ((*i).contains(regExp_File)) {
			// Have a "File#=XYZ" line.
			index = extractIndex(*i);
			if (index <= numberOfEntries && index != 0)
				files[index-1] = (*i).section('=', 1).trimmed();
		}
		if ((*i).contains(regExp_Title)) {
			// Have a "Title#=XYZ" line.
			index = extractIndex(*i);
			if (index <= numberOfEntries && index != 0)
				titles[index-1] = (*i).section('=', 1).trimmed();
		}
	}

	QFileInfo lInfo(pPath);
	QString lCurrentPath = lInfo.dir().absolutePath();
	for (uint i=0; i<entryCnt; i++)
	{
		if(files[i].isEmpty())
			continue;
		KUrl lUrl(files[i]);

		if(lUrl.protocol() == "http")
			gMainWindow->mStreamsTab->addManualUrl(lUrl, titles[i]);
		else
		{
			if(lUrl.isRelative())
				lUrl = KUrl(lCurrentPath, files[i]);
			gMainWindow->mFileSystemTab->addManualUrl(lUrl);
		}
	}
	delete[] files;
	delete[] titles;
}

uint Mpris2Player::extractIndex(const QString &pStr)
{
	/* Extract the index number out of a .pls line.
	 * Example:
	*   extractIndex("File2=foobar") == 2
	*/
	bool ok = false;
	unsigned int ret;
	QString tmp(pStr.section('=', 0, 0));
	tmp.remove(QRegExp("^\\D*"));
	ret = tmp.simplified().toUInt(&ok);
	if (!ok)
	{
		qWarning("error extracting index, corrupt pls file.");
		ret = 0;
	}
	return ret;
}

void Mpris2Player::notifyChangedProperty(const QLatin1String &pPropertyName) {
	QDBusMessage signal = QDBusMessage::createSignal("/org/mpris/MediaPlayer2",
	                                                 "org.freedesktop.DBus.Properties",
	                                                 "PropertiesChanged");
	signal << QString("org.mpris.MediaPlayer2.Player");
	QVariantMap lChanged;
	lChanged.insert(pPropertyName, property(pPropertyName.latin1()));
	signal << lChanged;
	signal << QStringList();
	QDBusConnection::sessionBus().send(signal);
}
