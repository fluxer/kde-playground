/*
  This file is part of KOrganizer.

  Copyright (c) 2008 Thomas Thrainer <tom_t@gmx.at>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef CALENDARVIEWS_TODODELEGATES_H
#define CALENDARVIEWS_TODODELEGATES_H

#include <Akonadi/Calendar/ETMCalendar>
#include <QStyledItemDelegate>

#include <QPainter>
#include <QSize>
#include <QStyleOptionViewItem>
#include <QTextDocument>

/**
  This delegate is responsible for displaying progress bars for the completion
  status of indivitual todos. It also provides a slider to change the completion
  status of the todo when in editing mode.

  @author Thomas Thrainer
*/
class TodoCompleteDelegate : public QStyledItemDelegate
{
  Q_OBJECT
  public:
    explicit TodoCompleteDelegate( QObject *parent = 0 );

    ~TodoCompleteDelegate();

    void paint( QPainter *painter, const QStyleOptionViewItem &option,
                const QModelIndex &index ) const;
    QSize sizeHint( const QStyleOptionViewItem &option,
                    const QModelIndex &index ) const;

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option,
                           const QModelIndex &index ) const;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const;
    void setModelData( QWidget *editor, QAbstractItemModel *model,
                       const QModelIndex &index ) const;
    void updateEditorGeometry( QWidget *editor,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index ) const;

  private:
    void initStyleOptionProgressBar( QStyleOptionProgressBar *option,
                                     const QModelIndex &index ) const;
};

class TodoCompleteSlider : public QSlider
{
  Q_OBJECT

  public:
    explicit TodoCompleteSlider( QWidget *parent );

  private Q_SLOTS:
    void updateTip( int value );
};

/**
  This delegate is responsible for displaying the priority of todos.
  It also provides a combo box to change the priority of the todo
  when in editing mode.

  @author Thomas Thrainer
 */
class TodoPriorityDelegate : public QStyledItemDelegate
{
  Q_OBJECT
  public:
    explicit TodoPriorityDelegate( QObject *parent = 0 );

    ~TodoPriorityDelegate();

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option,
                           const QModelIndex &index ) const;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const;
    void setModelData( QWidget *editor, QAbstractItemModel *model,
                       const QModelIndex &index ) const;
    void updateEditorGeometry( QWidget *editor,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index ) const;
};

/**
  This delegate is responsible for displaying the due date of todos.
  It also provides a combo box to change the due date of the todo
  when in editing mode.

  @author Thomas Thrainer
 */
class TodoDueDateDelegate : public QStyledItemDelegate
{
  Q_OBJECT
  public:
    explicit TodoDueDateDelegate( QObject *parent = 0 );

    ~TodoDueDateDelegate();

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option,
                           const QModelIndex &index ) const;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const;
    void setModelData( QWidget *editor, QAbstractItemModel *model,
                       const QModelIndex &index ) const;
    void updateEditorGeometry( QWidget *editor,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index ) const;
};

/**
  This delegate is responsible for displaying the categories of todos.
  It also provides a combo box to change the categories of the todo
  when in editing mode.

  @author Thomas Thrainer
 */
class TodoCategoriesDelegate : public QStyledItemDelegate
{
  Q_OBJECT
  public:
    explicit TodoCategoriesDelegate( QObject *parent = 0 );

    ~TodoCategoriesDelegate();

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option,
                           const QModelIndex &index ) const;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const;
    void setModelData( QWidget *editor, QAbstractItemModel *model,
                       const QModelIndex &index ) const;
    void updateEditorGeometry( QWidget *editor,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index ) const;

    void setCalendar( const Akonadi::ETMCalendar::Ptr &cal );
  private:
    Akonadi::ETMCalendar::Ptr mCalendar;
};

/**
  This delegate is responsible for displaying possible rich text elements
  of a todo. That's the summary and the description.

  @author Thomas Thrainer
 */
class TodoRichTextDelegate : public QStyledItemDelegate
{
  Q_OBJECT
  public:
    explicit TodoRichTextDelegate( QObject *parent = 0 );

    ~TodoRichTextDelegate();

    void paint( QPainter *painter, const QStyleOptionViewItem &option,
                const QModelIndex &index ) const;
    QSize sizeHint( const QStyleOptionViewItem &option,
                    const QModelIndex &index ) const;
  private:
    QTextDocument *m_textDoc;
};

#endif
