//
//     Copyright 2008 Simon Edwards <simon@simonzone.com>

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


class KCategoryDrawer
{
%TypeHeaderCode
#include <kcategorydrawer.h>
%End

public:
    KCategoryDrawer (QWidget* parent /TransferThis/ = 0);

    virtual void            drawCategory (const QModelIndex& index, int sortRole, const QStyleOption& option, QPainter* painter) const;
    virtual int             categoryHeight (const QModelIndex& index, const QStyleOption& option) const;
    ~KCategoryDrawer ();
    int                     leftMargin () const;
    void                    setLeftMargin (int leftMargin);
    int                     rightMargin () const;
    void                    setRightMargin (int rightMargin);

    void            mouseButtonPressed (const QModelIndex& index, QMouseEvent* event);
    void            mouseButtonReleased (const QModelIndex& index, QMouseEvent* event);
    void            mouseButtonMoved (const QModelIndex& index, QMouseEvent* event);
    void            mouseButtonDoubleClicked (const QModelIndex& index, QMouseEvent* event);

    KCategorizedView*       view () const;

signals:
    void                    collapseOrExpandClicked (const QModelIndex& index);
    void                    actionRequested (int action, const QModelIndex& index);

protected:
    void            mouseButtonPressed (const QModelIndex& index, const QRect& blockRect, QMouseEvent* event);
    void            mouseButtonReleased (const QModelIndex& index, const QRect& blockRect, QMouseEvent* event);
    void            mouseMoved (const QModelIndex& index, const QRect& blockRect, QMouseEvent* event);
    void            mouseButtonDoubleClicked (const QModelIndex& index, const QRect& blockRect, QMouseEvent* event);
    void            mouseLeft (const QModelIndex& index, const QRect& blockRect);
};
