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


class KToolBar : QToolBar
{
%TypeHeaderCode
#include <ktoolbar.h>
%End


public:
    explicit                KToolBar (QWidget* parent /TransferThis/, bool isMainToolBar = 0, bool readConfig = 1);
                            KToolBar (const QString& objectName, QMainWindow* parentWindow, Qt::ToolBarArea area, bool newLine = 0, bool isMainToolBar = 0, bool readConfig = 1);
    KMainWindow*            mainWindow () const;
    void                    setIconDimensions (int size);
    int                     iconSizeDefault () const;
    void                    saveSettings (KConfigGroup& cg);
    void                    applySettings (const KConfigGroup& cg, bool forceGlobal = 0);
    void                    loadState (const QDomElement& element);
    void                    saveState (QDomElement& element) const;
    bool                    eventFilter (QObject* watched, QEvent* event);
    static Qt::ToolButtonStyle  toolButtonStyleSetting ();
    static bool             toolBarsEditable ();
    static void             setToolBarsEditable (bool editable);
    static bool             toolBarsLocked ();
    static void             setToolBarsLocked (bool locked);

protected:
    virtual void            slotMovableChanged (bool movable);
    virtual void            contextMenuEvent (QContextMenuEvent*);
    virtual void            actionEvent (QActionEvent*);
    virtual void            dragEnterEvent (QDragEnterEvent*);
    virtual void            dragMoveEvent (QDragMoveEvent*);
    virtual void            dragLeaveEvent (QDragLeaveEvent*);
    virtual void            dropEvent (QDropEvent*);
    virtual void            mousePressEvent (QMouseEvent*);
    virtual void            mouseMoveEvent (QMouseEvent*);
    virtual void            mouseReleaseEvent (QMouseEvent*);
public:
    explicit                KToolBar (const QString& objectName, QWidget* parent /TransferThis/, bool readConfig = 1);
    ~KToolBar ();
    void                    addXMLGUIClient (KXMLGUIClient* client);
};
// KToolBar


