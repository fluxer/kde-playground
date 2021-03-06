/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/signencryptfilestask.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

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

#ifndef __KLEOPATRA_CRYPTO_SIGNENCRYPTFILESTASK_H__
#define __KLEOPATRA_CRYPTO_SIGNENCRYPTFILESTASK_H__

#include <crypto/task.h>

#include <utils/pimpl_ptr.h>

#include <gpgme++/global.h>

#include <vector>

#include <QString>

namespace GpgME {
    class Key;
}

namespace Kleo {
    class OverwritePolicy;
    class Input;
}

namespace boost {
    template <typename T> class shared_ptr;
}

namespace Kleo {
namespace Crypto {

    class SignEncryptFilesTask : public Task {
        Q_OBJECT
    public:
        explicit SignEncryptFilesTask( QObject * parent=0 );
        ~SignEncryptFilesTask();

        void setInputFileName( const QString & fileName );
        void setInputFileNames( const QStringList & fileNames );
        void setInput( const boost::shared_ptr<Input> & input );
        void setOutputFileName( const QString & fileName );
        void setSigners( const std::vector<GpgME::Key> & singners );
        void setRecipients( const std::vector<GpgME::Key> & recipients );

        void setSign( bool sign );
        void setEncrypt( bool encrypt );
        void setRemoveInputFileOnSuccess( bool );
        void setDetachedSignature( bool detached );

        void setOverwritePolicy( const boost::shared_ptr<OverwritePolicy> & policy );
        GpgME::Protocol protocol() const;

        /* reimp */ void cancel();
        /* reimp */ QString label() const;
        /* reimp */ QString tag() const;

    private:
        /* reimp */ void doStart();
        /* reimp */ unsigned long long inputSize() const;

    private:
        class Private;
        kdtools::pimpl_ptr<Private> d;
        Q_PRIVATE_SLOT( d, void slotResult( const GpgME::SigningResult & ) )
        Q_PRIVATE_SLOT( d, void slotResult( const GpgME::SigningResult &, const GpgME::EncryptionResult & ) )
        Q_PRIVATE_SLOT( d, void slotResult( const GpgME::EncryptionResult & ) )
    };

}
}

#endif /* __KLEOPATRA_CRYPTO_SIGNENCRYPTFILESTASK_H__ */

