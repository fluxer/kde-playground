/* Copyright 2009 Thomas McGuire <mcguire@kde.org>

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2 of the License or
   ( at your option ) version 3 or, at the discretion of KDE e.V.
   ( which shall act as a proxy as in section 14 of the GPLv3 ), any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "pop3resource.h"
#include "accountdialog.h"
#include "settings.h"
#include "jobs.h"
#include "pop3resourceattribute.h"

#include <Akonadi/CollectionFetchJob>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/AttributeFactory>
#include <akonadi/kmime/messageflags.h>
#include <akonadi/kmime/specialmailcollectionsrequestjob.h>
#include <akonadi/kmime/specialmailcollections.h>
#include <kmime/kmime_util.h>
#include <Mailtransport/PrecommandJob>
#include <Mailtransport/Transport>

#include <KPasswordDialog>
#include <KMessageBox>
#include <kwallet.h>
#include <klocale.h>

#include <QTimer>

using namespace Akonadi;
using namespace MailTransport;
using namespace KWallet;

POP3Resource::POP3Resource( const QString &id )
    : ResourceBase( id ),
      mState( Idle ),
      mPopSession( 0 ),
      mAskAgain( false ),
      mIntervalTimer( new QTimer( this ) ),
      mTestLocalInbox( false ),
      mWallet( 0 )
{
  Akonadi::AttributeFactory::registerAttribute<Pop3ResourceAttribute>();
  setNeedsNetwork( true );
  Settings::self()->setResourceId( identifier() );
  resetState();

  connect( this, SIGNAL(abortRequested()),
           this, SLOT(slotAbortRequested()) );
  connect( mIntervalTimer, SIGNAL(timeout()),
           this, SLOT(intervalCheckTriggered()) );
  connect( this, SIGNAL(reloadConfiguration()), SLOT(configurationChanged()) );
}

POP3Resource::~POP3Resource()
{
  Settings::self()->writeConfig();
  delete mWallet;
  mWallet = 0;
}

void POP3Resource::configurationChanged()
{
  Settings::self()->writeConfig();
  updateIntervalTimer();
}

void POP3Resource::updateIntervalTimer()
{
  if ( Settings::self()->intervalCheckEnabled() && mState == Idle ) {
    mIntervalTimer->start( Settings::self()->intervalCheckInterval() * 1000 * 60 );
  }
  else {
    mIntervalTimer->stop();
  }
}

void POP3Resource::intervalCheckTriggered()
{
  Q_ASSERT( mState == Idle );
  if ( isOnline() ) {
    kDebug() << "Starting interval mail check.";
    startMailCheck();
    mIntervalCheckInProgress = true;
  } else {
    mIntervalTimer->start();
  }
}

void POP3Resource::aboutToQuit()
{
  if ( mState != Idle )
    cancelSync( i18n( "Mail check aborted." ) );
}

void POP3Resource::slotAbortRequested()
{
  if ( mState != Idle )
    cancelSync( i18n( "Mail check was canceled manually." ), false /* no error */ );
}

void POP3Resource::configure( WId windowId )
{
  QPointer<AccountDialog> accountDialog( new AccountDialog( this, windowId ) );
  if ( accountDialog->exec() == QDialog::Accepted ) {
    updateIntervalTimer();
    emit configurationDialogAccepted();
  }
  else {
    emit configurationDialogRejected();
  }

  delete accountDialog;
}

void POP3Resource::retrieveItems( const Akonadi::Collection &collection )
{
  Q_UNUSED( collection );
  kWarning() << "This should never be called, we don't have a collection!";
}

bool POP3Resource::retrieveItem( const Akonadi::Item &item, const QSet<QByteArray> &parts )
{
  Q_UNUSED( item );
  Q_UNUSED( parts );
  kWarning() << "This should never be called, we don't have any item!";
  return false;
}

QString POP3Resource::buildLabelForPasswordDialog( const QString &detailedError ) const
{
  QString queryText = i18n( "Please enter the username and password for account '%1'.",
                            agentName() );
  queryText += QLatin1String("<br>") + detailedError;
  return queryText;
}

void POP3Resource::walletOpenedForLoading( bool success )
{
  bool passwordLoaded = success;
  if ( success ) {
    if ( mWallet && mWallet->isOpen() && mWallet->hasFolder( QLatin1String("pop3") ) ) {
      mWallet->setFolder( QLatin1String("pop3") );
      if ( mWallet->hasEntry( identifier() ) )
        mWallet->readPassword( identifier(), mPassword );
      else
        passwordLoaded = false;
    }
    else {
      passwordLoaded = false;
    }
  }
  delete mWallet;
  mWallet = 0;

  if ( !passwordLoaded ) {
    QString queryText = buildLabelForPasswordDialog(
        i18n( "You are asked here because the password could not be loaded from the wallet." ) );
    showPasswordDialog( queryText );
  }
  else {
    advanceState( Connect );
  }
}

