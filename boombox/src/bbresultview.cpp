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
#include "bbresultview.h"
#include "bbplaylistsystem.h"
#include "bbsetmodel.h"
#include "bblistitem.h"
#include "bbcollectiontab.h"
#include "bbmetainfodialog.h"
#include "bbmainwindow.h"

#include <KLineEdit>
#include <QTreeView>
#include <QLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QPainter>
#include <KLocale>
#include <KAction>
#include <QMenu>
#include <QApplication>
#include <QStyledItemDelegate>

const int gTextMargin = 2;

class BBSongItemDelegate : public QStyledItemDelegate
{
	public:
		virtual void paint(QPainter *pPainter, const QStyleOptionViewItem &pOption, const QModelIndex &pIndex) const;
		virtual QSize sizeHint(const QStyleOptionViewItem &pOption, const QModelIndex &pIndex) const;
};

void BBSongItemDelegate::paint(QPainter *pPainter, const QStyleOptionViewItem &pOption, const QModelIndex &pIndex) const
{
	QStyleOptionViewItemV4 lOption = pOption;
	initStyleOption(&lOption, pIndex);
	QStyle *style = QApplication::style();

	pPainter->save();
	style->drawPrimitive(QStyle::PE_PanelItemViewItem, &lOption, pPainter);
	QRectF r = pOption.rect;
	r.adjust(gTextMargin, 0, -gTextMargin, 0);
	pPainter->setFont(lOption.font);
	if(lOption.state & QStyle::State_Selected)
		pPainter->setPen(lOption.palette.color(QPalette::Active, QPalette::HighlightedText));
	else
		pPainter->setPen(lOption.palette.color(QPalette::Active, QPalette::Text));

	if(pIndex.internalPointer() != NULL)
	{
		QRectF lLengthRect;
		pPainter->drawText(r, Qt::AlignVCenter | Qt::AlignRight, pIndex.data(BBLengthRole).toString(), &lLengthRect);
		r.setRight(lLengthRect.left() - gTextMargin);
		pPainter->drawText(r, Qt::AlignVCenter, pIndex.data(Qt::DisplayRole).toString());
	}
	else
	{
		int lOriginalFontSize = lOption.font.pointSize();
		lOption.font.setPointSize(lOriginalFontSize + 2);
		pPainter->setFont(lOption.font);
		QFontMetrics lFontMetric(lOption.font);
		QString lArtist = pIndex.data(BBArtistRole).toString();
		pPainter->drawText(r.left() + gTextMargin, r.top() + lFontMetric.ascent(), lArtist);
		lOption.font.setPointSize(lOriginalFontSize + 1);
		pPainter->setFont(lOption.font);
		pPainter->drawText(r.left() + lFontMetric.width(lArtist) + 6*gTextMargin, r.top() + lFontMetric.ascent(),
								 pIndex.data(BBAlbumRole).toString());
	}
	pPainter->restore();
}

QSize BBSongItemDelegate::sizeHint(const QStyleOptionViewItem &pOption, const QModelIndex &pIndex) const
{
	QSize lSize = QStyledItemDelegate::sizeHint(pOption, pIndex);
	if(!pIndex.parent().isValid())
	{
		QFont lFont = pOption.font;
		lFont.setPointSize(lFont.pointSize() + 2);
		QFontMetrics lFontMetrics(lFont);
		lSize.setHeight(lFontMetrics.height());
	}
	return lSize;
}

