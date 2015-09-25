/***************************************************************************
 *   Copyright (C) by Simon Persson                                        *
 *   simonop@spray.se                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef BBFILTERDOCK_H
#define BBFILTERDOCK_H

#include <QDockWidget>
#include <QStringList>
#include <QItemSelectionModel>
#include <KLineEdit>

#include "bblistitem.h"

class QListView;
class BBStringSetModel;
class KPushButton;
class BBCollectionTab;
class QMenu;

class BBFilterDock: public QDockWidget
{
	Q_OBJECT
	public:
		BBFilterDock(const QString &pCategory, const QString &pTitle, BBCollectionTab *pCollectionTab);
		const QString & category() const {return mCategory;}
		void addCondition(bool pUseSelection, QStringList &pList);
		QStringList currentSelection() { return mSelection; }
		QString filterText() { return mLineEdit->text(); }

	public slots:
		void clearSelection() { mSelectionModel->clearSelection(); }
		void clearFilterText() { mLineEdit->clear(); }
		void setFilterText(const QString &pString) { mLineEdit->setText(pString); }
		void processResult(QSet<BBStringListItem> pResult);
		void updateSelection(const QItemSelection &pSelected, const QItemSelection &pDeselected);
		void lineEditChanged(const QString &pNewText);
		void forceSelection(const QStringList &pList);
		void syncSelection();

	protected slots:
		void showPopupMenu(const QPoint &pPoint);
		void editFilesTriggered();

	protected:
		QString mCategory;
		QString mQuotedSelection;
		QString mFilterTextCondition;
		QStringList mSelection;
		QListView *mListView;
		KLineEdit *mLineEdit;
		KPushButton *mPushButton;
		BBStringSetModel *mModel;
		QItemSelectionModel *mSelectionModel;
		BBCollectionTab *mCollectionTab;
		QMenu *mContextMenu;
};
#endif
