//
//     Copyright 2009 Simon Edwards <simon@simonzone.com>

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

namespace Plasma
{

class TextBrowser : QGraphicsProxyWidget
{
%TypeHeaderCode
#include <plasma/widgets/textbrowser.h>
%End

public:
    explicit                TextBrowser (QGraphicsWidget* parent /TransferThis/ = 0);
    void                    setText (const QString& text);
    QString                 text () const;
    void                    setHorizontalScrollBarPolicy (Qt::ScrollBarPolicy policy);
    void                    setVerticalScrollBarPolicy (Qt::ScrollBarPolicy policy);
    void                    setStyleSheet (const QString& stylesheet);
    QString                 styleSheet ();
    KTextBrowser*           nativeWidget () const;
    void                    dataUpdated (const QString& sourceName, const Plasma::DataEngine::Data& data);

signals:
    void                    textChanged ();

protected:
    void                    resizeEvent (QGraphicsSceneResizeEvent* event);
public:
    ~TextBrowser ();
protected:
    void                    wheelEvent (QGraphicsSceneWheelEvent* event);
    void                    changeEvent (QEvent* event);
    void                    contextMenuEvent (QGraphicsSceneContextMenuEvent* event);
public:
    void                    append (const QString& text);
};
// TextBrowser

};
// Plasma


