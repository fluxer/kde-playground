/*
    Copyright (c) 2008 Tobias Koenig <tokoe@kde.org>

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

#ifndef AKONADI_AGENTTYPE_P_H
#define AKONADI_AGENTTYPE_P_H

#include <QtCore/QSharedData>
#include <QtCore/QStringList>
#include <QtCore/QVariantMap>

namespace Akonadi
{

/**
 * @internal
 */
class AgentType::Private : public QSharedData
{
public:
    Private()
    {
    }

    Private(const Private &other)
        : QSharedData(other)
    {
        mIdentifier = other.mIdentifier;
        mName = other.mName;
        mDescription = other.mDescription;
        mIconName = other.mIconName;
        mMimeTypes = other.mMimeTypes;
        mCapabilities = other.mCapabilities;
        mCustomProperties = other.mCustomProperties;
    }

    QString mIdentifier;
    QString mName;
    QString mDescription;
    QString mIconName;
    QStringList mMimeTypes;
    QStringList mCapabilities;
    QVariantMap mCustomProperties;
};

}

#endif
