//
//     Copyright 2008 Jim Bublitz <jbublitz@nwinternet.com>
//     Earlier copyrights 1998 - 2007 Jim Bublitz also apply

//                 Generated by twine

// This file is part of PyKDE4.

// PyKDE4 is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of
// the License, or (at your option) any later version.

// PyKDE4 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


class KPasswordDialog : KDialog
{
%TypeHeaderCode
#include <kpassworddialog.h>
%End



public:
    enum KPasswordDialogFlag
    {
        NoFlags,
        ShowKeepPassword,
        ShowUsernameLine,
        UsernameReadOnly,
        ShowAnonymousLoginCheckBox,
        ShowDomainLine,
        DomainReadOnly
    };

    typedef QFlags<KPasswordDialog::KPasswordDialogFlag> KPasswordDialogFlags;


    enum ErrorType
    {
        UnknownError,
        UsernameError,
        PasswordError,
        FatalError,
        DomainError
    };

    explicit                KPasswordDialog (QWidget* parent /TransferThis/ = 0, const KPasswordDialog::KPasswordDialogFlags& flags = 0, const KDialog::ButtonCodes otherButtons = 0);
    void                    setPrompt (const QString& prompt);
    QString                 prompt () const;
    void                    setPixmap (const QPixmap&);
    QPixmap                 pixmap () const;
    void                    addCommentLine (const QString& label, const QString& comment);
    void                    showErrorMessage (const QString& message, const KPasswordDialog::ErrorType type = KPasswordDialog::PasswordError);
    QString                 password () const;
    void                    setUsername (const QString&);
    QString                 username () const;

    void                    setDomain (const QString&);
    QString                 domain () const;
    void                    setAnonymousMode (bool anonymous);
    bool                    anonymousMode () const;

    bool                    keepPassword () const;
    void                    setKeepPassword (bool b);
    void                    setUsernameReadOnly (bool readOnly);
    void                    setPassword (const QString& password);
    void                    setKnownLogins (const QMap<QString,QString>& knownLogins);
    void                    accept ();

signals:
    void                    gotPassword (const QString& password, bool keep);
    void                    gotUsernameAndPassword (const QString& username, const QString& password, bool keep);

protected:
    virtual bool            checkPassword ();

private:
//force
                            KPasswordDialog (const KPasswordDialog&);
//end
public:
    ~KPasswordDialog ();
};
// KPasswordDialog