void POP3Resource::walletOpenedForSaving( bool success )
{
  if ( success ) {
    if ( mWallet && mWallet->isOpen() ) {
      if ( !mWallet->hasFolder( QLatin1String("pop3") ) ) {
        mWallet->createFolder( QLatin1String("pop3") );
      }
      mWallet->setFolder( QLatin1String("pop3") );
      mWallet->writePassword( identifier(), mPassword );
    }
  }
  else
    kWarning() << "Unable to write the password to the wallet.";

  delete mWallet;
  mWallet = 0;
  finish();
}


void POP3Resource::showPasswordDialog( const QString &queryText )
{
  QPointer<KPasswordDialog> dlg =
    new KPasswordDialog(
      0,
      KPasswordDialog::ShowUsernameLine );
  dlg->setModal( true );
  dlg->setUsername( Settings::self()->login() );
  dlg->setPassword( mPassword );
  dlg->setPrompt( queryText );
  dlg->setCaption( name() );
  dlg->addCommentLine( i18n( "Account:" ), name() );

  bool gotIt = false;
  if ( dlg->exec() ) {
    mPassword = dlg->password();
    Settings::self()->setLogin( dlg->username() );
    Settings::self()->writeConfig();
    if ( !dlg->password().isEmpty()  ) {
      mSavePassword = true;
    }

    mAskAgain = false;
    advanceState( Connect );
    gotIt = true;
  }
  delete dlg;
  if ( !gotIt ) {
    cancelSync( i18n( "No username and password supplied." ) );
  }
}

void POP3Resource::advanceState( State nextState )
{
  mState = nextState;
  doStateStep();
}

