//
//     Copyright 2008 Jim Bublitz <jbublitz@nwinternet.com>
//     Earlier copyrights 1998 - 2007 Jim Bublitz also apply

//                 Generated by twine

// This file is part of PyKDE4.

// PyKDE4 is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of
// the License, or (at your option) any later version.

// PyKDE4 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


class KServiceAction
{
%TypeHeaderCode
#include <kserviceaction.h>
%End


public:
                            KServiceAction (const QString& name, const QString& text, const QString& icon, const QString& exec, bool noDisplay = 0);
                            KServiceAction ();
                            KServiceAction (const KServiceAction& other);
    void                    setData (const QVariant& userData);
    QVariant                data () const;
    QString                 name () const;
    QString                 text () const;
    QString                 icon () const;
    QString                 exec () const /PyName=exec_/;
    bool                    noDisplay () const;
    bool                    isSeparator () const;
    ~KServiceAction ();
};
// KServiceAction


QDataStream&            operator >> (QDataStream& str, KServiceAction& act);
QDataStream&            operator << (QDataStream& str, const KServiceAction& act);
