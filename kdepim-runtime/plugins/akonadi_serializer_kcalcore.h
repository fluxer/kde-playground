/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#ifndef AKONADI_SERIALIZER_KCALCORE_H
#define AKONADI_SERIALIZER_KCALCORE_H

#include <QtCore/QObject>

#include <akonadi/differencesalgorithminterface.h>
#include <akonadi/itemserializerplugin.h>
#include <akonadi/gidextractorinterface.h>
#include <KCalCore/ICalFormat>

namespace Akonadi {

class SerializerPluginKCalCore : public QObject,
                             public ItemSerializerPlugin,
                             public DifferencesAlgorithmInterface,
                             public GidExtractorInterface

{
    Q_OBJECT
    Q_INTERFACES( Akonadi::ItemSerializerPlugin )
    Q_INTERFACES( Akonadi::DifferencesAlgorithmInterface )
    Q_INTERFACES( Akonadi::GidExtractorInterface )

  public:
    bool deserialize( Item& item, const QByteArray& label, QIODevice& data, int version );
    void serialize( const Item& item, const QByteArray& label, QIODevice& data, int &version );

    void compare( Akonadi::AbstractDifferencesReporter *reporter,
                  const Akonadi::Item &leftItem,
                  const Akonadi::Item &rightItem );

    QString extractGid( const Item &item ) const;

  private:
    KCalCore::ICalFormat mFormat;
};

}

#endif
