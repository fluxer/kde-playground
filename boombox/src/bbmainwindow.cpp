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
#include "bbmainwindow.h"
#include "bbcollectiontab.h"
#include "bbfilesystemtab.h"
#include "bbstreamstab.h"
#include "mpris2player.h"
#include "bbsettings.h"
#include "ui_directoriespage.h"
#include "playeradaptor.h"
#include "mediaplayer2adaptor.h"

#include <QCloseEvent>
#include <QDebug>
#include <QLabel>
#include <QLayout>
#include <QPainter>
#include <QTabBar>
#include <QTimer>
#include <QToolButton>
#include <KMenuBar>
#include <KMenu>
#include <KAboutApplicationDialog>
#include <KApplication>
#include <KAction>
#include <KActionCollection>
#include <KShortcutsDialog>
#include <KStandardAction>
#include <KStandardShortcut>
#include <KConfigDialog>
#include <KMessageBox>
#include <KToolInvocation>
#include <phonon/audiooutput.h>
#include <phonon/seekslider.h>
#include <phonon/mediaobject.h>
#include <phonon/volumeslider.h>

 // maximum entries in the playback history
const int gMaxHistory = 10;

class BBCenteredMenuBar : public QWidget
{
public:
	BBCenteredMenuBar(QWidget *pParent = 0);
	KMenuBar *mMenuBar;
	virtual QSize sizeHint() const;
};

BBCenteredMenuBar::BBCenteredMenuBar(QWidget *pParent)
	: QWidget(pParent)
{
	QHBoxLayout *lLayout = new QHBoxLayout(this);
	lLayout->setMargin(0);
	mMenuBar = new KMenuBar();
	lLayout->addWidget(mMenuBar, 0, Qt::AlignVCenter);
	lLayout->addSpacing(14);
}

QSize BBCenteredMenuBar::sizeHint() const
{
	return QWidget::sizeHint() + QSize(0, 10); //trick to make oxygen show it correctly.
}

BBTitleLabel::BBTitleLabel(QWidget *pParent)
	: QLabel(pParent), mMargin(30), mUpdateInterval(40), mSpeed(0.025)
{
	mTimer = new QTimer(this);
	connect(mTimer, SIGNAL(timeout()), this, SLOT(advanceAnimation()));
}

void BBTitleLabel::setNewText(const QString &pText)
{
	setText(pText);
	mVx = -mSpeed;

	QRect lRect = contentsRect();
	lRect.setWidth(fontMetrics().boundingRect(pText).width() + mMargin);
	mSecondPic = QPixmap(lRect.size());
	mSecondPic.fill(Qt::transparent);
	QPainter lPainter(&mSecondPic);
	lPainter.setPen(palette().color(QPalette::Active, QPalette::WindowText));
	lPainter.drawText(lRect, Qt::AlignHCenter, text());

	if(lRect.width() > width() + mMargin)
	{
		mX = -mMargin/2.0;
		mTimer->start(mUpdateInterval);
	}
	else
	{
		mX = (width() - lRect.width())/2.0;
		mTimer->stop();
	}
}

void BBTitleLabel::advanceAnimation()
{
	mX += mVx*mUpdateInterval;
	if((mX > 0 && mVx > 0) || (mX + mSecondPic.width() < width() && mVx < 0))
		mVx = -mVx;
	update();
}

void BBTitleLabel::paintEvent(QPaintEvent *)
{
	QPainter lPainter(this);
	lPainter.drawPixmap(mX, 0, mSecondPic);
}

void BBTitleLabel::resizeEvent(QResizeEvent *pEvent)
{
	QLabel::resizeEvent(pEvent);

	if(mSecondPic.width() > width() + mMargin)
		mTimer->start(40);
	else
	{
		mX = (width() - mSecondPic.width())/2.0;
		mTimer->stop();
	}
}