void POP3Resource::doStateStep()
{
  switch ( mState )
  {
  case Idle:
    {
      Q_ASSERT( false );
      kWarning() << "State machine should not be called in idle state!";
      break;
    }
  case FetchTargetCollection:
    {
      kDebug() << "================ Starting state FetchTargetCollection ==========";
      emit status( Running, i18n( "Preparing transmission from \"%1\".", name() ) );
      Collection targetCollection( Settings::self()->targetCollection() );
      if ( !targetCollection.isValid() ) {
        // No target collection set in the config? Try requesting a default inbox
        SpecialMailCollectionsRequestJob *requestJob = new SpecialMailCollectionsRequestJob( this );
        requestJob->requestDefaultCollection( SpecialMailCollections::Inbox );
        requestJob->start();
        connect ( requestJob, SIGNAL(result(KJob*)),
                  this, SLOT(localFolderRequestJobFinished(KJob*)) );
      }
      else {
        CollectionFetchJob *fetchJob = new CollectionFetchJob( targetCollection,
                                                          CollectionFetchJob::Base );
        fetchJob->start();
        connect( fetchJob, SIGNAL(result(KJob*)),
                 this, SLOT(targetCollectionFetchJobFinished(KJob*)) );
      }
      break;
    }
  case Precommand:
    {
      kDebug() << "================ Starting state Precommand =====================";
      if ( !Settings::self()->precommand().isEmpty() ) {
        PrecommandJob *precommandJob = new PrecommandJob( Settings::self()->precommand(), this );
        connect( precommandJob, SIGNAL(result(KJob*)),
                 this, SLOT(precommandResult(KJob*)) );
        precommandJob->start();
        emit status( Running, i18n( "Executing precommand." ) );
      }
      else {
        advanceState( RequestPassword );
      }
      break;
    }
  case RequestPassword:
    {
      kDebug() << "================ Starting state RequestPassword ================";

      // Don't show any wallet or password prompts when we are unit-testing
      if ( !Settings::self()->unitTestPassword().isEmpty() ) {
        mPassword = Settings::self()->unitTestPassword();
        advanceState( Connect );
        break;
      }

      const bool passwordNeeded = Settings::self()->authenticationMethod() != MailTransport::Transport::EnumAuthenticationType::GSSAPI;
      const bool loadPasswordFromWallet = !mAskAgain && passwordNeeded && !Settings::self()->login().isEmpty() &&
                                          mPassword.isEmpty();
      if ( loadPasswordFromWallet ) {
        mWallet = Wallet::openWallet( Wallet::NetworkWallet(), winIdForDialogs(),
                                      Wallet::Asynchronous );
      }
      if ( loadPasswordFromWallet && mWallet ) {
        connect( mWallet, SIGNAL(walletOpened(bool)),
                 this, SLOT(walletOpenedForLoading(bool)) );
      }
      else if ( passwordNeeded && ( mPassword.isEmpty() || mAskAgain ) ) {
        QString detail;
        if ( mAskAgain )
          detail = i18n( "You are asked here because the previous login was not successful." );
        else if ( Settings::self()->login().isEmpty() )
          detail = i18n( "You are asked here because the username you supplied is empty." );
        else if ( !mWallet )
          detail = i18n( "You are asked here because the wallet password storage is disabled." );

        showPasswordDialog( buildLabelForPasswordDialog( detail ) );
      }
      else {
        // No password needed or using previous password, go on with Connect
        advanceState( Connect );
      }

      break;
    }
  case Connect:
    {
      kDebug() << "================ Starting state Connect ========================";
      Q_ASSERT( !mPopSession );
      mPopSession = new POPSession( mPassword );
      connect( mPopSession, SIGNAL(slaveError(int,QString)),
               this, SLOT(slotSessionError(int,QString)) );
      advanceState( Login );
      break;
    }
  case Login:
    {
      kDebug() << "================ Starting state Login ==========================";

      LoginJob *loginJob = new LoginJob( mPopSession );
      connect( loginJob, SIGNAL(result(KJob*)),
               this, SLOT(loginJobResult(KJob*)) );
      loginJob->start();
      break;
    }
  case List:
    {
      kDebug() << "================ Starting state List ===========================";
      emit status( Running, i18n( "Fetching mail listing." ) );
      ListJob *listJob = new ListJob( mPopSession );
      connect( listJob, SIGNAL(result(KJob*)),
               this, SLOT(listJobResult(KJob*)) );
      listJob->start();
    }
    break;
  case UIDList:
    {
      kDebug() << "================ Starting state UIDList ========================";
      UIDListJob *uidListJob = new UIDListJob( mPopSession );
      connect( uidListJob, SIGNAL(result(KJob*)),
               this, SLOT(uidListJobResult(KJob*)) );
      uidListJob->start();
    }
    break;
  case Download:
    {
      kDebug() << "================ Starting state Download =======================";
      FetchJob *fetchJob = new FetchJob( mPopSession );

      // Determine which mails we want to download. Those are all mails which are
      // currently on ther server, minus the ones we have already downloaded (we
      // remember which UIDs we have downloaded in the settings)
      QList<int> idsToDownload = mIdsToSizeMap.keys();
      const QList<QString> UIDsOnServer = mIdsToUidsMap.values();
      const QList<QString> alreadyDownloadedUIDs = Settings::self()->seenUidList();
      foreach( const QString &uidOnServer, UIDsOnServer ) {
        if ( alreadyDownloadedUIDs.contains( uidOnServer ) ) {
          const int idOfUIDOnServer = mUidsToIdsMap.value( uidOnServer, -1 );
          Q_ASSERT( idOfUIDOnServer != -1 );
          idsToDownload.removeAll( idOfUIDOnServer );
        }
      }
      mIdsToDownload = idsToDownload;
      kDebug() << "We are going to download" << mIdsToDownload.size() << "messages";

      // For proper progress, the job needs to know the sizes of the messages, so
      // put them into a list here
      QList<int> sizesOfMessagesToDownload;
      foreach( int id, idsToDownload ) {
        sizesOfMessagesToDownload.append( mIdsToSizeMap.value( id ) );
      }

      fetchJob->setFetchIds( idsToDownload, sizesOfMessagesToDownload );
      connect( fetchJob, SIGNAL(result(KJob*)),
               this, SLOT(fetchJobResult(KJob*)) );
      connect( fetchJob, SIGNAL(messageFinished(int,KMime::Message::Ptr)),
               this, SLOT(messageFinished(int,KMime::Message::Ptr)) );
      connect( fetchJob, SIGNAL(processedAmount(KJob*,KJob::Unit,qulonglong)),
               this, SLOT(messageDownloadProgress(KJob*,KJob::Unit,qulonglong)) );
      fetchJob->start();
    }
    break;
  case Save:
    {
      kDebug() << "================ Starting state Save ===========================";
      kDebug() << mPendingCreateJobs.size() << "item create jobs are pending";
      if ( mPendingCreateJobs.size() > 0 )
        emit status( Running, i18n( "Saving downloaded messages." ) );

      // It can happen that the create job map is empty, for example if there was no
      // mail to download or if all ItemCreateJob's finished before reaching this
      // stage
      if ( mPendingCreateJobs.isEmpty() ) {
        advanceState( Delete );
      }
    }
    break;
  case Delete:
    {
      kDebug() << "================ Starting state Delete =========================";
      QList<int> idsToKill = idsToDelete();
      if ( !idsToKill.isEmpty() ) {
        emit status( Running, i18n( "Deleting messages from the server.") );
        DeleteJob *deleteJob = new DeleteJob( mPopSession );
        deleteJob->setDeleteIds( idsToKill );
        connect( deleteJob, SIGNAL(result(KJob*)),
                 this, SLOT(deleteJobResult(KJob*)) );
        deleteJob->start();
      }
      else {
        advanceState( Quit );
      }
    }
    break;
  case Quit:
    {
      kDebug() << "================ Starting state Quit ===========================";
      QuitJob *quitJob = new QuitJob( mPopSession );
      connect( quitJob, SIGNAL(result(KJob*)),
               this, SLOT(quitJobResult(KJob*)) );
      quitJob->start();
    }
    break;
  case SavePassword:
    {
      kDebug() << "================ Starting state SavePassword ===================";
      if ( !mSavePassword )
        finish();
      else {
        kDebug() << "Writing password back to the wallet.";
        emit status( Running, i18n( "Saving password to the wallet." ) );
        mWallet = Wallet::openWallet( Wallet::NetworkWallet(), winIdForDialogs(),
                                      Wallet::Asynchronous );
        if ( mWallet ) {
          connect( mWallet, SIGNAL(walletOpened(bool)),
                   this, SLOT(walletOpenedForSaving(bool)) );
        } else {
          finish();
        }
      }
      break;
    }
  }
}

