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

#include <QImageWriter>
#include <klocale.h>
#include <kapplication.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include "kgpg.h"

KGPG::KGPG(QWidget *parent)
    : KMainWindow(parent),
    m_release(false)
{
    m_ui.setupUi(this);

    // required by context
    kDebug() << gpgme_check_version(NULL);

#ifndef kgpg_key_query
    gpgme_error_t gpgresult = gpgme_new(&m_gpgctx);
    if (gpgresult != 0) {
        setError(gpgme_strerror(gpgresult));
        return;
    }
    m_release = true;

    gpgresult = gpgme_set_keylist_mode(m_gpgctx, GPGME_KEYLIST_MODE_LOCAL);
    if (gpgresult != 0) {
        setError(gpgme_strerror(gpgresult));
        return;
    }

    // required by key query
    gpgresult = gpgme_op_keylist_start(m_gpgctx, NULL, false);
    if (gpgresult != 0) {
        setError(gpgme_strerror(gpgresult));
        return;
    }

    char* gpgkeyfpr = NULL; // will either use key or password from input for encryption
    gpgme_key_t gpgkey;
    gpgresult = gpgme_op_keylist_next(m_gpgctx, &gpgkey);
    while (gpgresult == 0) {
        gpgme_user_id_t gpguid = NULL;
        for (gpguid = gpgkey->uids; gpguid; gpguid = gpguid->next) {
            kWarning() << gpguid->email << gpguid->name << gpguid->comment << gpguid->uidhash << gpgkey->fpr;
            ::free(gpgkeyfpr);
            gpgkeyfpr = ::strdup(gpgkey->fpr);
        }

        gpgme_key_unref(gpgkey);
        gpgresult = gpgme_op_keylist_next(m_gpgctx, &gpgkey);
    }
#endif // kgpg_key_query

#ifndef kgpg_encrypt
    gpgme_data_t gpgindata;
    gpgresult = gpgme_data_new_from_file(&gpgindata, "/home/smil3y/gpgme/tests/run-encrypt.c", 1);
    if (gpgresult != 0) {
        setError(gpgme_strerror(gpgresult));
        ::free(gpgkeyfpr);
        return;
    }

    gpgme_key_t gpgencryptkey;
    gpgresult = gpgme_get_key(m_gpgctx, gpgkeyfpr, &gpgencryptkey, 1);
    ::free(gpgkeyfpr);
    if (gpgresult != 0) {
        kWarning() << "gpgme_get_key" << gpgme_strerror(gpgresult);
        gpgme_data_release(gpgindata);
        return;
    }

    gpgme_data_t gpgoutdata;
    gpgresult = gpgme_data_new(&gpgoutdata);
    if (gpgresult != 0) {
        setError(gpgme_strerror(gpgresult));
        gpgme_data_release(gpgindata);
        return;
    }

    gpgme_key_t gpgkeys[2] = { gpgencryptkey, NULL };
    gpgresult = gpgme_op_encrypt(m_gpgctx, gpgkeys, GPGME_ENCRYPT_ALWAYS_TRUST, gpgindata, gpgoutdata);
    if (gpgresult != 0) {
        setError(gpgme_strerror(gpgresult));
        gpgme_data_release(gpgindata);
        gpgme_data_release(gpgoutdata);
        return;
    }
#endif // kgpg_encrypt

#ifndef kgpg_decrypt
    gpgme_data_t gpgdecryptoutdata;
    gpgme_data_new(&gpgdecryptoutdata);
    if (gpgresult != 0) {
        setError(gpgme_strerror(gpgresult));
        gpgme_data_release(gpgindata);
        gpgme_data_release(gpgoutdata);
        return;
    }

    gpgme_data_seek(gpgoutdata, 0, SEEK_SET);
    gpgresult = gpgme_op_decrypt(m_gpgctx, gpgoutdata, gpgdecryptoutdata);
    if (gpgresult != 0) {
        setError(gpgme_strerror(gpgresult));
        gpgme_data_release(gpgindata);
        gpgme_data_release(gpgoutdata);
        gpgme_data_release(gpgdecryptoutdata);
        return;
    }

    size_t gpgbuffersize = 0;
    char* gpgbuffer = gpgme_data_release_and_get_mem(gpgdecryptoutdata, &gpgbuffersize);
    kWarning() << "decryption" << gpgbuffer;
#endif // kgpg_decrypt

    gpgme_free(gpgbuffer);
    gpgme_data_release(gpgindata);
    gpgme_data_release(gpgoutdata);
    
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
    switch (mode) {
        // TODO: implement
        default: {
            break;
        }
    }
}

void KGPG::setSource(const QString &source)
{
    const KUrl sourceurl(source);
    m_ui.sourcerequester->setUrl(sourceurl);
    m_ui.destinationrequester->setUrl(sourceurl.directory());
}

void KGPG::setError(const char* const error)
{
    const QString errorstring = QString::fromLocal8Bit(error);
    kWarning() << errorstring;

    const QString errormessage = i18n("Error: %1", errorstring);
    m_ui.statusbar->showMessage(errormessage);
    m_ui.progressbar->setMinimum(0);
    m_ui.progressbar->setMaximum(1);
}

void KGPG::start()
{
    switch (m_mode) {
        // TODO: implement
        default: {
            break;
        }
    }
}

int main(int argc, char **argv)
{
    KAboutData aboutData("kgpg", 0, ki18n("KGPG"),
        "1.0.0", ki18n("KDE encryption and decryption utility"), KAboutData::License_GPL,
        ki18n("(c) 2022 Ivailo Monev"));
    aboutData.addAuthor(ki18n("Ivailo Monev"), KLocalizedString(), "xakepa10@gmail.com");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineOptions options;
    options.add("encrypt <url>", ki18n("Encrypt the specified URL"));
    options.add("decrypt <url>", ki18n("Decrypt the specified URL"));
    options.add("sign <url>", ki18n("Sign the specified URL"));
    options.add("verify <url>", ki18n("Verify the specified URL"));
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    KGPG* kgpg = new KGPG();
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->isSet("encrypt")) {
        kgpg->setSource(args->getOption("encrypt"));
        kgpg->setMode(KGPG::EncryptMode);
    } else if (args->isSet("decrypt")) {
        kgpg->setSource(args->getOption("decrypt"));
        kgpg->setMode(KGPG::DecryptMode);
        kgpg->start();
    } else if (args->isSet("sign")) {
        kgpg->setSource(args->getOption("sign"));
        kgpg->setMode(KGPG::SignMode);
    } else if (args->isSet("verify")) {
        kgpg->setSource(args->getOption("verify"));
        kgpg->setMode(KGPG::VerifyMode);
        kgpg->start();
    } else {
        kgpg->setMode(KGPG::EncryptMode);
    }
    kgpg->show();

    return app.exec();
}

#include "moc_kgpg.cpp"