BBMainWindow::BBMainWindow(const KAboutData *pAboutData, QWidget *parent)
	: KTabWidget(parent,Qt::Window), mAboutData(pAboutData),
     mPlaylistSystem(0), mPlayingPlaylistSystem(0), mLastUpdatedTime(0), mMpris2Player(0)
{
	mBufferingStatusTimer = new QTimer(this);
	connect(mBufferingStatusTimer, SIGNAL(timeout()), this, SLOT(setBufferingStatus()));

	setWindowTitle("BoomBox");
	setDocumentMode(true);

	setupActions();
	setupControls();

	connect(tabBar(), SIGNAL(currentChanged(int)), this, SLOT(setActiveTab(int)));

	mCollectionTab = new BBCollectionTab(0);
	addTab(mCollectionTab, mCollectionTab->windowTitle());
	tabBar()->setTabButton(0, QTabBar::RightSide, mCollectionTab->getTabMenuBar());

	mFileSystemTab = new BBFileSystemTab(1);
	addTab(mFileSystemTab, mFileSystemTab->windowTitle());

	mStreamsTab = new BBStreamsTab(2);
	addTab(mStreamsTab, mStreamsTab->windowTitle());

	mAudioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
	mVolumeSlider->setAudioOutput(mAudioOutput);
	mMediaObject = new Phonon::MediaObject(this);
	mMediaObject->setTickInterval(1000);
	connect(mMediaObject, SIGNAL(tick(qint64)), SLOT(updateElapsedTime(qint64)));
	connect(mMediaObject, SIGNAL(currentSourceChanged(Phonon::MediaSource)), SLOT(updateCurrentSong(Phonon::MediaSource)));
	connect(mMediaObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
			  SLOT(updatePhononState(Phonon::State, Phonon::State)));
	connect(mMediaObject, SIGNAL(aboutToFinish()), SLOT(queueNextSong()));
	connect(mMediaObject, SIGNAL(metaDataChanged()), SLOT(updateMetaDataDisplay()));

	mPositionSlider->setMediaObject(mMediaObject);
	Phonon::createPath(mMediaObject, mAudioOutput);
}

BBMainWindow::~BBMainWindow()
{
}

void BBMainWindow::setupActions()
{
	mActionCollection = new KActionCollection(this);

	BBCenteredMenuBar *lLeftMenu = new BBCenteredMenuBar();
	KMenu *lBBMenu = new KMenu(i18n("BoomBox"));
	lBBMenu->addAction((QAction *)KStandardAction::preferences(this, SLOT(showConfigDialog()), mActionCollection));
	lBBMenu->addAction((QAction *)KStandardAction::keyBindings(this, SLOT(showShortcutsDialog()), mActionCollection));
	lBBMenu->addSeparator();
	lBBMenu->addAction((QAction *)KStandardAction::help(this, SLOT(showHelp()), mActionCollection));
	lBBMenu->addAction((QAction *)KStandardAction::aboutApp(this, SLOT(showAboutDialog()), mActionCollection));
	lBBMenu->addSeparator();
	lBBMenu->addAction((QAction *)KStandardAction::quit(this, SLOT(close()), mActionCollection));
	lLeftMenu->mMenuBar->addMenu(lBBMenu);
	setCornerWidget(lLeftMenu, Qt::TopLeftCorner);

	mPlayPauseAction = new KAction(KIcon("media-playback-start"), i18n("Play/Pause"), this);
	mActionCollection->addAction("play_pause", mPlayPauseAction);
	mPlayPauseAction->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_X));
	connect(mPlayPauseAction, SIGNAL(triggered()), SLOT(togglePlayback()));

	mNextAction = new KAction(KIcon("media-skip-forward"), i18n("Next Song"), this);
	mActionCollection->addAction("next_song", mNextAction);
	mNextAction->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_C));
	connect(mNextAction, SIGNAL(triggered()), SLOT(jumpToNextSong()));

	mPreviousAction = new KAction(KIcon("media-skip-backward"), i18n("Previous Song"), this);
	mActionCollection->addAction("previous_song", mPreviousAction);
	mPreviousAction->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_Z));
	connect(mPreviousAction, SIGNAL(triggered()), SLOT(jumpToPreviousSong()));

	KAction *lNextTabAction = new KAction(i18n("Activate Next Tab"), this);
	lNextTabAction->setShortcut(KStandardShortcut::tabNext());
	mActionCollection->addAction("next_tab", lNextTabAction);
	addAction((QAction *)lNextTabAction);
	connect(lNextTabAction, SIGNAL(triggered()), this, SLOT(activateNextTab()));

	KAction *lPreviousTabAction = new KAction(i18n("Activate Previous Tab"), this);
	lPreviousTabAction->setShortcut(KStandardShortcut::tabPrev());
	mActionCollection->addAction("previous_tab", lPreviousTabAction);
	addAction((QAction *)lPreviousTabAction);
	connect(lPreviousTabAction, SIGNAL(triggered()), this, SLOT(activatePreviousTab()));

	mActionCollection->readSettings();
}