void POP3Resource::localFolderRequestJobFinished( KJob *job )
{
  if ( job->error() ) {
    cancelSync( i18n( "Error while trying to get the local inbox folder, "
                      "aborting mail check." ) + QLatin1Char('\n') + job->errorString() );
    return;
  }
  if ( mTestLocalInbox ) {
    KMessageBox::information(0,
                             i18n("<qt>The folder you deleted was associated with the account "
                                  "<b>%1</b> which delivered mail into it. The folder the account "
                                  "delivers new mail into was reset to the main Inbox folder.</qt>", name()));
  }
  mTestLocalInbox = false;

  mTargetCollection = SpecialMailCollections::self()->defaultCollection( SpecialMailCollections::Inbox );
  Q_ASSERT( mTargetCollection.isValid() );
  advanceState( Precommand );
}

void POP3Resource::targetCollectionFetchJobFinished( KJob *job )
{
  if ( job->error() ) {
    if ( !mTestLocalInbox ) {
      mTestLocalInbox = true;
      Settings::self()->setTargetCollection(  -1  );
      advanceState( FetchTargetCollection );
      return;
    } else {
      cancelSync( i18n( "Error while trying to get the folder for incoming mail, "
                        "aborting mail check." ) + QLatin1Char('\n') + job->errorString() );
      mTestLocalInbox = false;
      return;
    }
  }
  mTestLocalInbox = false;
  Akonadi::CollectionFetchJob *fetchJob =
      dynamic_cast<Akonadi::CollectionFetchJob*>( job );
  Q_ASSERT( fetchJob );
  Q_ASSERT( fetchJob->collections().size() <= 1 );

  if ( fetchJob->collections().isEmpty() ) {
    cancelSync( i18n( "Could not find folder for incoming mail, aborting mail check.") );
    return;
  }
  else {
    mTargetCollection = fetchJob->collections().first();
    advanceState( Precommand );
  }
}

void POP3Resource::precommandResult( KJob *job )
{
  if ( job->error() ) {
    cancelSync( i18n( "Error while executing precommand." ) +
                QLatin1Char('\n') + job->errorString() );
    return;
  }
  else {
    advanceState( RequestPassword );
  }
}

void POP3Resource::loginJobResult( KJob *job )
{
  if ( job->error() ) {
    kDebug() << job->error() << job->errorText();
    if ( job->error() == KIO::ERR_COULD_NOT_LOGIN )
      mAskAgain = true;
    cancelSync( i18n( "Unable to login to the server %1.", Settings::self()->host() ) +
                QLatin1Char('\n') + job->errorString() );
  }
  else {
    advanceState( List );
  }
}

void POP3Resource::listJobResult( KJob *job )
{
  if ( job->error() ) {
    cancelSync( i18n( "Error while getting the list of messages on the server." ) +
                QLatin1Char('\n') + job->errorString() );
  }
  else {
    ListJob *listJob = dynamic_cast<ListJob*>( job );
    Q_ASSERT( listJob );
    mIdsToSizeMap = listJob->idList();
    kDebug() << "IdsToSizeMap:" << mIdsToSizeMap;
    advanceState( UIDList );
  }
}

