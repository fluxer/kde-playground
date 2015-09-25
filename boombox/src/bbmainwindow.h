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
#ifndef BBMAINWINDOW_H
#define BBMAINWINDOW_H

#include <KAboutData>
#include <KTabWidget>
#include <KMenuBar>
#include <QCheckBox>
#include <QEvent>
#include <QLabel>
#include <QModelIndex>
#include <QVariant>
#include <phonon/phononnamespace.h>

namespace Phonon
{
class SeekSlider;
class VolumeSlider;
class AudioOutput;
class MediaObject;
class MediaSource;
}

class BBCollectionTab;
class BBFileSystemTab;
class BBStreamsTab;
class BBPlaylistSystem;
class Mpris2Player;
class KActionCollection;
class KAction;
class QToolButton;

class BBTitleLabel : public QLabel
{
Q_OBJECT
public:
	explicit BBTitleLabel(QWidget *pParent = 0);
	virtual QSize minimumSizeHint() const {return QSize(1, 1);}

public slots:
	void advanceAnimation();
	void setNewText(const QString &pText);

protected:
	virtual void paintEvent(QPaintEvent *);
	virtual void resizeEvent(QResizeEvent *pEvent);
	float mX, mVx;
	QTimer *mTimer;
	QPixmap mSecondPic;
	const int mMargin, mUpdateInterval;
	const float mSpeed;
};

class BBTabMenuBar : public KMenuBar
{
public:
	BBTabMenuBar(QWidget *pParent = 0) : KMenuBar(pParent) {}
	virtual bool event(QEvent *pEvent)
	{
		if(!isEnabled() && pEvent->type() == QEvent::MouseButtonPress)
		{
			pEvent->ignore();
			return false;
		}
		else
			return KMenuBar::event(pEvent);
	}
};

struct BBSongQueryJob
{
	//the query
	bool mShuffle;

	//the result
	QVariant mSong;

	//from mainwindow, to be returned again along with result
	bool mOnlyPlaceInQueue;
	bool mResumePlaying;
};

struct BBSongRef
{
	QVariant mSong;
	int mTabIndex;
};

class BBMainWindow : public KTabWidget
{
Q_OBJECT

public:
	BBMainWindow(const KAboutData *pAboutData, QWidget *parent = 0);
	~BBMainWindow();
	bool shuffleActive() { return mShuffleCheckBox->isChecked(); }
	void setShuffle(bool pEnable) { mShuffleCheckBox->setChecked(pEnable); }
	void saveSession();
	void readSession();
	void setCurrentSong(QVariant pSong, bool pStartPlayback = true, bool pAddToHistory = true);
	void addToPlayQueue(const QVariant &pSong, const QString &pDisplayText, int pTabNumber);
	void addCurrentSongToHistory();
	void setCurrentPlaylistSystem(int pTabNumber);

public slots:
	void setActiveTab(int pCurrentIndex);
	void showConfigDialog();
	void showHelp();
	void showAboutDialog();
	void showShortcutsDialog();
	void updatePhononState(Phonon::State pNewState, Phonon::State pOldState);
	void togglePlayback();
	void jumpToNextSong(bool pStartPlayback = false);
	void jumpToPreviousSong();
	void updateElapsedTime(qint64 pElapsedTime);
	void updateCurrentSong(const Phonon::MediaSource &pNewSource);
	void queueNextSong();
	void playNextSong(const BBSongQueryJob &pJob);
	void activateNextTab();
	void activatePreviousTab();
	virtual void changeEvent(QEvent *);
	void updateMetaDataDisplay();
	void updateShuffleStatus();
	void setBufferingStatus();
	void playSongFromHistory();
	void playSongFromQueue();

protected:
	void closeEvent(QCloseEvent *pEvent);

	void setupActions();
	void setupControls();

	BBCollectionTab *mCollectionTab;
	BBFileSystemTab *mFileSystemTab;
	BBStreamsTab *mStreamsTab;
	KActionCollection *mActionCollection;
	Phonon::SeekSlider *mPositionSlider;
	Phonon::MediaObject *mMediaObject;
	Phonon::AudioOutput *mAudioOutput;
	Phonon::VolumeSlider *mVolumeSlider;
	QVariant mNextSong;
	KAction *mPlayPauseAction;
	KAction *mNextAction;
	KAction *mPreviousAction;
	const KAboutData *mAboutData;

	QLabel *mElapsedTimeLabel;
	BBTitleLabel *mCurrentSongLabel;
	QCheckBox *mShuffleCheckBox;
	QToolButton *mPlayPauseButton;
	QToolButton *mNextButton;
	QToolButton *mPreviousButton;
	QWidget *mPlaybackControls;
	QMenu *mHistoryMenu;
	QMenu *mPlayQueueMenu;

	BBPlaylistSystem *mPlaylistSystem;
	BBPlaylistSystem *mPlayingPlaylistSystem;

	QTimer *mBufferingStatusTimer;

	qint64 mLastUpdatedTime;
	Mpris2Player *mMpris2Player;
	friend class Mpris2Player;
};

extern BBMainWindow *gMainWindow;

#endif // BBMAINWINDOW_H