void BBMainWindow::setupControls()
{
	mPlaybackControls = new QWidget();

	mHistoryMenu = new QMenu(this);

	mPreviousButton = new QToolButton(mPlaybackControls);
	mPreviousButton->setIconSize(QSize(32,32));
	mPreviousButton->setDefaultAction(mPreviousAction);
	mPreviousButton->setAutoRaise(true);
	mPreviousButton->setPopupMode(QToolButton::MenuButtonPopup);
	mPreviousButton->setMenu(mHistoryMenu);
	mPreviousButton->setEnabled(false);

	mPlayPauseButton = new QToolButton(mPlaybackControls);
	mPlayPauseButton->setIconSize(QSize(32,32));
	mPlayPauseButton->setAutoRaise(true);
	mPlayPauseButton->setDefaultAction(mPlayPauseAction);

	mPlayQueueMenu = new QMenu(this);

	mNextButton = new QToolButton(mPlaybackControls);
	mNextButton->setIconSize(QSize(32,32));
	mNextButton->setDefaultAction(mNextAction);
	mNextButton->setAutoRaise(true);
	mNextButton->setPopupMode(QToolButton::MenuButtonPopup);
	mNextButton->setMenu(0);

	mPositionSlider = new Phonon::SeekSlider(mPlaybackControls);
	mPositionSlider->setIconVisible(false);
	mShuffleCheckBox = new QCheckBox(i18n("Shuffle"), mPlaybackControls);
	connect(mShuffleCheckBox, SIGNAL(toggled(bool)), SLOT(updateShuffleStatus()));

	mVolumeSlider = new Phonon::VolumeSlider(mPlaybackControls);
	mVolumeSlider->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	mVolumeSlider->setMuteVisible(false);

	mElapsedTimeLabel = new QLabel("00:00", mPlaybackControls);
	mElapsedTimeLabel->setFont(KGlobalSettings::generalFont());
	mElapsedTimeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	mCurrentSongLabel = new BBTitleLabel(mPlaybackControls);
	mCurrentSongLabel->setFont(KGlobalSettings::generalFont());
	mCurrentSongLabel->setNewText(i18n("Hey ho! Let's go!"));

	QHBoxLayout *lHLayout = new QHBoxLayout;
	lHLayout->addWidget(mPreviousButton);
	lHLayout->addWidget(mPlayPauseButton);
	lHLayout->addWidget(mNextButton);
	QVBoxLayout *lVLayout = new QVBoxLayout;
	lHLayout->addLayout(lVLayout);
	QHBoxLayout *lHLayout2 = new QHBoxLayout;
	lVLayout->addLayout(lHLayout2);
	lHLayout2->addWidget(mCurrentSongLabel);
	lHLayout2->addWidget(mElapsedTimeLabel);
	QHBoxLayout *lHLayout3 = new QHBoxLayout;
	lVLayout->addLayout(lHLayout3);
	lHLayout3->addWidget(mShuffleCheckBox);
	lHLayout3->addWidget(mPositionSlider);
	lHLayout3->addWidget(mVolumeSlider);

	mPlaybackControls->setLayout(lHLayout);
}

void BBMainWindow::closeEvent(QCloseEvent *pEvent)
{
	saveSession();
	pEvent->accept();
	kapp->quit();
}

void BBMainWindow::setActiveTab(int pCurrentIndex)
{
	int lCount = count();
	for(int i = 0; i < lCount; ++i)
	{
		QWidget *lMenuBar = tabBar()->tabButton(i, QTabBar::RightSide);
		if(lMenuBar)
			lMenuBar->setEnabled(i == pCurrentIndex);
	}
	if(mCollectionTab->tabNumber() == pCurrentIndex)
		mCollectionTab->embedControls(mPlaybackControls);
	else if(mFileSystemTab->tabNumber() == pCurrentIndex)
		mFileSystemTab->embedControls(mPlaybackControls);
	else if(mStreamsTab->tabNumber() == pCurrentIndex)
		mStreamsTab->embedControls(mPlaybackControls);
}