BBResultView::BBResultView(BBCollectionTab *pCollectionTab, BBAlbumSongModel *pModel)
	: QWidget(pCollectionTab), mModel(pModel), mCollectionTab(pCollectionTab)
{
	QVBoxLayout *lLayout = new QVBoxLayout;
	mLineEdit = new KLineEdit;
	mTreeView = new QTreeView;
	QHBoxLayout *lHLayout = new QHBoxLayout;
	mResetAllButton = new QPushButton(i18n("Reset All Filters"));

	lHLayout->addWidget(mLineEdit);
	lHLayout->addWidget(mResetAllButton);
	lLayout->addLayout(lHLayout);
	lLayout->addWidget(mTreeView);
	lLayout->setSpacing(1);

	setLayout(lLayout);

	mLineEdit->setClearButtonShown(true);
	mLineEdit->setClickMessage(i18n("Enter filtering text here..."));

	mTreeView->setModel(pModel);
	mTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	mTreeView->setHeaderHidden(true);
	mTreeView->setAlternatingRowColors(true);
	mTreeView->setAnimated(true);
	mTreeView->setItemDelegate(new BBSongItemDelegate);
	mTreeView->setContextMenuPolicy(Qt::CustomContextMenu);

	KAction *lEditAction = new KAction(i18n("Edit"), this);
	lEditAction->setShortcut(Qt::Key_F2);
	lEditAction->setShortcutContext(Qt::WidgetShortcut);
	mTreeView->addAction(lEditAction);
	connect(lEditAction, SIGNAL(triggered()), SLOT(editFilesTriggered()));

	KAction *lEnqueueAction = new KAction(i18n("Add to Queue"), this);
	lEnqueueAction->setShortcut(Qt::CTRL + Qt::Key_E);
	lEnqueueAction->setShortcutContext(Qt::WidgetShortcut);
	mTreeView->addAction(lEnqueueAction);
	connect(lEnqueueAction, SIGNAL(triggered()), SLOT(enqueueTriggered()));

	mContextMenu = new QMenu(this);
	mContextMenu->addAction(lEnqueueAction);
	mContextMenu->addAction(lEditAction);

	mAutoExpandInProgress = false;

	connect(mLineEdit, SIGNAL(textChanged(const QString &)), SLOT(lineEditChanged(const QString &)));
	connect(mTreeView, SIGNAL(activated(QModelIndex)), SLOT(itemClicked(QModelIndex)));
	connect(mTreeView, SIGNAL(expanded(QModelIndex)), SLOT(itemExpanded(QModelIndex)));
	connect(mTreeView, SIGNAL(collapsed(QModelIndex)), SLOT(itemCollapsed(QModelIndex)));
	connect(mTreeView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showPopupMenu(QPoint)));
	connect(mResetAllButton, SIGNAL(clicked()), mCollectionTab, SLOT(resetAllFilters()));
}

void BBResultView::lineEditChanged(const QString &)
{
	mCollectionTab->refreshAllExcept(NULL);
}

QSet<BBSongListItem> BBResultView::filesToUpdate()
{
	QSet<BBSongListItem> lSet;
	QItemSelectionModel *lSelectionModel = mTreeView->selectionModel();
	if(lSelectionModel->hasSelection())
	{
		QModelIndexList lIndexes = lSelectionModel->selectedIndexes();
		foreach(QModelIndex i, lIndexes)
		{
			if(i.parent().isValid()) //only song items, not albums
				lSet.insert(mModel->findSongItem(i));
		}
	}
	else
	{
		lSet = mModel->allSongs();
	}
	return lSet;
}

void BBResultView::itemClicked(QModelIndex pIndex)
{
	if(pIndex.parent().isValid())
	{
		gMainWindow->setCurrentPlaylistSystem(mCollectionTab->tabNumber());
		gMainWindow->setCurrentSong(pIndex.data(BBFileIDRole));
	}
}

void BBResultView::scrollToNewSong(const QModelIndex &pIndexCurrent, const QModelIndex &pIndexPrevious)
{
	mTreeView->setAnimated(false);
	QModelIndex lPreviousAlbum(pIndexPrevious.parent());
	if(lPreviousAlbum != pIndexCurrent.parent() && !mModel->isManuallyExpanded(lPreviousAlbum))
		mTreeView->collapse(lPreviousAlbum);

	mAutoExpandInProgress = true;
	mTreeView->scrollTo(pIndexCurrent);
	mAutoExpandInProgress = false;
	mTreeView->setAnimated(true);
}

void BBResultView::itemExpanded(const QModelIndex &pIndex)
{
	if(!mAutoExpandInProgress)
		mModel->setManuallyExpanded(pIndex, true);
}

void BBResultView::itemCollapsed(const QModelIndex &pIndex)
{
	mModel->setManuallyExpanded(pIndex, false);
}

void BBResultView::showPopupMenu(const QPoint &pPoint)
{
	QModelIndex lIndex = mTreeView->indexAt(pPoint);
	if(lIndex.isValid() && lIndex.parent().isValid())
		mContextMenu->popup(mTreeView->mapToGlobal(pPoint));
}

void BBResultView::editFilesTriggered()
{
	QItemSelectionModel *lSelectionModel = mTreeView->selectionModel();
	QModelIndex lIndex = lSelectionModel->currentIndex();
	if(!lIndex.isValid() || !lIndex.parent().isValid())
		return;

	BBMetaInfoDialog *lDialog = new BBMetaInfoDialog(mCollectionTab, mCollectionTab);
	connect(lDialog, SIGNAL(finished(int)), lDialog, SLOT(deleteLater()));
	if(lDialog->fillInValues(lIndex))
		lDialog->show();
}

void BBResultView::enqueueTriggered()
{
	QItemSelectionModel *lSelectionModel = mTreeView->selectionModel();
	QModelIndex lIndex = lSelectionModel->currentIndex();
	if(!lIndex.isValid() || !lIndex.parent().isValid())
		return;

	QString lDisplayText = QString("%1 - %2").arg(lIndex.data(BBArtistRole).toString(), lIndex.data(BBTitleRole).toString());
	gMainWindow->addToPlayQueue(lIndex.data(BBFileIDRole), lDisplayText, mCollectionTab->tabNumber());
}

