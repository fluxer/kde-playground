/*
    Copyright (c) 2010 Grégory Oestreicher <greg@kamago.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef URLCONFIGURATIONDIALOG_H
#define URLCONFIGURATIONDIALOG_H

#include "ui_urlconfigurationdialog.h"

#include "davutils.h"

#include <KDialog>

#include <QtCore/QString>

class KJob;
#include <QModelIndex>
#include <QStandardItemModel>

class UrlConfigurationDialog : public KDialog
{
  Q_OBJECT

  public:
    explicit UrlConfigurationDialog( QWidget *parent = 0 );
    ~UrlConfigurationDialog();

    DavUtils::Protocol protocol() const;
    void setProtocol( DavUtils::Protocol protocol );

    QString remoteUrl() const;
    void setRemoteUrl( const QString &url );

    bool useDefaultCredentials() const;
    void setUseDefaultCredentials( bool defaultCreds );

    QString username() const;
    void setDefaultUsername( const QString &name );
    void setUsername( const QString &name );

    QString password() const;
    void setDefaultPassword( const QString &password );
    void setPassword( const QString &password );

  private Q_SLOTS:
    void onConfigChanged();
    void checkUserInput();
    void onFetchButtonClicked();
    void onOkButtonClicked();
    void onCollectionsFetchDone( KJob *job );
    void onModelDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight );
    void onChangeDisplayNameFinished( KJob *job );

  private:
    void initModel();
    bool checkUserAuthInput();
    void addModelRow( const QString &displayName, const QString &url );

    Ui::UrlConfigurationDialog mUi;
    QStandardItemModel *mModel;
    QString mDefaultUsername;
    QString mDefaultPassword;
};

#endif