void BBMainWindow::setCurrentPlaylistSystem(int pTabNumber)
{
	if(mPlaylistSystem)
		mPlaylistSystem->disconnect();

	if(pTabNumber == mCollectionTab->tabNumber())
		mPlaylistSystem = mCollectionTab;
	if(pTabNumber == mFileSystemTab->tabNumber())
		mPlaylistSystem = mFileSystemTab;
	if(pTabNumber == mStreamsTab->tabNumber())
		mPlaylistSystem = mStreamsTab;

	connect(mPlaylistSystem, SIGNAL(songQueryReady(BBSongQueryJob)), this, SLOT(playNextSong(BBSongQueryJob)));
}

void BBMainWindow::showConfigDialog()
{
	if(KConfigDialog::showDialog("settings"))
		return;

	KConfigDialog *dialog = new KConfigDialog(this, "settings", BBSettings::self());
	QWidget *generalSettingsDlg = new QWidget;
	Ui_DirectoriesPage lDirPage;
	lDirPage.setupUi(generalSettingsDlg);
	dialog->addPage(generalSettingsDlg, i18n("File locations"), "folder-sound");
	//connect(dialog, SIGNAL(settingsChanged(QString)), m_view, SLOT(settingsChanged()));
	dialog->show();
}

void BBMainWindow::showHelp()
{
	KToolInvocation::invokeHelp();
}

void BBMainWindow::showAboutDialog()
{
	KAboutApplicationDialog *lDialog = new KAboutApplicationDialog(mAboutData, this);
	lDialog->show();
	connect(lDialog, SIGNAL(finished()), lDialog, SLOT(deleteLater()));
}

void BBMainWindow::showShortcutsDialog()
{
	KShortcutsDialog lDialog(KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsAllowed, this);
	lDialog.addCollection(mActionCollection, "BoomBox");
	lDialog.addCollection(mCollectionTab->actionCollection(), i18n("Music Collection Tab"));
	lDialog.addCollection(mFileSystemTab->actionCollection(), i18n("File System Tab"));
	lDialog.configure();
}

void BBMainWindow::togglePlayback()
{
	if(mMediaObject->currentSource().type() == Phonon::MediaSource::Empty)
		jumpToNextSong(true);
	else
	{
		Phonon::State lState = mMediaObject->state();
		if(lState == Phonon::PlayingState || lState == Phonon::BufferingState)
			mMediaObject->pause();
		else
			mMediaObject->play();
	}
}

void BBMainWindow::queueNextSong()
{
	QList<QAction *> lQueue = mPlayQueueMenu->actions();
	if(!lQueue.isEmpty())
	{
		QAction *lNext = lQueue.takeFirst();
		mPlayQueueMenu->removeAction(lNext);
		BBSongRef *lSongRef = (BBSongRef *)lNext->data().value<void *>();
		setCurrentIndex(lSongRef->mTabIndex);
		setCurrentPlaylistSystem(lSongRef->mTabIndex);

		BBSongQueryJob lJob;
		lJob.mOnlyPlaceInQueue = true;
		lJob.mResumePlaying = true;
		lJob.mSong = lSongRef->mSong;
		playNextSong(lJob);

		delete lSongRef;
		delete lNext;
		if(lQueue.isEmpty())
			mNextButton->setMenu(0);
	}
	else
	{
		BBSongQueryJob lJob;
		lJob.mShuffle = mShuffleCheckBox->checkState();
		lJob.mOnlyPlaceInQueue = true;
		lJob.mResumePlaying = true;
		mPlaylistSystem->queryNextSong(lJob);
	}
}

