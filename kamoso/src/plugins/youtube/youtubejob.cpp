/*************************************************************************************
 *  Copyright (C) 2008-2011 by Aleix Pol <aleixpol@kde.org>                          *
 *  Copyright (C) 2008-2011 by Alex Fiestas <afiestas@kde.org>                       *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "youtubejob.h"
#include "ui_videoInfo.h"

#include <QApplication>

#include <KDebug>
#include <KIO/Job>
#include <KUrl>
#include <KIcon>
#include <KToolInvocation>
#include <KLocalizedString>

const QByteArray YoutubeJob::developerKey("AI39si41ZFrIJoZGNH0hrZPhMuUlwHc6boMLi4e-_W6elIzVUIeDO9F7ix2swtnGAiKT4yc4F4gQw6yysTGvCn1lPNyli913Xg");

using KWallet::Wallet;
YoutubeJob::YoutubeJob(const KUrl& url, QObject* parent)
    : KJob(parent), m_authToken(0), m_url(url), dialog(0)
{
}

void YoutubeJob::start()
{
    kDebug() << "Job started";
    checkWallet();
}

void YoutubeJob::checkWallet()
{
    WId windowId = 0;
    if (qApp->activeWindow()) {
        windowId = qApp->activeWindow()->winId();
    }
    m_wallet = Wallet::openWallet(Wallet::NetworkWallet(), windowId);
    if(m_wallet != NULL){
        if(!m_wallet->hasFolder("youtubeKamoso")){
            if(!m_wallet->createFolder("youtubeKamoso")){
                //TODO: Error reporting here
                return;
            }
        }
        m_wallet->setFolder("youtubeKamoso");
    }

    if(!showDialog()){
        emit emitResult();
        return;
    }
    login();
}

void YoutubeJob::fileOpened(KIO::Job *job, const QByteArray &data)
{
    kDebug() << "fileOPened!!";
    job->suspend();
    #warning do something to evade the cast? like adding a metadata?
    KIO::SimpleJob *simpleJob = static_cast<KIO::SimpleJob*>(job);

    disconnect(job,SIGNAL(data(KIO::Job*,QByteArray)),this,SLOT(fileOpened(KIO::Job*,QByteArray)));
    connect(job,SIGNAL(data(KIO::Job*,QByteArray)),this,SLOT(moreData(KIO::Job*,QByteArray)));

    QByteArray extraHeaders("");
    extraHeaders.append("Authorization: GoogleLogin auth=");
    extraHeaders.append(m_authToken.data());
    extraHeaders.append("\r\n");
    extraHeaders.append("GData-Version: 2");
    extraHeaders.append("\r\n");
    extraHeaders.append("X-GData-Key: key=");
    extraHeaders.append(developerKey);
    extraHeaders.append("\r\n");
    extraHeaders.append("Slug: ");
    // extraHeaders.append(simpleJob->url().fileName());

    QByteArray finalData("--foobarfoo");
    finalData.append("\r\n");
    finalData.append("Content-Type: application/atom+xml; charset=UTF-8");
    finalData.append("\r\n");
    finalData.append("\r\n");
    finalData.append("<?xml version=\"1.0\"?>\r\n");
    finalData.append("<entry xmlns=\"http://www.w3.org/2005/Atom\"\r\n");
    finalData.append("xmlns:media=\"http://search.yahoo.com/mrss/\"\r\n");
    finalData.append("xmlns:yt=\"http://gdata.youtube.com/schemas/2007\">\r\n");
    // finalData.append("<media:group>\r\n");
    // finalData.append("<media:title type=\"plain\">"+m_videoInfo["videoTitle"]+"</media:title>\r\n");
    finalData.append("<media:description type=\"plain\">\r\n");
    // finalData.append(m_videoInfo["videoDesc"]+"\r\n");
    finalData.append("</media:description>\r\n");
    finalData.append("<media:category\r\n");
    finalData.append("scheme=\"http://gdata.youtube.com/schemas/2007/categories.cat\">People\r\n");
    finalData.append("</media:category>\r\n");
    // finalData.append("<media:keywords>"+m_videoInfo["videoTags"]+"</media:keywords>\r\n");
    finalData.append("</media:group>\r\n");
    finalData.append("</entry>");
    finalData.append("\r\n");
    finalData.append("--foobarfoo");
    finalData.append("\r\n");
    finalData.append("Content-Type: video/ogg");
    finalData.append("\r\n");
    finalData.append("Content-Transfer-Encoding: binary");
    finalData.append("\r\n");
    finalData.append("\r\n");
    finalData.append(data);
//     kDebug() << finalData;
    KUrl url("http://uploads.gdata.youtube.com/feeds/api/users/default/uploads");
    uploadJob = KIO::http_post(url,finalData,KIO::HideProgressInfo);
    uploadJob->addMetaData("cookies","none");
    uploadJob->addMetaData("connection","close");
    uploadJob->addMetaData("customHTTPHeader",extraHeaders.data());
    uploadJob->addMetaData("content-type","Content-Type: multipart/related; boundary=\"foobarfoo\"");
    uploadJob->setAsyncDataEnabled(true);
    connect(uploadJob,SIGNAL(dataReq(KIO::Job*,QByteArray&)),this,SLOT(uploadNeedData()));
    connect(uploadJob,SIGNAL(data(KIO::Job*,QByteArray)),this,SLOT(uploadDone(KIO::Job*,QByteArray)));
    uploadJob->start();
}

void YoutubeJob::moreData(KIO::Job *job, const QByteArray &data)
{
    job->suspend();
    if(data.size() == 0){
        kDebug() << "Data is zero, going to end this!";
        disconnect(uploadJob,SIGNAL(dataReq(KIO::Job*,QByteArray&)),this,SLOT(uploadNeedData()));
        connect(uploadJob,SIGNAL(dataReq(KIO::Job*,QByteArray&)),this,SLOT(uploadFinal()));

        QByteArray final("\r\n");
        final.append("--foobarfoo--");
        uploadJob->sendAsyncData(final);
    }else{
        kDebug() << "Sending more data....";
//         kDebug() << data;
        uploadJob->sendAsyncData(data);
    }
}

void YoutubeJob::uploadFinal()
{
    //Sending an empty QByteArray the job ends
    kDebug() << "Sendind the empty packed";
    uploadJob->sendAsyncData(QByteArray());
}

void YoutubeJob::uploadNeedData()
{
    kDebug() << "openFile job resumed!";
    openFileJob->resume();
}

void YoutubeJob::uploadDone(KIO::Job *job, const QByteArray &data)
{
    kDebug() << "Upload Response";
//     kDebug() << data.data();
    QString dataStr(data);
    QRegExp rx("<media:player url='(\\S+)'/>");
    dataStr.contains(rx);
//     kDebug() << rx.cap(1);
    KUrl url = rx.cap(1);
    if (!url.isEmpty()) {
        kDebug() << "Url : " << url.url();
        job->kill();
        KToolInvocation::invokeBrowser(url.url());
        emit emitResult();
    }
}

QMap<QString, QString> YoutubeJob::showVideoDialog()
{
    Ui::videoForm *videoForm = new Ui::videoForm;
    QWidget *videoWidget = new QWidget();
    videoForm->setupUi(videoWidget);

    KDialog *dialog = new KDialog();
    dialog->setMainWidget(videoWidget);
    dialog->setCaption(i18n("Video information:"));
    dialog->setButtons(KDialog::Ok);
    dialog->setMinimumWidth(425);
    dialog->setMinimumHeight(315);
    dialog->setMaximumWidth(425);
    dialog->setMaximumHeight(315);
    videoForm->titleText->setFocus();
    videoForm->titleText->setText(i18n("Kamoso %1", QDate::currentDate().toString(Qt::ISODate)));
    int response = dialog->exec();

    QMap<QString, QString> videoInfo;
    if(response == QDialog::Accepted){
        if(!videoForm->descriptionText->toPlainText().isEmpty()){
            videoInfo["videoDesc"] = videoForm->descriptionText->toPlainText();
        } else {
            videoInfo["videoDesc"] = i18n("This video has been recorded using Kamoso, a KDE software to play with webcams!");
        }
        if(!videoForm->titleText->text().isEmpty()){
            videoInfo["videoTitle"] = videoForm->titleText->text();
        }
        if(!videoForm->tagText->text().isEmpty()){
            videoInfo["videoTags"] = videoForm->tagText->text();
        } else {
            videoInfo["videoTags"] = "KDE, Kamoso";
        }
    }
    return videoInfo;
}
void YoutubeJob::login()
{
    QMap<QString, QString> authInfo;
    authInfo["username"] = dialog->username();
    authInfo["password"] = dialog->password();

    KUrl url("https://www.google.com/youtube/accounts/ClientLogin");
    QByteArray data("Email=");
    data.append(authInfo["username"].toAscii());
    data.append("&Passwd=");
    data.append(authInfo["password"].toAscii());
    data.append("&service=youtube&source=Kamoso");
    KIO::TransferJob *loginJob = KIO::http_post(url,data,KIO::HideProgressInfo);
    loginJob->addMetaData("cookies","none");
    loginJob->addMetaData("content-type","Content-Type:application/x-www-form-urlencoded");
    connect(loginJob,SIGNAL(data(KIO::Job*,QByteArray)),this,SLOT(loginDone(KIO::Job*,QByteArray)));
    loginJob->start();
}

void YoutubeJob::loginDone(KIO::Job *job, const QByteArray &data)
{
    delete job;
    kDebug() << "LoginDone, data received\n";
//     kDebug() << data.data();
    if(data.at(0) == 'E'){
        authenticated(false);
    }else{
        QList<QByteArray> tokens = data.split('\n');
        m_authToken = tokens.first().remove(0,5);
        kDebug() << "Final AuthToken: " << m_authToken.data();
        authenticated(true);
    }
}

bool YoutubeJob::showDialog()
{
    QString server = QString("http://www.youtube.com");

    if(m_wallet != NULL) {
        dialog = new KPasswordDialog(0L,KPasswordDialog::ShowKeepPassword | KPasswordDialog::ShowUsernameLine);
        QMap<QString, QString> authInfo;
        m_wallet->readMap("youtubeAuth",authInfo);
        dialog->setPassword(authInfo["password"]);
        dialog->setUsername(authInfo["username"]);
        dialog->setKeepPassword(true);
    }else{
        dialog = new KPasswordDialog(0L,KPasswordDialog::ShowUsernameLine);
    }
    dialog->setPrompt(i18n("You need to supply a username and a password to be able to upload videos to YouTube"));
    dialog->addCommentLine(i18n("Server:"),server);
    dialog->setCaption(i18n("Authentication for YouTube"));

    int response = dialog->exec();
    if(response == QDialog::Rejected){
        return false;
    }
    while((dialog->username() == "" || dialog->password() == "") && response == QDialog::Accepted )
    {
        response = dialog->exec();
        if(response == QDialog::Rejected){
            return false;
        }
    }

    if(dialog->keepPassword() == true &&  m_wallet != NULL) {
        QMap<QString, QString> toSave;
        toSave["username"] = dialog->username();
        toSave["password"] = dialog->password();
        m_wallet->writeMap("youtubeAuth",toSave);
        m_wallet->sync();
    }
    return true;
}

void YoutubeJob::authenticated(bool auth)
{
    kDebug() << "Authentication: " << auth ;
    if(auth == false){
        if(showDialog()){
            login();
        }
        return;
    }
    QMap<QString, QString> videoInfo;
    m_videoInfo = showVideoDialog();
    kDebug() << "File To Upload: " << m_url.path();
    openFileJob = KIO::get(m_url,KIO::NoReload,KIO::HideProgressInfo);
    connect(openFileJob,SIGNAL(data(KIO::Job*,QByteArray)),this,SLOT(fileOpened(KIO::Job*,QByteArray)));
    openFileJob->start();
}
