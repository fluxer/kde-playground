/*
 * This file is part of the syndication library
 *
 * Copyright (C) 2005 Frank Osterfeld <osterfeld@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef SYNDICATION_RSS2_CATEGORY_H
#define SYNDICATION_RSS2_CATEGORY_H

#include <syndication/elementwrapper.h>

#include <QDomElement>
#include <QString>

namespace Syndication {
namespace RSS2 {

/**
 * A category which can be assigned to items or whole feeds.
 * These can be simple tags as known from delicious or Technorati, or
 * a category from a hierarchical taxonomy or ontology.
 *
 * @author Frank Osterfeld
 */
class SYNDICATION_EXPORT Category : public ElementWrapper
{
    public:

        /**
         * Creates a Category object wrapping a @c &lt;category> XML element.
         *
         * @param element The @c &lt;category> element to wrap
         */
        explicit Category(const QDomElement& element);

        /**
         * Default constructor, creates a null object, for which isNull() is
         * @c true.
         */
        Category();

        /**
         * Name of the category. This is both to be used as identifier and as
         * human-readable string. It can bea forward-slash-separated string
         * to identify a hierarchic location in the domain indicated by
         * domain(). Examples: "General", "Programming", "Funny",
         * "Books/History".
         *
         * @return The category identifier/name as string or a null string for
         * null objects.
         *
         */
        QString category() const;

        /**
         * optional, identifies the domain of the category, i.e. a
         * categorization taxonomy.
         *
         * @return The domain of the category, or a null string if none is set
         * (and for null objects)
         */
        QString domain() const;

        /**
         * Returns a description of the object for debugging purposes.
         *
         * @return debug string
         */
        QString debugInfo() const;
};

} // namespace RSS2
} // namespace Syndication

#endif // SYNDICATION_RSS2_CATEGORY_H