void BBMainWindow::jumpToNextSong(bool pStartPlayback)
{
	QList<QAction *> lQueue = mPlayQueueMenu->actions();
	if(!lQueue.isEmpty())
	{
		QAction *lNext = lQueue.takeFirst();
		mPlayQueueMenu->removeAction(lNext);
		BBSongRef *lSongRef = (BBSongRef *)lNext->data().value<void *>();
		setCurrentIndex(lSongRef->mTabIndex);
		setCurrentPlaylistSystem(lSongRef->mTabIndex);
		setCurrentSong(lSongRef->mSong, pStartPlayback);
		delete lSongRef;
		delete lNext;
		if(lQueue.isEmpty())
			mNextButton->setMenu(0);
	}
	else
	{
		BBSongQueryJob lJob;
		lJob.mShuffle = mShuffleCheckBox->checkState();
		lJob.mOnlyPlaceInQueue = false;
		lJob.mResumePlaying = pStartPlayback || mMediaObject->state() == Phonon::PlayingState;;
		mPlaylistSystem->queryNextSong(lJob);
	}
}

void BBMainWindow::jumpToPreviousSong()
{
	QList<QAction *> lHistory = mHistoryMenu->actions();
	if(lHistory.isEmpty())
		return;

	QAction *lPrevious = lHistory.takeFirst();
	mHistoryMenu->removeAction(lPrevious);
	BBSongRef *lSongRef = (BBSongRef *)lPrevious->data().value<void *>();
	setCurrentIndex(lSongRef->mTabIndex);
	setCurrentPlaylistSystem(lSongRef->mTabIndex);
	setCurrentSong(lSongRef->mSong, false, false);
	delete lSongRef;
	delete lPrevious;
	if(lHistory.isEmpty())
		mPreviousButton->setEnabled(false);
}

void BBMainWindow::playNextSong(const BBSongQueryJob &pJob)
{
	Phonon::MediaSource lMediaSource = mMediaObject->currentSource();
	if(lMediaSource.type() == Phonon::MediaSource::Url
		|| lMediaSource.type() == Phonon::MediaSource::LocalFile)
	{
		addCurrentSongToHistory();
	}

	mNextSong = pJob.mSong;
	KUrl lSongUrl = mPlaylistSystem->songUrl(pJob.mSong);
	if(pJob.mOnlyPlaceInQueue && mMediaObject->state() == Phonon::PlayingState)
		mMediaObject->enqueue(lSongUrl);
	else
	{
		mMediaObject->stop();
		mMediaObject->clearQueue();
		mMediaObject->setCurrentSource(lSongUrl);
		if(pJob.mResumePlaying)
			mMediaObject->play();
	}
}

void BBMainWindow::setCurrentSong(QVariant pSong, bool pStartPlayback, bool pAddToHistory)
{
	KUrl lNewSongUrl = mPlaylistSystem->songUrl(pSong);
	if(lNewSongUrl.isEmpty())
		return;

	Phonon::MediaSource lMediaSource = mMediaObject->currentSource();
	if(pAddToHistory && (lMediaSource.type() == Phonon::MediaSource::Url
		|| lMediaSource.type() == Phonon::MediaSource::LocalFile))
	{
		addCurrentSongToHistory();
	}

	bool lResumePlaying = mMediaObject->state() == Phonon::PlayingState;
	mNextSong = pSong;
	mMediaObject->stop();
	mMediaObject->clearQueue();
	mMediaObject->setCurrentSource(lNewSongUrl);
	if(pStartPlayback || lResumePlaying)
		mMediaObject->play();
}

void BBMainWindow::addToPlayQueue(const QVariant &pSong, const QString &pDisplayText, int pTabNumber)
{
	QAction *lQueueAction = new QAction(this);
	BBSongRef *lSongRef = new BBSongRef();
	lSongRef->mSong = pSong;
	lSongRef->mTabIndex = pTabNumber;
	lQueueAction->setData(QVariant::fromValue((void *)lSongRef));
	lQueueAction->setText(pDisplayText);
	connect(lQueueAction, SIGNAL(triggered()), this, SLOT(playSongFromQueue()));
	mPlayQueueMenu->addAction(lQueueAction);
	mNextButton->setMenu(mPlayQueueMenu);
}