void POP3Resource::uidListJobResult( KJob *job )
{
  if ( job->error() ) {
    cancelSync( i18n( "Error while getting list of unique mail identifiers from the server." ) +
                QLatin1Char('\n') + job->errorString() );
  }
  else {
    UIDListJob *listJob = dynamic_cast<UIDListJob*>( job );
    Q_ASSERT( listJob );
    mIdsToUidsMap = listJob->uidList();
    mUidsToIdsMap = listJob->idList();
    kDebug() << "IdToUidMap:" << mIdsToUidsMap;
    kDebug() << "UidToIdMap:" << mUidsToIdsMap;

    mUidListValid = !mIdsToUidsMap.isEmpty() || mIdsToSizeMap.isEmpty();
    if ( Settings::self()->leaveOnServer() && !mUidListValid ) {
      // FIXME: this needs a proper parent window
      KMessageBox::sorry( 0,
            i18n( "Your POP3 server (Account: %1) does not support "
                  "the UIDL command: this command is required to determine, in a reliable way, "
                  "which of the mails on the server KMail has already seen before;\n"
                  "the feature to leave the mails on the server will therefore not "
                  "work properly.", name() ) );
    }

    advanceState( Download );
  }
}

void POP3Resource::fetchJobResult( KJob *job )
{
  if ( job->error() ) {
    cancelSync( i18n( "Error while fetching mails from the server." ) +
                QLatin1Char('\n') + job->errorString() );
    return;
  }
  else {
    kDebug() << "Downloaded" << mDownloadedIDs.size() << "mails";

    if ( !mIdsToDownload.isEmpty() ) {
      kWarning() << "We did not download all messages, there are still some remaining "
                    "IDs, even though we requested their download:" << mIdsToDownload;
    }

    advanceState( Save );
  }
}

void POP3Resource::messageFinished( int messageId, KMime::Message::Ptr message )
{
  if ( mState != Download ) {
    // This can happen if the slave does not get notified in time about the fact
    // that the job was killed
    return;
  }

  //kDebug() << "Got message" << messageId
  //         << "with subject" << message->subject()->asUnicodeString();

  Akonadi::Item item;
  item.setMimeType( QLatin1String("message/rfc822") );
  item.setPayload<KMime::Message::Ptr>( message );

  Pop3ResourceAttribute *attr  = item.attribute<Pop3ResourceAttribute>( Akonadi::Entity::AddIfMissing );
  attr->setPop3AccountName( identifier() );
  // update status flags
  if ( KMime::isSigned( message.get() ) )
    item.setFlag( Akonadi::MessageFlags::Signed );
  if ( KMime::isEncrypted( message.get() ) )
    item.setFlag( Akonadi::MessageFlags::Encrypted );
  if ( KMime::isInvitation( message.get() ) )
    item.setFlag( Akonadi::MessageFlags::HasInvitation );
  if ( KMime::hasAttachment( message.get() ) )
    item.setFlag( Akonadi::MessageFlags::HasAttachment );

  ItemCreateJob *itemCreateJob = new ItemCreateJob( item, mTargetCollection );

  mPendingCreateJobs.insert( itemCreateJob, messageId );
  connect( itemCreateJob, SIGNAL(result(KJob*)),
           this, SLOT(itemCreateJobResult(KJob*)) );

  mDownloadedIDs.append( messageId );
  mIdsToDownload.removeAll( messageId );
}

void POP3Resource::messageDownloadProgress( KJob *job, KJob::Unit unit, qulonglong totalBytes )
{
  Q_UNUSED( totalBytes );
  Q_UNUSED( unit );
  Q_ASSERT( unit == KJob::Bytes );
  QString statusMessage;
  const int totalMessages = mIdsToDownload.size() + mDownloadedIDs.size();
  int bytesRemainingOnServer = 0;
  foreach( const QString &alreadyDownloadedUID, Settings::self()->seenUidList() ) {
    const int alreadyDownloadedID = mUidsToIdsMap.value( alreadyDownloadedUID, -1 );
    if ( alreadyDownloadedID != -1 )
      bytesRemainingOnServer += mIdsToSizeMap.value( alreadyDownloadedID );
  }

  if ( Settings::self()->leaveOnServer() && bytesRemainingOnServer > 0 ) {

    statusMessage = i18n( "Fetching message %1 of %2 (%3 of %4 KB) for %5 "
                          "(%6 KB remain on the server).",
                          mDownloadedIDs.size() + 1, totalMessages,
                          job->processedAmount( KJob::Bytes ) / 1024,
                          job->totalAmount( KJob::Bytes ) / 1024, name(),
                          bytesRemainingOnServer / 1024 );
  }
  else {
    statusMessage = i18n( "Fetching message %1 of %2 (%3 of %4 KB) for %5",
                          mDownloadedIDs.size() +1, totalMessages,
                          job->processedAmount( KJob::Bytes ) / 1024,
                          job->totalAmount( KJob::Bytes ) / 1024, name() );
  }
  emit status( Running, statusMessage );
  emit percent( job->percent() );
}

