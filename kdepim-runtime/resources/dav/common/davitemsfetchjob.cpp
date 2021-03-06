/*
    Copyright (c) 2010 Grégory Oestreicher <greg@kamago.net>
    Based on DavItemsListJob which is copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

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

#include "davitemsfetchjob.h"
#include "davmanager.h"
#include "davmultigetprotocol.h"

#include <kio/davjob.h>
#include <klocale.h>

DavItemsFetchJob::DavItemsFetchJob( const DavUtils::DavUrl &collectionUrl, const QStringList &urls, QObject *parent )
  : KJob( parent ), mCollectionUrl( collectionUrl ), mUrls( urls )
{
}

void DavItemsFetchJob::start()
{
  const DavMultigetProtocol *protocol =
      dynamic_cast<const DavMultigetProtocol*>( DavManager::self()->davProtocol( mCollectionUrl.protocol() ) );
  if ( !protocol ) {
    setError( UserDefinedError );
    setErrorText( i18n( "Protocol for the collection does not support MULTIGET" ) );
    emitResult();
    return;
  }

  const QDomDocument report = protocol->itemsReportQuery( mUrls );
  KIO::DavJob *job = DavManager::self()->createReportJob( mCollectionUrl.url(), report, QLatin1String( "0" ) );
  job->addMetaData( QLatin1String("PropagateHttpHeader"), QLatin1String("true") );
  connect( job, SIGNAL(result(KJob*)), this, SLOT(davJobFinished(KJob*)) );
}

DavItem::List DavItemsFetchJob::items() const
{
  return mItems.values();
}

DavItem DavItemsFetchJob::item( const QString &url ) const
{
  return mItems.value( url );
}

void DavItemsFetchJob::davJobFinished( KJob *job )
{
  KIO::DavJob *davJob = qobject_cast<KIO::DavJob*>( job );
  const int responseCode = davJob->queryMetaData( QLatin1String("responsecode") ).isEmpty() ?
                            0 :
                            davJob->queryMetaData( QLatin1String("responsecode") ).toInt();

  // KIO::DavJob does not set error() even if the HTTP status code is a 4xx or a 5xx
  if ( davJob->error() || ( responseCode >= 400 && responseCode < 600 ) ) {
    QString err;
    if ( davJob->error() && davJob->error() != KIO::ERR_SLAVE_DEFINED )
      err = KIO::buildErrorString( davJob->error(), davJob->errorText() );
    else
      err = davJob->errorText();

    setError( UserDefinedError + responseCode );
    setErrorText( i18n( "There was a problem with the request.\n"
                        "%1 (%2).", err, responseCode ) );

    emitResult();
    return;
  }

  const DavMultigetProtocol *protocol =
      static_cast<const DavMultigetProtocol*>( DavManager::self()->davProtocol( mCollectionUrl.protocol() ) );

  const QDomDocument document = davJob->response();
  const QDomElement documentElement = document.documentElement();

  QDomElement responseElement = DavUtils::firstChildElementNS( documentElement, QLatin1String("DAV:"), QLatin1String("response") );

  while ( !responseElement.isNull() ) {
    QDomElement propstatElement = DavUtils::firstChildElementNS( responseElement, QLatin1String("DAV:"), QLatin1String("propstat") );

    if ( propstatElement.isNull() ) {
      responseElement = DavUtils::nextSiblingElementNS( responseElement, QLatin1String("DAV:"), QLatin1String("response") );
      continue;
    }

    // Check for errors
    const QDomElement statusElement = DavUtils::firstChildElementNS( propstatElement, QLatin1String("DAV:"), QLatin1String("status") );
    if ( !statusElement.text().contains( QLatin1String("200") ) ) {
      responseElement = DavUtils::nextSiblingElementNS( responseElement, QLatin1String("DAV:"), QLatin1String("response") );
      continue;
    }

    const QDomElement propElement = DavUtils::firstChildElementNS( propstatElement, QLatin1String("DAV:"), QLatin1String("prop") );

    DavItem item;

    // extract path
    const QDomElement hrefElement = DavUtils::firstChildElementNS( responseElement, QLatin1String("DAV:"), QLatin1String("href") );
    const QString href = hrefElement.text();

    KUrl url = davJob->url();
    url.setUser( QString() );
    if ( href.startsWith( QLatin1Char('/') ) ) {
      // href is only a path, use request url to complete
      url.setEncodedPath( href.toLatin1() );
    } else {
      // href is a complete url
      KUrl tmpUrl( href );
      url = tmpUrl;
    }

    item.setUrl( url.prettyUrl() );

    // extract etag
    const QDomElement getetagElement = DavUtils::firstChildElementNS( propElement, QLatin1String("DAV:"), QLatin1String("getetag") );
    item.setEtag( getetagElement.text() );

    // extract content
    const QDomElement dataElement = DavUtils::firstChildElementNS( propElement,
                                                                   protocol->responseNamespace(),
                                                                   protocol->dataTagName() );

    if ( dataElement.isNull() ) {
      responseElement = DavUtils::nextSiblingElementNS( responseElement, QLatin1String("DAV:"), QLatin1String("response") );
      continue;
    }

    const QByteArray data = dataElement.firstChild().toText().data().toUtf8();
    if ( data.isEmpty() ) {
      responseElement = DavUtils::nextSiblingElementNS( responseElement, QLatin1String("DAV:"), QLatin1String("response") );
      continue;
    }

    item.setData( data );

    mItems.insert( item.url(), item );
    responseElement = DavUtils::nextSiblingElementNS( responseElement, QLatin1String("DAV:"), QLatin1String("response") );
  }

  emitResult();
}
