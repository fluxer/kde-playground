/*  This file is part of the KDE project
    Copyright (C) 2007-2008 Gökçen Eraslan <gokcen@pardus.org.tr>
    Copyright (C) 2008 Daniel Nicoletti <dantti85-pk@yahoo.com.br>
    Copyright (C) 2010 Dario Freddi <drf@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef AUTHDIALOG_H
#define AUTHDIALOG_H

#include "ui_AuthDialog.h"
#include "ui_authdetails.h"

#include <PolkitQt1/Identity>
#include <PolkitQt1/ActionDescription>

namespace PolkitQt1
{
class Details;
}

class AuthDialog : public KDialog, private Ui::AuthDialog
{
    Q_OBJECT
public:
    AuthDialog(const QString &actionId,
               const QString &message,
               const QString &iconName,
               const PolkitQt1::Details &details,
               const PolkitQt1::Identity::List &identities,
               WId parent);
    ~AuthDialog();

    void setRequest(const QString &request, bool requiresAdmin);
    void setOptions();
    QString password() const;
    void authenticationFailure();

    PolkitQt1::Identity adminUserSelected() const;

    PolkitQt1::ActionDescription m_actionDescription;

signals:
    void adminUserSelected(PolkitQt1::Identity);

public slots:
    virtual void accept();

private slots:
    void on_userCB_currentIndexChanged(int index);
    void notificationActivated(unsigned int action);

private:
    QString m_appname;
    QString m_message;

    void createUserCB(const PolkitQt1::Identity::List &identities);
    void showEvent(QShowEvent *);
};

class AuthDetails : public QWidget, private Ui::AuthDetails
{
    Q_OBJECT
public:
    AuthDetails(const PolkitQt1::Details &details,
                const PolkitQt1::ActionDescription &actionDescription,
                const QString &appname,
                QWidget *parent);

private slots:
    void openUrl(const QString&);
    void openAction(const QString&);
};

#endif // AUTHDIALOG_H