void POP3Resource::itemCreateJobResult( KJob *job )
{
  if ( mState != Download && mState != Save ) {
    // This can happen if the slave does not get notified in time about the fact
    // that the job was killed
    return;
  }

  ItemCreateJob *createJob = dynamic_cast<ItemCreateJob*>( job );
  Q_ASSERT( createJob );

  if ( job->error() ) {
    cancelSync( i18n( "Unable to store downloaded mails." ) +
                QLatin1Char('\n') + job->errorString() );
    return;
  }

  const int idOfMessageJustCreated = mPendingCreateJobs.value( createJob, -1 );
  Q_ASSERT( idOfMessageJustCreated != -1 );
  mPendingCreateJobs.remove( createJob );
  mIDsStored.append( idOfMessageJustCreated );
  //kDebug() << "Just stored message with ID" << idOfMessageJustCreated
  //         << "on the Akonadi server";

  // Have all create jobs finished? Go to the next state, then
  if ( mState == Save && mPendingCreateJobs.isEmpty() ) {
    advanceState( Delete );
  }
}

int POP3Resource::idToTime( int id ) const
{
  const QString uid = mIdsToUidsMap.value( id );
  if ( !uid.isEmpty() ) {
    const int index = Settings::self()->seenUidList().indexOf( uid );
    if ( index != -1 )
      return Settings::self()->seenUidTimeList().at( index );
  }

  // If we don't find any mail, either we have no UID, or it is not in the seen UID
  // list. In that case, we assume that the mail is new, i.e. from now
  return time( 0 );
}

int POP3Resource::idOfOldestMessage( QList<int> &idList ) const
{
  int timeOfOldestMessage = time( 0 ) + 999;
  int idOfOldestMessage = -1;
  foreach( int id, idList ) {
    const int idTime = idToTime( id );
    if ( idTime < timeOfOldestMessage ) {
      timeOfOldestMessage = idTime;
      idOfOldestMessage = id;
    }
  }
  Q_ASSERT( idList.isEmpty() || idOfOldestMessage != -1 );
  return idOfOldestMessage;
}

QList<int> POP3Resource::idsToDelete() const
{
  QList<int> idsToDeleteFromServer = mIdsToSizeMap.keys();
  QList<int> idsToSave;

  // Don't attempt to delete messages that weren't downloaded correctly
  foreach( int idNotDownloaded, mIdsToDownload )
    idsToDeleteFromServer.removeAll( idNotDownloaded );

  // By default, we delete all messages. But if we have "leave on server"
  // rules, we can save some messages.
  if ( Settings::self()->leaveOnServer() && !idsToDeleteFromServer.isEmpty() ) {

    // If the time-limited leave rule is checked, add the newer messages to
    // the list of messages to keep
    if ( Settings::self()->leaveOnServerDays() > 0 ) {
      const int secondsPerDay = 86400;
      time_t timeLimit = time( 0 ) - ( secondsPerDay * Settings::self()->leaveOnServerDays() );
      foreach( int idToDelete, idsToDeleteFromServer ) {
        const int msgTime = idToTime( idToDelete );
        if ( msgTime >= timeLimit ) {
          idsToSave.append( idToDelete );
        }
        else {
          kDebug() << "Message" << idToDelete << "is too old and will be deleted.";
        }
      }
    }

    // Otherwise, add all messages to the list of messages to keep - this may
    // be reduced in the following number-limited leave rule and size-limited
    // leave rule checks
    else {
      foreach ( int idToDelete, idsToDeleteFromServer ) {
        idsToSave.append( idToDelete );
      }
    }

    //
    // Delete more old messages if there are more than mLeaveOnServerCount
    //
    if ( Settings::self()->leaveOnServerCount() > 0 ) {
      const int numToDelete = idsToSave.count() - Settings::self()->leaveOnServerCount();
      if ( numToDelete > 0 && numToDelete < idsToSave.count() ) {
        // Get rid of the first numToDelete messages
        for ( int i = 0; i < numToDelete; i++ ) {
          idsToSave.removeAll( idOfOldestMessage( idsToSave ) );
        }
      }
      else if ( numToDelete >= idsToSave.count() )
        idsToSave.clear();
    }

    //
    // Delete more old messages until we're under mLeaveOnServerSize MBs
    //
    if ( Settings::self()->leaveOnServerSize() > 0 ) {
      const qint64 limitInBytes = Settings::self()->leaveOnServerSize() * ( 1024 * 1024 );
      qint64 sizeOnServerAfterDeletion = 0;
      foreach( int id, idsToSave ) {
        sizeOnServerAfterDeletion += mIdsToSizeMap.value( id );
      }
      while ( sizeOnServerAfterDeletion > limitInBytes ) {
        int oldestId = idOfOldestMessage( idsToSave );
        idsToSave.removeAll( oldestId );
        sizeOnServerAfterDeletion -= mIdsToSizeMap.value( oldestId );
      }
    }

    //
    // Now save the messages from deletion
    //
    foreach( int idToSave, idsToSave ) {
      idsToDeleteFromServer.removeAll( idToSave );
    }
  }

  kDebug() << "Going to delete" << idsToDeleteFromServer.size() << idsToDeleteFromServer;
  return idsToDeleteFromServer;
}