void BBMainWindow::addCurrentSongToHistory()
{
	QAction *lHistoryAction = new QAction(this);
	BBSongRef *lSongRef = new BBSongRef();
	lSongRef->mSong = mPlayingPlaylistSystem->currentSong();
	lSongRef->mTabIndex = mPlayingPlaylistSystem->tabNumber();
	lHistoryAction->setData(QVariant::fromValue((void *)lSongRef));
	lHistoryAction->setText(mCurrentSongLabel->text());
	connect(lHistoryAction, SIGNAL(triggered()), this, SLOT(playSongFromHistory()));
	QList<QAction *> lHistory = mHistoryMenu->actions();
	mHistoryMenu->insertAction(lHistory.isEmpty() ? 0 : lHistory.first(), lHistoryAction);
	if(lHistory.count() == gMaxHistory)
	{
		mHistoryMenu->removeAction(lHistory.last());
		delete lHistory.last();
	}
	mPreviousButton->setEnabled(true);
}

void BBMainWindow::updateElapsedTime(qint64 pElapsedTime)
{
	QTime lTime(0, pElapsedTime/60000, (pElapsedTime/1000) % 60);
	mElapsedTimeLabel->setText(lTime.toString("mm:ss"));
	if(qAbs(pElapsedTime - mLastUpdatedTime) > 1500) {
		mMpris2Player->notifySeeked(pElapsedTime * 1000);
	}
	mLastUpdatedTime = pElapsedTime;
}

void BBMainWindow::updateCurrentSong(const Phonon::MediaSource &pNewSource)
{
	Q_UNUSED(pNewSource)
	mPlaylistSystem->setCurrentSong(mNextSong);
	mCurrentSongLabel->setNewText(mPlaylistSystem->displayString(mNextSong));
	//at this time we can be sure the origin for current song is the current playlist system
	mPlayingPlaylistSystem = mPlaylistSystem;
}

void BBMainWindow::updatePhononState(Phonon::State pNewState, Phonon::State pOldState)
{
	switch (pNewState)
	{
	case Phonon::ErrorState:
		if(mMediaObject->errorType() == Phonon::FatalError)
			QMessageBox::warning(this, i18n("Fatal Error"), mMediaObject->errorString());
		else
			QMessageBox::warning(this, i18n("Error"), mMediaObject->errorString());
		break;
	case Phonon::PlayingState:
		mPlayPauseAction->setIcon(KIcon("media-playback-pause"));
		break;
	case Phonon::StoppedState:
		mPlayPauseAction->setIcon(KIcon("media-playback-start"));
		mElapsedTimeLabel->setText("00:00");
		break;
	case Phonon::PausedState:
		mPlayPauseAction->setIcon(KIcon("media-playback-start"));
		break;
	case Phonon::BufferingState:
		mBufferingStatusTimer->start(200);
		break;
	default:
		break;
	}

	if(pOldState == Phonon::BufferingState)
	{
		if(mPlaylistSystem == mStreamsTab)
			updateMetaDataDisplay();
		else
			mCurrentSongLabel->setNewText(mPlaylistSystem->displayString(mPlaylistSystem->currentSong()));
		mBufferingStatusTimer->stop();
	}
	if(mMpris2Player) {
		mMpris2Player->notifyChangedProperty(QLatin1String("PlaybackStatus"));
	}
}

void BBMainWindow::playSongFromHistory()
{
	QAction *lSenderAction = qobject_cast<QAction *>(sender());
	if(lSenderAction)
	{
		BBSongRef *lSongRef = (BBSongRef *)lSenderAction->data().value<void *>();
		setCurrentIndex(lSongRef->mTabIndex);
		setCurrentPlaylistSystem(lSongRef->mTabIndex);
		setCurrentSong(lSongRef->mSong, false, false);
		delete lSongRef;

		QList<QAction *> lHistory = mHistoryMenu->actions();
		QAction *lAction;
		while((lAction = lHistory.takeFirst()) != lSenderAction)
		{
			mHistoryMenu->removeAction(lAction);
			lAction->deleteLater();
		}
		lSenderAction->deleteLater();

		if(lHistory.isEmpty())
			mPreviousButton->setEnabled(false);
	}
}

