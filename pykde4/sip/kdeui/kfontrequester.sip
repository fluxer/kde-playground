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


class KFontRequester : QWidget
{
%TypeHeaderCode
#include <kfontrequester.h>
%End


public:
    explicit                KFontRequester (QWidget* parent /TransferThis/ = 0, bool onlyFixed = 0);
    QFont                   font () const;
    bool                    isFixedOnly () const;
    QString                 sampleText () const;
    QString                 title () const;
    QLabel*                 label () const;
    QPushButton*            button () const;
    virtual void            setFont (const QFont& font, bool onlyFixed = 0);
    virtual void            setSampleText (const QString& text);
    virtual void            setTitle (const QString& title);

signals:
    void                    fontSelected (const QFont& font);

private:
//force
                            KFontRequester (const KFontRequester&);
//end
public:
    ~KFontRequester ();
};
// KFontRequester


