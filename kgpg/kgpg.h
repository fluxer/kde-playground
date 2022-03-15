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

#ifndef KGPG_H
#define KGPG_H

#include <QResizeEvent>
#include <kmainwindow.h>

#include <gpgme.h>

#include "ui_kgpgwidget.h"

struct KGPGKey
{
    QByteArray name;
    QByteArray email;
    QByteArray comment;
    QByteArray uidhash;
    QByteArray fpr;
    bool disabled;
    bool revoked;
    bool expired;
    bool canencrypt;
    bool cansign;
};

class KGPG : public KMainWindow
{
    Q_OBJECT
public:
    enum KGPGMode {
        EncryptMode = 0,
        DecryptMode = 1,
        SignMode    = 2,
        VerifyMode  = 3
    };

    explicit KGPG(QWidget *parent = 0);
    ~KGPG();

    void setMode(const KGPGMode mode);
    void setSource(const QString &source);
    void setError(const char* const error);
    void setError(const QString &error);
    void setProgress(const int gpgcurrent, const int gpgtotal);

    void start();

    static gpgme_error_t gpgPasswordCallback(void *opaque, const char *uid_hint,
                                             const char *passphrase_info,
                                             int prev_was_bad, int fd);
    static void gpgProgressCallback(void *opaque, const char *what,
                                    int type, int current, int total);

private Q_SLOTS:
    void slotKeysBox(const int index);
    void slotStart();
    void slotEncryptMode();
    void slotDecryptMode();
    void slotSignMode();
    void slotVerifyMode();
    void slotQuit();

private:
    void updateKeys(const gpgme_keylist_mode_t gpgmode, const bool gpgsecret);

    Ui_KGPGWindow m_ui;
    KGPGMode m_mode;

    bool m_release;
    gpgme_ctx_t m_gpgctx;

    QList<KGPGKey> m_keys;
};

#endif // KGPG_H
