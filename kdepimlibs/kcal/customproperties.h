/*
  This file is part of the kcal library.

  Copyright (c) 2002,2006,2010 David Jarvie <djarvie@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
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
/**
  @file
  This file is part of the API for handling calendar data and
  defines the CustomProperties class.

  @author David Jarvie \<djarvie@kde.org\>
*/

#ifndef KCAL_CUSTOMPROPERTIES_H
#define KCAL_CUSTOMPROPERTIES_H

#include "kcal_export.h"

#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QByteArray>

namespace KCal {

/**
  @brief
  A class to manage custom calendar properties.

  This class represents custom calendar properties.
  It is used as a base class for classes which represent calendar components.
  A custom property name written by the kcal library has the form X-KDE-APP-KEY
  where APP represents the application name, and KEY distinguishes individual
  properties for the application.
  In keeping with RFC2445, property names must be composed only of the
  characters A-Z, a-z, 0-9 and '-'.
*/
class KCAL_DEPRECATED_EXPORT CustomProperties
{
  public:
    /**
      Constructs an empty custom properties instance.
    */
    CustomProperties();

    /**
      Copy constructor.
      @param other is the one to copy.
    */
    CustomProperties( const CustomProperties &other );

    /**
      Destructor.
    */
    virtual ~CustomProperties();

    /**
      Compare this with @p properties for equality.
      @param properties is the one to compare.

      @warning The comparison is not polymorphic.
    */
    // KDE5: make protected to prevent accidental usage
    bool operator==( const CustomProperties &properties ) const;

    /**
      Create or modify a custom calendar property.

      @param app   Application name as it appears in the custom property name.
      @param key   Property identifier specific to the application.
      @param value The property's value. A call with a value of QString()
      will be ignored.
      @see removeCustomProperty().
    */
    void setCustomProperty( const QByteArray &app, const QByteArray &key,
                            const QString &value );

    /**
      Delete a custom calendar property.

      @param app Application name as it appears in the custom property name.
      @param key Property identifier specific to the application.
      @see setCustomProperty().
    */
    void removeCustomProperty( const QByteArray &app, const QByteArray &key );

    /**
      Return the value of a custom calendar property.

      @param app Application name as it appears in the custom property name.
      @param key Property identifier specific to the application.
      @return Property value, or QString() if (and only if) the property
      does not exist.
    */
    QString customProperty( const QByteArray &app, const QByteArray &key ) const;

    /**
      Validate and return the full name of a custom calendar property.

      @param app Application name as it appears in the custom property name.
      @param key Property identifier specific to the application.
      @return Full property name, or empty string if it would contain invalid
              characters

      @since 4.5
    */
    static QByteArray customPropertyName( const QByteArray &app, const QByteArray &key );

    /**
      Create or modify a non-KDE or non-standard custom calendar property.

      @param name Full property name
      @param value The property's value. A call with a value of QString()
      will be ignored.
      @see removeNonKDECustomProperty().
    */
    void setNonKDECustomProperty( const QByteArray &name, const QString &value );

    /**
      Delete a non-KDE or non-standard custom calendar property.

      @param name Full property name
      @see setNonKDECustomProperty().
    */
    void removeNonKDECustomProperty( const QByteArray &name );

    /**
      Return the value of a non-KDE or non-standard custom calendar property.

      @param name Full property name
      @return Property value, or QString() if (and only if) the property
      does not exist.
    */
    QString nonKDECustomProperty( const QByteArray &name ) const;

    /**
      Initialise the alarm's custom calendar properties to the specified
      key/value pairs.
      @param properties is a QMap of property key/value pairs.
      @see customProperties().
    */
    void setCustomProperties( const QMap<QByteArray, QString> &properties );

    /**
      Returns all custom calendar property key/value pairs.
      @see setCustomProperties().
    */
    QMap<QByteArray, QString> customProperties() const;

    /**
      Assignment operator.

      @warning The assignment is not polymorphic.
    */
    // KDE5: make protected to prevent accidental usage
    CustomProperties &operator=( const CustomProperties &other );

  protected:
    /**
      Called when a custom property has been changed.
      The default implementation does nothing: override in derived classes
      to perform change processing.
    */
    virtual void customPropertyUpdated()  {}

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