void POP3Resource::deleteJobResult( KJob *job )
{
  if ( job->error() ) {
    cancelSync( i18n( "Failed to delete the messages from the server.") +
                QLatin1Char('\n') + job->errorString() );
    return;
  }

  DeleteJob *deleteJob = dynamic_cast<DeleteJob*>( job );
  Q_ASSERT( deleteJob );
  mDeletedIDs = deleteJob->deletedIDs();

  // Remove all deleted messages from the list of already downloaded messages,
  // as it is no longer necessary to store them (they just waste space)
  QList<QString> seenUIDs = Settings::self()->seenUidList();
  QList<int> timeOfSeenUids = Settings::self()->seenUidTimeList();
  Q_ASSERT( seenUIDs.size() == timeOfSeenUids.size() );
  foreach( int deletedId, mDeletedIDs ) {
    QString deletedUID = mIdsToUidsMap.value( deletedId );
    if ( !deletedUID.isEmpty() ) {
      int index = seenUIDs.indexOf( deletedUID );
      if ( index != -1 ) {
        // TEST
        kDebug() << "Removing UID" << deletedUID << "from the seen UID list, as it was deleted.";
        seenUIDs.removeAt( index );
        timeOfSeenUids.removeAt( index );
      }
    }
  }
  Settings::self()->setSeenUidList( seenUIDs );
  Settings::self()->setSeenUidTimeList( timeOfSeenUids );
  Settings::self()->writeConfig(),

  advanceState( Quit );
}

void POP3Resource::finish()
{
  kDebug() << "================= Mail check finished. =============================";
  saveSeenUIDList();
  if ( !mIntervalCheckInProgress )
    collectionsRetrieved( Akonadi::Collection::List() );
  if ( mDownloadedIDs.isEmpty() )
    emit status( Idle, i18n( "Finished mail check, no message downloaded." ) );
  else
    emit status( Idle, i18np( "Finished mail check, 1 message downloaded.",
                              "Finished mail check, %1 messages downloaded.",
                              mDownloadedIDs.size() ) );

  resetState();
}

void POP3Resource::quitJobResult( KJob *job )
{
  if ( job->error() ) {
    cancelSync( i18n( "Unable to complete the mail fetch." ) +
                QLatin1Char('\n') + job->errorString() );
    return;
  }

  advanceState( SavePassword );
}

void POP3Resource::slotSessionError( int errorCode, const QString &errorMessage )
{
  kWarning() << "Error in our session, unrelated to a currently running job!";
  cancelSync( KIO::buildErrorString( errorCode, errorMessage ) );
}

