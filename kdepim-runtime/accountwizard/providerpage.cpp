/*
    Copyright (c) 2009 Volker Krause <vkrause@kde.org>
    Copyright (c) 2010 Tom Albers <toma@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "providerpage.h"
#include "global.h"

#include <KLocale>
#include <KDebug>
#include <QSortFilterProxyModel>

ProviderPage::ProviderPage(KAssistantDialog* parent) :
  Page( parent ),
  m_model( new QStandardItemModel( this ) ),
  m_newPageWanted( false ),
  m_newPageReady( false )
{
  ui.setupUi( this );

  QSortFilterProxyModel *proxy = new QSortFilterProxyModel( this );
  proxy->setSourceModel( m_model );
  ui.listView->setModel( proxy );
  ui.searchLine->setProxy( proxy );

  m_fetchItem = new QStandardItem( i18n( "Fetching provider list..." ) );
  m_fetchItem->setFlags(Qt::NoItemFlags);
  m_model->appendRow( m_fetchItem );

  connect( ui.listView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(selectionChanged()) );

  kDebug();
}

void ProviderPage::selectionChanged()
{
  if ( ui.listView->selectionModel()->hasSelection() ) {
    setValid( true );
  } else {
    setValid( false );
  }
}

void ProviderPage::leavePageNext()
{
  m_newPageReady = false;
  if ( !ui.listView->selectionModel()->hasSelection() )
    return;
  const QModelIndex index = ui.listView->selectionModel()->selectedIndexes().first();
  if ( !index.isValid() )
    return;

  const QSortFilterProxyModel *proxy = static_cast<const QSortFilterProxyModel*>( ui.listView->model() );
  const QStandardItem* item =  m_model->itemFromIndex( proxy->mapToSource( index ) );
  kDebug() << "Item selected:"<< item->text();

}

void ProviderPage::findDesktopAndSetAssistant( const QStringList& list )
{
  foreach ( const QString& file, list ) {
    kDebug() << file;
    if ( file.endsWith( QLatin1String ( ".desktop" ) ) ) {
      kDebug() << "Yay, a desktop file!" << file;
      Global::setAssistant( file );
      m_newPageReady = true;
      if ( m_newPageWanted ) {
        kDebug() << "New page was already requested, now we are done, approve it";
        emit leavePageNextOk();
      }
      break;
    }
  }
}

QTreeView *ProviderPage::treeview() const
{
  return ui.listView;
}

void ProviderPage::leavePageBackRequested()
{
  emit leavePageBackOk();
}

void ProviderPage::leavePageNextRequested()
{
  m_newPageWanted = true;
  if ( m_newPageReady ) {
    kDebug() << "New page requested and we are done, so ok...";
    emit leavePageNextOk();
  } else {
    kDebug() << "New page requested, but we are not done yet...";
  }
}

