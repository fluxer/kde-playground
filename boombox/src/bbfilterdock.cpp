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
#include "bbfilterdock.h"
#include "bbdatabase.h"
#include "bbcollectiontab.h"
#include "bbsetmodel.h"

#include <QApplication>
#include <KLineEdit>
#include <KPushButton>
#include <QListView>
#include <QLayout>
#include <KLocale>
#include <KAction>
#include <QMenu>

BBFilterDock::BBFilterDock(const QString &pCategory, const QString &pTitle, BBCollectionTab *pCollectionTab)
	: QDockWidget(pTitle), mCategory(pCategory), mCollectionTab(pCollectionTab)
{
	setObjectName(pCategory);
	setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
	QWidget *lWidget = new QWidget(this);
	QVBoxLayout *lLayout = new QVBoxLayout;
	mLineEdit = new KLineEdit;
	mListView = new QListView;
	mPushButton = new KPushButton(i18n("Deselect All"));

	lLayout->addWidget(mLineEdit);
	lLayout->addWidget(mPushButton);
	lLayout->addWidget(mListView);
	lLayout->setSpacing(1);

	lWidget->setLayout(lLayout);
	setWidget(lWidget);

	mLineEdit->setClearButtonShown(true);
	mLineEdit->setClickMessage(i18n("Enter filtering text here..."));

	mModel = new BBStringSetModel(this);
	mListView->setModel(mModel);
	mSelectionModel = mListView->selectionModel();
	mListView->setUniformItemSizes(true);
	mListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	mListView->setSelectionBehavior(QAbstractItemView::SelectItems);
	mListView->setContextMenuPolicy(Qt::CustomContextMenu);

	KAction *lEditAction = new KAction(i18n("Edit"), this);
	lEditAction->setShortcut(Qt::Key_F2);
	lEditAction->setShortcutContext(Qt::WidgetShortcut);
	mListView->addAction(lEditAction);
	connect(lEditAction, SIGNAL(triggered()), SLOT(editFilesTriggered()));
	mContextMenu = new QMenu(this);
	mContextMenu->addAction(lEditAction);

	connect(mListView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showPopupMenu(QPoint)));
	connect(mSelectionModel, SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
	        this, SLOT(updateSelection(const QItemSelection &, const QItemSelection &)));
	connect(mLineEdit, SIGNAL(textChanged(const QString &)), SLOT(lineEditChanged(const QString &)));
	connect(mPushButton, SIGNAL(clicked(bool)), mSelectionModel, SLOT(clearSelection()));

}

void BBFilterDock::lineEditChanged(const QString &pNewText)
{
	mFilterTextCondition = QString("%1 LIKE '%%2%'").arg(mCategory, pNewText);
	mCollectionTab->refreshAllExcept(NULL);
}

void BBFilterDock::updateSelection(const QItemSelection &pSelected, const QItemSelection &pDeselected)
{
	foreach(QModelIndex i, pDeselected.indexes())
	{
		int lIndex = mSelection.indexOf(i.data().toString());
		if(lIndex != -1)
			mSelection.removeAt(lIndex);
	}
	foreach(QModelIndex i, pSelected.indexes())
	{
		QString lString = i.data().toString();
		if(mSelection.indexOf(lString) == -1)
			mSelection.append(lString);
	}
	mQuotedSelection = QString("%1 IN %2").arg(mCategory, BBDatabase::prepareList(mSelection));

	mCollectionTab->refreshAllExcept(this);
}

void BBFilterDock::addCondition(bool pUseSelection, QStringList &pList)
{
	if(!pUseSelection)
	{
		if(!mFilterTextCondition.isEmpty())
			pList << mFilterTextCondition;
	}
	else
	{
		if(!mSelection.isEmpty())
			pList << mQuotedSelection;
		else
		{
			if(!mFilterTextCondition.isEmpty())
				pList << mFilterTextCondition;
		}
	}
}

void BBFilterDock::processResult(QSet<BBStringListItem> pResult)
{
	mModel->setNewSet(pResult);
	QModelIndexList lList = mSelectionModel->selectedIndexes();
	if(!lList.isEmpty())
		mListView->scrollTo(lList.first());
}

void BBFilterDock::showPopupMenu(const QPoint &pPoint)
{
	if(mListView->indexAt(pPoint).isValid())
		mContextMenu->popup(mListView->mapToGlobal(pPoint));
}

void BBFilterDock::editFilesTriggered()
{
	QModelIndexList lList = mSelectionModel->selectedIndexes();
	if(lList.isEmpty())
		return; //should never happen...

	QString lOldValue = lList.first().data().toString();
	mCollectionTab->changeFiles(mCategory, lOldValue);
}

void BBFilterDock::forceSelection(const QStringList &pList)
{
	mSelection = pList;
	mQuotedSelection = QString("%1 IN %2").arg(mCategory, BBDatabase::prepareList(mSelection));
}

void BBFilterDock::syncSelection()
{
	foreach(const QString &lEntry, mSelection)
	{
		int lIndex = mModel->findStringPos(lEntry);
		if(lIndex != -1)
		{
			mSelectionModel->select(mModel->index(lIndex), QItemSelectionModel::Select);
		}
	}
	QModelIndexList lList = mSelectionModel->selectedIndexes();
	if(!lList.isEmpty())
		mListView->scrollTo(lList.first());
}
