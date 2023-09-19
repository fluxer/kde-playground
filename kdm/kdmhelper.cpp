/*  This file is part of the KDE project
    Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

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

#include "kdmhelper.h"

#include <kdebug.h>

#include <sys/types.h>
#include <pwd.h>
#include <shadow.h>
#include <unistd.h>

KDMHelper::KDMHelper(const char* const helper, QObject *parent)
    : KAuthorization(helper, parent)
{
}

int KDMHelper::check(const QVariantMap &parameters)
{
    const QByteArray userbytes = parameters.value("user").toByteArray();
    const QByteArray passbytes = parameters.value("password").toByteArray();
    if (userbytes.isEmpty() || passbytes.isEmpty()) {
        return KAuthorization::HelperError;
    }

    struct passwd* pw = ::getpwnam(userbytes.constData());
    if (!pw) {
        kWarning() << "null passwd struct";
        return KAuthorization::HelperError;
    }

    QByteArray pw_passwd = pw->pw_passwd;
    if (pw_passwd == "x" || pw_passwd == "*") {
        if (::lckpwdf() != 0) {
            kWarning() << "lckpwdf() failed";
            return KAuthorization::HelperError;
        }
        ::setspent();
        struct spwd *spw = ::getspnam(userbytes.constData());
        if (!spw) {
            kWarning() << "null shadow passwd struct";
            ::endspent();
            ::ulckpwdf();
            return KAuthorization::HelperError;
        }
        pw_passwd = spw->sp_pwdp;
        ::endspent();
        ::ulckpwdf();
    }
    if (passbytes == pw_passwd || ::crypt(passbytes.constData(), pw_passwd) == pw_passwd) {
        return KAuthorization::NoError;
    }
    kWarning() << "password does not match";
    return 1;
}

int KDMHelper::login(const QVariantMap &parameters)
{
    if (!parameters.contains("user")) {
        return KAuthorization::HelperError;
    }

    // TODO:
    return KAuthorization::NoError;
}

K_AUTH_MAIN("org.kde.kdm.helper", KDMHelper)
