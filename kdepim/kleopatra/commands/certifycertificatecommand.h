/* -*- mode: c++; c-basic-offset:4 -*-
    commands/signcertificatecommand.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef __KLEOPATRA_COMMANDS_CERTIFYCERTIFICATECOMMAND_H__
#define __KLEOPATRA_COMMANDS_CERTIFYCERTIFICATECOMMAND_H__

#include <commands/command.h>

namespace GpgME {
    class UserID;
}

namespace Kleo {
namespace Commands {

    class CertifyCertificateCommand : public Command {
        Q_OBJECT
    public:
        explicit CertifyCertificateCommand( QAbstractItemView * view, KeyListController * parent );
        explicit CertifyCertificateCommand( KeyListController * parent );
        explicit CertifyCertificateCommand( const GpgME::Key & key );
        explicit CertifyCertificateCommand( const GpgME::UserID & uid );
        explicit CertifyCertificateCommand( const std::vector<GpgME::UserID> & uids );
        ~CertifyCertificateCommand();

        /* reimp */ static Restrictions restrictions() { return OnlyOneKey|MustBeOpenPGP; }

        void setCertificationExportable( bool on );
        void setCertificationRevocable( bool on );

        void setCertifyingKey( const GpgME::Key & key );

        void setUserID( const GpgME::UserID & uid );
        void setUserIDs( const std::vector<GpgME::UserID> & uids );

    private:
        /* reimp */ void doStart();
        /* reimp */ void doCancel();

    private:
        class Private;
        inline Private * d_func();
        inline const Private * d_func() const;
        Q_PRIVATE_SLOT( d_func(), void slotResult(GpgME::Error) )
        Q_PRIVATE_SLOT( d_func(), void slotDialogRejected() )
        Q_PRIVATE_SLOT( d_func(), void slotCertificationPrepared() )
    };

}
}

#endif // __KLEOPATRA_COMMANDS_SIGNCERTIFICATECOMMAND_H__
