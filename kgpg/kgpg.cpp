/*  This file is part of the KDE project
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

#include <QScopedPointer>
#include <klocale.h>
#include <kmimetype.h>
#include <kpassworddialog.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include "kgpg.h"

KGPG::KGPG(QWidget *parent)
    : KMainWindow(parent),
    m_mode(KGPG::EncryptMode),
    m_release(false)
{
    m_ui.setupUi(this);
    m_ui.startbutton->setEnabled(false);
    m_ui.progressbar->setVisible(false);

    // required by context
    kDebug() << gpgme_check_version(NULL);

    gpgme_error_t gpgresult = gpgme_new(&m_gpgctx);
    if (gpgresult != 0) {
        setError(gpgme_strerror(gpgresult));
        return;
    }
    m_release = true;

    gpgme_set_pinentry_mode(m_gpgctx, GPGME_PINENTRY_MODE_LOOPBACK); // for password callback
    gpgme_set_passphrase_cb(m_gpgctx, KGPG::gpgPasswordCallback, this);
    gpgme_set_progress_cb(m_gpgctx, KGPG::gpgProgressCallback, this);

    connect(m_ui.keysbox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotKeysBox(int)));
    connect(m_ui.startbutton, SIGNAL(clicked()), this, SLOT(slotStart()));

    connect(m_ui.actionQuit, SIGNAL(triggered()), this, SLOT(slotQuit()));
    connect(m_ui.actionEncrypt, SIGNAL(triggered()), this, SLOT(slotEncryptMode()));
    connect(m_ui.actionDecrypt, SIGNAL(triggered()), this, SLOT(slotDecryptMode()));
    connect(m_ui.actionSign, SIGNAL(triggered()), this, SLOT(slotSignMode()));
    connect(m_ui.actionVerify, SIGNAL(triggered()), this, SLOT(slotVerifyMode()));
}

KGPG::~KGPG()
{
    // will crash if not initialized
    if (m_release) {
        gpgme_release(m_gpgctx);
    }
}

void KGPG::setMode(const KGPGMode mode)
{
    m_mode = mode;

    m_ui.actionEncrypt->setChecked(false);
    m_ui.actionDecrypt->setChecked(false);
    m_ui.actionSign->setChecked(false);
    m_ui.actionVerify->setChecked(false);

    switch (mode) {
        case KGPG::EncryptMode: {
            updateKeys(GPGME_KEYLIST_MODE_LOCAL, true);
            m_ui.startbutton->setEnabled(!m_keys.isEmpty());
            m_ui.destinationrequester->setVisible(true);
            m_ui.actionEncrypt->setChecked(true);
            break;
        }
        case KGPG::DecryptMode: {
            updateKeys(GPGME_KEYLIST_MODE_LOCAL, false);
            m_ui.startbutton->setEnabled(!m_keys.isEmpty());
            m_ui.destinationrequester->setVisible(true);
            m_ui.actionDecrypt->setChecked(true);
            break;
        }
        case KGPG::SignMode: {
            updateKeys(GPGME_KEYLIST_MODE_LOCAL | GPGME_KEYLIST_MODE_SIGS, true);
            m_ui.startbutton->setEnabled(!m_keys.isEmpty());
            m_ui.destinationrequester->setVisible(true);
            m_ui.actionSign->setChecked(true);
            break;
        }
        case KGPG::VerifyMode: {
            updateKeys(GPGME_KEYLIST_MODE_LOCAL | GPGME_KEYLIST_MODE_SIGS, false);
            m_ui.startbutton->setEnabled(true);
            m_ui.destinationrequester->setVisible(false);
            m_ui.actionVerify->setChecked(true);
            break;
        }
        default: {
            Q_ASSERT(false);
            break;
        }
    }
    
}

void KGPG::setSource(const QString &source)
{
    const KUrl sourceurl(source);
    // TODO: invalid source or destination URL should disable start button
    switch (m_mode) {
        case KGPG::EncryptMode: {
            gpgme_set_armor(m_gpgctx, 0);

            QString destinationstring = sourceurl.prettyUrl();
            destinationstring.append(QLatin1String(".gpg"));
            m_ui.sourcerequester->setUrl(sourceurl);
            m_ui.destinationrequester->setUrl(KUrl(destinationstring));
            break;
        }
        case KGPG::DecryptMode: {
            gpgme_set_armor(m_gpgctx, 0);

            QString destinationstring = sourceurl.prettyUrl();
            if (destinationstring.endsWith(QLatin1String(".gpg"))) {
                destinationstring.chop(4);
            }
            m_ui.sourcerequester->setFilter(QString::fromLatin1("application/pgp-encrypted"));
            m_ui.destinationrequester->setFilter(QString());
            m_ui.sourcerequester->setUrl(sourceurl);
            m_ui.destinationrequester->setUrl(KUrl(destinationstring));
            break;
        }
        case KGPG::SignMode: {
            gpgme_set_armor(m_gpgctx, 1);

            QString destinationstring = sourceurl.prettyUrl();
            destinationstring.append(QLatin1String(".asc"));
            m_ui.sourcerequester->setFilter(QString());
            m_ui.destinationrequester->setFilter(QString::fromLatin1("application/pgp-signature"));
            m_ui.sourcerequester->setUrl(sourceurl);
            m_ui.destinationrequester->setUrl(KUrl(destinationstring));
            break;
        }
        case KGPG::VerifyMode: {
            gpgme_set_armor(m_gpgctx, 1);

            m_ui.sourcerequester->setFilter(QString::fromLatin1("application/pgp-signature"));
            m_ui.sourcerequester->setUrl(sourceurl);
            break;
        }
        default: {
            Q_ASSERT(false);
            break;
        }
    }
}

void KGPG::setError(const char* const error)
{
    setError(QString::fromLocal8Bit(error));
}
void KGPG::setError(const QString &error)
{
    kWarning() << error;

    const QString errormessage = i18n("Error: %1", error);
    m_ui.statusbar->showMessage(errormessage);
    KMessageBox::error(this, errormessage);

    m_ui.progressbar->setMinimum(0);
    m_ui.progressbar->setMaximum(100);
    m_ui.progressbar->setVisible(false);
    m_ui.startbutton->setVisible(true);
}

void KGPG::setProgress(const int gpgcurrent, const int gpgtotal)
{
    m_ui.progressbar->setMaximum(gpgtotal);
    m_ui.progressbar->setValue(gpgcurrent);
}

gpgme_error_t KGPG::gpgPasswordCallback(void *opaque, const char *uid_hint,
                                        const char *passphrase_info,
                                        int prev_was_bad, int fd)
{
    // qDebug() << Q_FUNC_INFO << uid_hint << passphrase_info << prev_was_bad << fd;

    QScopedPointer<KPasswordDialog> kpassdialog(new KPasswordDialog());
    kpassdialog->setPrompt(i18n("Enter a password"));
    if (!kpassdialog->exec()) {
        return 1;
    }

    const QByteArray bytepassword = kpassdialog->password().toLocal8Bit() + "\n";
    ssize_t gpgresult = 0;
    do {
        gpgresult = gpgme_io_write(fd, bytepassword.constData(), bytepassword.size());
    } while (gpgresult < 0 && errno == EAGAIN);

    if (gpgresult != bytepassword.size()) {
        return gpgme_error_from_errno(errno);
    }
    return 0;
}

void KGPG::gpgProgressCallback(void *opaque, const char *what,
                               int type, int current, int total)
{
    // qDebug() << Q_FUNC_INFO << what << type << current << total;
    KGPG *kgpg = static_cast<KGPG*>(opaque);
    kgpg->setProgress(current, total);
}

void KGPG::start()
{
    m_ui.progressbar->setMinimum(0);
    m_ui.progressbar->setMaximum(0);
    m_ui.progressbar->setVisible(true);
    m_ui.startbutton->setVisible(false);

    switch (m_mode) {
        case KGPG::EncryptMode: {
            if (m_keys.isEmpty()) {
                setError("No key available");
                break;
            }

            const QByteArray kgpgkeyfpr = m_keys.at(m_ui.keysbox->currentIndex()).fpr;
            const QByteArray gpginputfile = m_ui.sourcerequester->url().toLocalFile().toLocal8Bit();

            gpgme_data_t gpgindata;
            gpgme_error_t gpgresult = gpgme_data_new_from_file(&gpgindata, gpginputfile.constData(), 1);
            if (gpgresult != 0) {
                setError(gpgme_strerror(gpgresult));
                break;
            }

            gpgme_key_t gpgencryptkey;
            gpgresult = gpgme_get_key(m_gpgctx, kgpgkeyfpr.constData(), &gpgencryptkey, 1);
            if (gpgresult != 0) {
                setError(gpgme_strerror(gpgresult));
                gpgme_data_release(gpgindata);
                break;
            }

            gpgme_data_t gpgoutdata;
            gpgresult = gpgme_data_new(&gpgoutdata);
            if (gpgresult != 0) {
                setError(gpgme_strerror(gpgresult));
                gpgme_data_release(gpgindata);
                break;
            }

            gpgme_key_t gpgkeys[2] = { gpgencryptkey, NULL };
            gpgresult = gpgme_op_encrypt(m_gpgctx, gpgkeys, GPGME_ENCRYPT_ALWAYS_TRUST, gpgindata, gpgoutdata);
            if (gpgresult != 0) {
                setError(gpgme_strerror(gpgresult));
                gpgme_data_release(gpgindata);
                gpgme_data_release(gpgoutdata);
                break;
            }

            size_t gpgbuffersize = 0;
            char* gpgbuffer = gpgme_data_release_and_get_mem(gpgoutdata, &gpgbuffersize);

            const QString outputfile = m_ui.destinationrequester->url().toLocalFile();
            QFile encryptedfile(outputfile);
            if (!encryptedfile.open(QFile::WriteOnly)) {
                setError(encryptedfile.errorString());
                gpgme_free(gpgbuffer);
                gpgme_data_release(gpgindata);
                break;
            }

            if (encryptedfile.write(gpgbuffer, gpgbuffersize) != gpgbuffersize) {
                setError(encryptedfile.errorString());
                gpgme_free(gpgbuffer);
                gpgme_data_release(gpgindata);
                break;
            }
            // qDebug() << Q_FUNC_INFO << "encrypted" << gpgbuffer;

            gpgme_free(gpgbuffer);
            gpgme_data_release(gpgindata);
            break;
        }
        case KGPG::DecryptMode: {
            if (m_keys.isEmpty()) {
                setError("No key available");
                break;
            }

            const QByteArray gpginputfile = m_ui.sourcerequester->url().toLocalFile().toLocal8Bit();

            gpgme_data_t gpgindata;
            gpgme_error_t gpgresult = gpgme_data_new_from_file(&gpgindata, gpginputfile.constData(), 1);
            if (gpgresult != 0) {
                setError(gpgme_strerror(gpgresult));
                break;
            }
    
            gpgme_data_t gpgoutdata;
            gpgresult = gpgme_data_new(&gpgoutdata);
            if (gpgresult != 0) {
                setError(gpgme_strerror(gpgresult));
                gpgme_data_release(gpgindata);
                gpgme_data_release(gpgoutdata);
                break;
            }

            gpgresult = gpgme_op_decrypt(m_gpgctx, gpgindata, gpgoutdata);
            if (gpgresult != 0) {
                setError(gpgme_strerror(gpgresult));
                gpgme_data_release(gpgindata);
                gpgme_data_release(gpgoutdata);
                break;
            }

            size_t gpgbuffersize = 0;
            char* gpgbuffer = gpgme_data_release_and_get_mem(gpgoutdata, &gpgbuffersize);

            const QString outputfile = m_ui.destinationrequester->url().toLocalFile();
            QFile encryptedfile(outputfile);
            if (!encryptedfile.open(QFile::WriteOnly)) {
                setError(encryptedfile.errorString());
                gpgme_free(gpgbuffer);
                gpgme_data_release(gpgindata);
                break;
            }

            if (encryptedfile.write(gpgbuffer, gpgbuffersize) != gpgbuffersize) {
                setError(encryptedfile.errorString());
                gpgme_free(gpgbuffer);
                gpgme_data_release(gpgindata);
                break;
            }
            // qDebug() << Q_FUNC_INFO << "decryption" << gpgbuffer;

            gpgme_free(gpgbuffer);
            gpgme_data_release(gpgindata);
            break;
        }
        case KGPG::SignMode: {
            if (m_keys.isEmpty()) {
                setError("No key available");
                break;
            }

            const QByteArray kgpgkeyfpr = m_keys.at(m_ui.keysbox->currentIndex()).fpr;
            const QByteArray gpginputfile = m_ui.sourcerequester->url().toLocalFile().toLocal8Bit();

            gpgme_data_t gpgindata;
            gpgme_error_t gpgresult = gpgme_data_new_from_file(&gpgindata, gpginputfile.constData(), 1);
            if (gpgresult != 0) {
                setError(gpgme_strerror(gpgresult));
                break;
            }

            gpgme_key_t gpgencryptkey;
            gpgresult = gpgme_get_key(m_gpgctx, kgpgkeyfpr.constData(), &gpgencryptkey, 1);
            if (gpgresult != 0) {
                setError(gpgme_strerror(gpgresult));
                gpgme_data_release(gpgindata);
                break;
            }

            gpgme_data_t gpgoutdata;
            gpgresult = gpgme_data_new(&gpgoutdata);
            if (gpgresult != 0) {
                setError(gpgme_strerror(gpgresult));
                gpgme_data_release(gpgindata);
                break;
            }

            gpgme_key_t gpgkeys[2] = { gpgencryptkey, NULL };
            // the only important difference from the code for KGPG::EncryptMode mode is the function call bellow
            gpgresult = gpgme_op_encrypt_sign(m_gpgctx, gpgkeys, GPGME_ENCRYPT_ALWAYS_TRUST, gpgindata, gpgoutdata);
            if (gpgresult != 0) {
                setError(gpgme_strerror(gpgresult));
                gpgme_data_release(gpgindata);
                gpgme_data_release(gpgoutdata);
                break;
            }

            size_t gpgbuffersize = 0;
            char* gpgbuffer = gpgme_data_release_and_get_mem(gpgoutdata, &gpgbuffersize);

            const QString outputfile = m_ui.destinationrequester->url().toLocalFile();
            QFile encryptedfile(outputfile);
            if (!encryptedfile.open(QFile::WriteOnly)) {
                setError(encryptedfile.errorString());
                gpgme_free(gpgbuffer);
                gpgme_data_release(gpgindata);
                break;
            }

            if (encryptedfile.write(gpgbuffer, gpgbuffersize) != gpgbuffersize) {
                setError(encryptedfile.errorString());
                gpgme_free(gpgbuffer);
                gpgme_data_release(gpgindata);
                break;
            }
            // qDebug() << Q_FUNC_INFO << "sign" << gpgbuffer;

            gpgme_free(gpgbuffer);
            gpgme_data_release(gpgindata);
            break;
        }
        case KGPG::VerifyMode: {
            if (m_keys.isEmpty()) {
                setError("No key available");
                break;
            }

            // TODO: implement
            setError("Not implemented");
            break;
        }
        default: {
            Q_ASSERT(false);
            break;
        }
    }

    m_ui.progressbar->setMinimum(1);
    m_ui.progressbar->setMaximum(100);
    m_ui.progressbar->setVisible(false);
    m_ui.startbutton->setVisible(true);
    m_ui.statusbar->showMessage("Done");
}

void KGPG::slotKeysBox(const int index)
{
    const KGPGKey kgpgkey = m_keys.at(index);
    m_ui.namelabel->setText(kgpgkey.name);
    m_ui.emaillabel->setText(kgpgkey.email);
    m_ui.commentlabel->setText(kgpgkey.comment);
    m_ui.disabledled->setState(kgpgkey.disabled ? KLed::On : KLed::Off);
    m_ui.revokedled->setState(kgpgkey.revoked ? KLed::On : KLed::Off);
    m_ui.expiredled->setState(kgpgkey.expired ? KLed::On : KLed::Off);
    m_ui.canencryptled->setState(kgpgkey.canencrypt ? KLed::On : KLed::Off);
    m_ui.cansignled->setState(kgpgkey.cansign ? KLed::On : KLed::Off);
}

void KGPG::slotStart()
{
    start();
}

void KGPG::slotEncryptMode()
{
    setMode(KGPG::EncryptMode);
}

void KGPG::slotDecryptMode()
{
    setMode(KGPG::DecryptMode);
}

void KGPG::slotSignMode()
{
    setMode(KGPG::SignMode);
}

void KGPG::slotVerifyMode()
{
    setMode(KGPG::VerifyMode);
}

void KGPG::slotQuit()
{
    qApp->quit();
}

void KGPG::updateKeys(const gpgme_keylist_mode_t gpgmode, const bool gpgsecret)
{
    disconnect(m_ui.keysbox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotKeysBox(int)));

    m_keys.clear();
    m_ui.keysbox->clear();

    gpgme_error_t gpgresult = gpgme_set_keylist_mode(m_gpgctx, gpgmode);
    if (gpgresult != 0) {
        setError(gpgme_strerror(gpgresult));
        return;
    }

    // required by key query
    gpgresult = gpgme_op_keylist_start(m_gpgctx, NULL, gpgsecret);
    if (gpgresult != 0) {
        setError(gpgme_strerror(gpgresult));
        return;
    }

    gpgme_key_t gpgkey;
    gpgresult = gpgme_op_keylist_next(m_gpgctx, &gpgkey);
    while (gpgresult == 0) {
        gpgme_user_id_t gpguid = NULL;
        for (gpguid = gpgkey->uids; gpguid; gpguid = gpguid->next) {
            KGPGKey kgpgkey;
            kgpgkey.name = gpguid->name;
            kgpgkey.email = gpguid->email;
            kgpgkey.comment = gpguid->comment;
            kgpgkey.uidhash = gpguid->uidhash;
            kgpgkey.fpr = gpgkey->fpr;
            kgpgkey.disabled = gpgkey->disabled;
            kgpgkey.revoked = gpgkey->revoked;
            kgpgkey.expired = gpgkey->expired;
            kgpgkey.canencrypt = gpgkey->can_encrypt;
            kgpgkey.cansign = gpgkey->can_sign;
            m_keys.append(kgpgkey);
        }

        gpgme_key_unref(gpgkey);
        gpgresult = gpgme_op_keylist_next(m_gpgctx, &gpgkey);
    }

    connect(m_ui.keysbox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotKeysBox(int)));
    foreach (const KGPGKey &kgpgkey, m_keys) {
        m_ui.keysbox->addItem(kgpgkey.uidhash);
    }

    // qDebug() << Q_FUNC_INFO << m_keys.size();
}

int main(int argc, char **argv)
{
    KAboutData aboutData("kgpg", 0, ki18n("KGPG"),
        "1.0.0", ki18n("KDE encryption and decryption utility"), KAboutData::License_GPL,
        ki18n("(c) 2022 Ivailo Monev"));
    aboutData.addAuthor(ki18n("Ivailo Monev"), KLocalizedString(), "xakepa10@gmail.com");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineOptions options;
    options.add("encrypt", ki18n("Encrypt the specified URL"));
    options.add("decrypt", ki18n("Decrypt the specified URL"));
    options.add("sign", ki18n("Sign the specified URL"));
    options.add("verify", ki18n("Verify the specified URL"));
    options.add("+[url]", ki18n("URL to be opened"));
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    KGPG* kgpg = new KGPG();
    kgpg->setMode(KGPG::EncryptMode);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    for (int pos = 0; pos < args->count(); ++pos) {
        const KUrl argurl = args->url(pos);
        const KMimeType::Ptr argmime = KMimeType::findByUrl(argurl);
        if (argmime && argmime->is(QString::fromLatin1("application/pgp-encrypted"))) {
            kgpg->setMode(KGPG::DecryptMode);
        } else if (argmime && argmime->is(QString::fromLatin1("application/pgp-signature"))) {
            kgpg->setMode(KGPG::VerifyMode);
        }
        kgpg->setSource(argurl.prettyUrl());
    }

    if (args->isSet("encrypt")) {
        kgpg->setMode(KGPG::EncryptMode);
    } else if (args->isSet("decrypt")) {
        kgpg->setMode(KGPG::DecryptMode);
    } else if (args->isSet("sign")) {
        kgpg->setMode(KGPG::SignMode);
    } else if (args->isSet("verify")) {
        kgpg->setMode(KGPG::VerifyMode);
    }

    kgpg->show();

    return app.exec();
}

#include "moc_kgpg.cpp"