void BBMainWindow::playSongFromQueue()
{
	QAction *lSenderAction = qobject_cast<QAction *>(sender());
	if(lSenderAction)
	{
		BBSongRef *lSongRef = (BBSongRef *)lSenderAction->data().value<void *>();
		setCurrentIndex(lSongRef->mTabIndex);
		setCurrentPlaylistSystem(lSongRef->mTabIndex);
		setCurrentSong(lSongRef->mSong, false);
		delete lSongRef;

		QList<QAction *> lQueue = mPlayQueueMenu->actions();
		QAction *lAction;
		while((lAction = lQueue.takeFirst()) != lSenderAction)
		{
			mPlayQueueMenu->removeAction(lAction);
			lAction->deleteLater();
		}
		lSenderAction->deleteLater();
		if(lQueue.isEmpty())
			mNextButton->setMenu(0);
	}
}

void BBMainWindow::changeEvent(QEvent *pEvent)
{
	if(pEvent->type() == QEvent::FontChange)
	{
		mCurrentSongLabel->setFont(KGlobalSettings::generalFont());
	}
	KTabWidget::changeEvent(pEvent);
}

void BBMainWindow::saveSession()
{
	KSharedConfigPtr lConfig = KGlobal::config();
	KConfigGroup lSessionGroup(lConfig, "session");

	lSessionGroup.writeEntry("shuffle_enabled", mShuffleCheckBox->isChecked());
	lSessionGroup.writeEntry("active_tab", this->currentIndex());
	mCollectionTab->saveSession(lSessionGroup);
	mFileSystemTab->saveSession(lSessionGroup);
	mStreamsTab->saveSession(lSessionGroup);
	lConfig->sync();
}

void BBMainWindow::readSession()
{
	KSharedConfigPtr lConfig = KGlobal::config();
	KConfigGroup lSessionGroup(lConfig, "session");
	mCollectionTab->readSession(lSessionGroup);
	mFileSystemTab->readSession(lSessionGroup);
	mStreamsTab->readSession(lSessionGroup);
	mShuffleCheckBox->setChecked(lSessionGroup.readEntry("shuffle_enabled", false));
	int lActiveTab = lSessionGroup.readEntry("active_tab", 0);
	setCurrentIndex(lActiveTab);
	setCurrentPlaylistSystem(lActiveTab);
	mPlayingPlaylistSystem = mPlaylistSystem;


	// only create mpris interface after everything else is constructed.
	// doing otherwise causes dbus deadlock with the "now playing" mpris
	// daemon  from ktp, running in kded
	QDBusConnection lConnection = QDBusConnection::sessionBus();
	lConnection.registerService("org.mpris.MediaPlayer2.BoomBox");
	mMpris2Player = new Mpris2Player(this);
	new MediaPlayer2Adaptor(mMpris2Player);
	new PlayerAdaptor(mMpris2Player);
	lConnection.registerObject("/org/mpris/MediaPlayer2", mMpris2Player);
}

void BBMainWindow::activateNextTab()
{
	setCurrentIndex((currentIndex() + 1) % count());
}

void BBMainWindow::activatePreviousTab()
{
	setCurrentIndex((currentIndex() + count() - 1) % count());
}

void BBMainWindow::updateMetaDataDisplay()
{
	// Streams can have changing metadata
	if(!mMediaObject->isSeekable()) {
		QMultiMap<QString, QString> lMetaData = mMediaObject->metaData();
		QString lArtist = lMetaData.value("ARTIST");
		QString lTitle = lMetaData.value("TITLE");
		QString lNewName;
		if(!lArtist.isEmpty())
		{
			if(!lTitle.isEmpty())
				lNewName = QString::fromAscii("%1 - %2").arg(lArtist, lTitle);
			else
				lNewName = lArtist;
		}
		else
		{
			if(!lTitle.isEmpty())
				lNewName = lTitle;
			else
				lNewName = mPlaylistSystem->displayString(mPlaylistSystem->currentSong());
		}
		mCurrentSongLabel->setNewText(lNewName);
	}
	// always update mpris clients
	if(mMpris2Player) {
		mMpris2Player->notifyChangedProperty(QLatin1String("Metadata"));
	}
}

void BBMainWindow::updateShuffleStatus() {
	if(mMpris2Player) {
		mMpris2Player->notifyChangedProperty(QLatin1String("Shuffle"));
	}
}

void BBMainWindow::setBufferingStatus()
{
	mCurrentSongLabel->setNewText(i18n("Buffering..."));
}
