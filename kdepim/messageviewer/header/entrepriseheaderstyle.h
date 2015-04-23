/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef ENTREPRISEHEADERSTYLE_H
#define ENTREPRISEHEADERSTYLE_H

#include "header/headerstyle.h"

namespace MessageViewer {

class EnterpriseHeaderStyle : public HeaderStyle {
    friend class HeaderStyle;
protected:
    EnterpriseHeaderStyle() : HeaderStyle() {}
    ~EnterpriseHeaderStyle() {}

public:
    const char * name() const { return "enterprise"; }

    QString format( KMime::Message *message ) const;

    bool hasAttachmentQuickList() const {
        return true;
    }
};

}

#endif // ENTREPRISEHEADERSTYLE_H