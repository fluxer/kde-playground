// Copyright 2009 Simon Edwards <simon@simonzone.com>

//                 Generated by twine2

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as
// published by the Free Software Foundation; either version 2, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details

// You should have received a copy of the GNU Library General Public
// License along with this program; if not, write to the
// Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

namespace KAuth
{
class ActionReply
{
%TypeHeaderCode
#include <kauthactionreply.h>
%End

public:
    enum Type
    {
        KAuthError,
        HelperError,
        Success
    };
    static const KAuth::ActionReply SuccessReply;
    static const KAuth::ActionReply HelperErrorReply;
    static const KAuth::ActionReply NoResponderReply;
    static const KAuth::ActionReply NoSuchActionReply;
    static const KAuth::ActionReply InvalidActionReply;
    static const KAuth::ActionReply AuthorizationDeniedReply;
    static const KAuth::ActionReply UserCancelledReply;
    static const KAuth::ActionReply HelperBusyReply;
    static const KAuth::ActionReply DBusErrorReply;
    enum Error
    {
        NoError,
        NoResponder,
        NoSuchAction,
        InvalidAction,
        AuthorizationDenied,
        UserCancelled,
        HelperBusy,
        DBusError
    };
                            ActionReply ();
                            ActionReply (KAuth::ActionReply::Type type);
//ig                            ActionReply (int errorCode);
                            ActionReply (const KAuth::ActionReply& reply);
    virtual ~ActionReply ();
    void                    setData (const QVariantMap& data);
    QVariantMap             data () const;
    void                    addData (const QString& key, const QVariant& value);
    KAuth::ActionReply::Type  type () const;
// FIXME not implemented
//ig    void                    setType (KAuth::ActionReply::Type type);
    bool                    succeeded () const;
    bool                    failed () const;
    int                     errorCode () const;
    void                    setErrorCode (int errorCode);
    QString                 errorDescription () const;
    void                    setErrorDescription (const QString& error);
    QByteArray              serialized () const;
    static KAuth::ActionReply  deserialize (const QByteArray& data);
//ig    QDataStream&            operator << (QDataStream&, const KAuth::ActionReply&);
//ig    QDataStream&            operator >> (QDataStream&, KAuth::ActionReply&);
    bool                    operator == (const KAuth::ActionReply& reply) const;
    bool                    operator != (const KAuth::ActionReply& reply) const;
};
};
