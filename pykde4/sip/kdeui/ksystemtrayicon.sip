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



class KSystemTrayIcon : QSystemTrayIcon
{
%TypeHeaderCode
#include <ksystemtrayicon.h>
%End


public:
    explicit                KSystemTrayIcon (QWidget* parent /TransferThis/ = 0);
    explicit                KSystemTrayIcon (const QString& icon, QWidget* parent /TransferThis/ = 0);
    explicit                KSystemTrayIcon (const QIcon& icon, QWidget* parent /TransferThis/ = 0);
    explicit                KSystemTrayIcon (QMovie* movie, QWidget* parent /TransferThis/);
    void                    setMovie (QMovie* movie);
    const QMovie*           movie () const;
    KActionCollection*      actionCollection ();
    QWidget*                parentWidget () const;
    bool                    parentWidgetTrayClose () const;
    static QIcon            loadIcon (const QString& icon, const KComponentData& componentData = KGlobal::mainComponent());
    void                    setContextMenuTitle (QAction* action);
    QAction*                contextMenuTitle () const;

signals:
    void                    quitSelected ();

public:
    void                    toggleActive ();
    ~KSystemTrayIcon ();
};
// KSystemTrayIcon