void POP3Resource::saveSeenUIDList()
{
  QList<QString> seenUIDs = Settings::self()->seenUidList();
  QList<int> timeOfSeenUIDs = Settings::self()->seenUidTimeList();

  //
  // Find the messages that we have successfully stored, but did not actually get
  // deleted.
  // Those messages, we have to remember, so we don't download them again.
  //
  QList<int> idsOfMessagesDownloadedButNotDeleted = mIDsStored;
  foreach( int deletedId, mDeletedIDs )
    idsOfMessagesDownloadedButNotDeleted.removeAll( deletedId );
  QList<QString> uidsOfMessagesDownloadedButNotDeleted;
  foreach( int id, idsOfMessagesDownloadedButNotDeleted ) {
    QString uid = mIdsToUidsMap.value( id );
    if ( !uid.isEmpty() ) {
      uidsOfMessagesDownloadedButNotDeleted.append( uid );
    }
  }
  Q_ASSERT( seenUIDs.size() == timeOfSeenUIDs.size() );
  foreach( const QString &uid, uidsOfMessagesDownloadedButNotDeleted ) {
    if ( !seenUIDs.contains( uid ) ) {
      seenUIDs.append( uid );
      timeOfSeenUIDs.append( time( 0 ) );
    }
  }

  //
  // If we have a valid UID list from the server, delete those UIDs that are in
  // the seenUidList but are not on the server.
  // This can happen if someone else than this resource deleted the mails from the
  // server which we kept here.
  //
  if ( mUidListValid ) {
    QList<QString>::iterator uidIt = seenUIDs.begin();
    QList<int>::iterator timeIt = timeOfSeenUIDs.begin();
    while ( uidIt != seenUIDs.end() ) {
      const QString curSeenUID = *uidIt;
      if ( !mUidsToIdsMap.contains( curSeenUID ) ) {
        // Ok, we have a UID in the seen UID list that is not anymore on the server.
        // Therefore remove it from the seen UID list, it is not needed there anymore,
        // it just wastes space.
        uidIt = seenUIDs.erase( uidIt );
        timeIt = timeOfSeenUIDs.erase( timeIt );
      }
      else {
        ++uidIt;
        ++timeIt;
      }
    }
  }
  else
    kWarning() << "UID list from server is not valid.";


  //
  // Now save it in the settings
  //
  kDebug() << "The seen UID list has" << seenUIDs.size() << "entries";
  Settings::self()->setSeenUidList( seenUIDs );
  Settings::self()->setSeenUidTimeList( timeOfSeenUIDs );
  Settings::self()->writeConfig();
}

void POP3Resource::cancelSync( const QString &errorMessage, bool error )
{
  if ( error ) {
    cancelTask( errorMessage );
    kWarning() << "============== ERROR DURING POP3 SYNC ==========================";
    kWarning() << errorMessage;
  }
  else {
    kDebug() << "Canceled the sync, but no error.";
    cancelTask();
  }
  saveSeenUIDList();
  resetState();
}

void POP3Resource::resetState()
{
  mState = Idle;
  mTargetCollection = Collection( -1 );
  mIdsToSizeMap.clear();
  mIdsToUidsMap.clear();
  mUidsToIdsMap.clear();
  mDownloadedIDs.clear();
  mIdsToDownload.clear();
  mPendingCreateJobs.clear();
  mIDsStored.clear();
  mDeletedIDs.clear();
  mUidListValid = false;
  mIntervalCheckInProgress = false;
  mSavePassword = false;
  updateIntervalTimer();
  delete mWallet;
  mWallet = 0;

  if ( mPopSession ) {
    // Closing the POP session means the KIO slave will get disconnected, which
    // automatically issues the QUIT command.
    // Delete the POP session later, otherwise the scheduler makes us crash
    mPopSession->abortCurrentJob();
    mPopSession->deleteLater();
    mPopSession = 0;
  }
}

void POP3Resource::startMailCheck()
{
  resetState();
  mIntervalTimer->stop();
  emit percent( 0 ); // Otherwise the value from the last sync is taken
  advanceState( FetchTargetCollection );
}

void POP3Resource::retrieveCollections()
{
  if ( mState == Idle ) {
    startMailCheck();
  }
  else {
    cancelSync(
        i18n( "Mail check already in progress, unable to start a second check." ) );
  }
}

void POP3Resource::clearCachedPassword()
{
    mPassword.clear();
}

void POP3Resource::cleanup()
{
  if (mWallet && mWallet->isOpen() && mWallet->hasFolder( QLatin1String("pop3") ) ) {
    mWallet->setFolder( QLatin1String("pop3") );
    if ( mWallet->hasEntry( identifier() ) )
        mWallet->removeEntry(identifier());
    }
}

void POP3Resource::doSetOnline( bool online )
{
  ResourceBase::doSetOnline( online );
  if ( online ) {
    emit status( Idle, i18n("Ready") );
  } else {
    if ( mState != Idle ) {
      cancelSync( i18n( "Mail check aborted after going offline." ), false /* no error */ );
    }
    emit status( Idle, i18n("Offline") );
    delete mWallet;
    mWallet = 0;
    clearCachedPassword();
  }
}

AKONADI_RESOURCE_MAIN( POP3Resource )
