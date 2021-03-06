/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2008 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#include "result.h"

#include <QtCore/QSharedData>
#include <QtCore/QHash>

#include "soprano/node.h" // for qHash( QUrl() )

class Nepomuk::Query::Result::Private : public QSharedData
{
public:
    QUrl resource;
    double score;
    QHash<QUrl, Soprano::Node> requestProperties;
};

Nepomuk::Query::Result::Result()
    : d( new Private() )
{
}

Nepomuk::Query::Result::Result( const QUrl &uri, double score )
    : d( new Private() )
{
    d->resource = uri;
    d->score = score;
}

Nepomuk::Query::Result::Result( const Result &other )
{
    d = other.d;
}

Nepomuk::Query::Result::~Result()
{
}

Nepomuk::Query::Result &Nepomuk::Query::Result::operator=( const Result &other )
{
    d = other.d;
    return *this;
}

double Nepomuk::Query::Result::score() const
{
    return d->score;
}

QUrl Nepomuk::Query::Result::resourceUri() const
{
    return d->resource;
}

void Nepomuk::Query::Result::setScore( double score )
{
    d->score = score;
}

void Nepomuk::Query::Result::addRequestProperty( const QUrl &property, const Soprano::Node &value )
{
    d->requestProperties[property] = value;
}

Soprano::Node Nepomuk::Query::Result::operator[]( const QUrl &property ) const
{
    return requestProperty( property );
}

Soprano::Node Nepomuk::Query::Result::requestProperty( const QUrl &property ) const
{
    QHash<QUrl, Soprano::Node>::const_iterator it = d->requestProperties.find( property );
    if ( it != d->requestProperties.end() ) {
        return *it;
    } else {
        return Soprano::Node();
    }
}

QHash<QUrl, Soprano::Node> Nepomuk::Query::Result::requestProperties() const
{
    return d->requestProperties;
}

bool Nepomuk::Query::Result::operator==( const Result &other ) const
{
    if ( d->resource != other.d->resource ||
         d->score != other.d->score ) {
        return false;
    }
    for ( QHash<QUrl, Soprano::Node>::const_iterator it = d->requestProperties.constBegin();
          it != d->requestProperties.constEnd(); ++it ) {
        QHash<QUrl, Soprano::Node>::const_iterator it2 = other.d->requestProperties.constFind( it.key() );
        if ( it2 == other.d->requestProperties.constEnd() ||
             it2.value() != it.value() ) {
            return false;
        }
    }
    for ( QHash<QUrl, Soprano::Node>::const_iterator it = other.d->requestProperties.constBegin();
          it != other.d->requestProperties.constEnd(); ++it ) {
        QHash<QUrl, Soprano::Node>::const_iterator it2 = d->requestProperties.constFind( it.key() );
        if ( it2 == d->requestProperties.constEnd() ||
             it2.value() != it.value() ) {
            return false;
        }
    }
    return true;
}
