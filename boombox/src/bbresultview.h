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
#ifndef BBRESULTVIEW_H
#define BBRESULTVIEW_H

#include <QCheckBox>
#include <QModelIndex>
#include <KDE/KLineEdit>

class QTreeView;
class QPushButton;
class QMenu;
class BBCollectionTab;
class BBAlbumSongModel;
class BBSongListItem;

class BBResultView : public QWidget
{
	Q_OBJECT
	public:
		BBResultView(BBCollectionTab *pCollectionTab, BBAlbumSongModel *pModel);

		QString filterText() {return mLineEdit->text();}
		QSet<BBSongListItem> filesToUpdate();
		void scrollToNewSong(const QModelIndex &pIndexCurrent, const QModelIndex &pIndexPrevious);

	protected slots:
		void lineEditChanged(const QString &pNewText);
		void itemClicked(QModelIndex pIndex);
		void itemExpanded(const QModelIndex &pIndex);
		void itemCollapsed(const QModelIndex &pIndex);
		void showPopupMenu(const QPoint &pPoint);
		void editFilesTriggered();
		void enqueueTriggered();

	protected:
		QTreeView *mTreeView;
		KLineEdit *mLineEdit;
		QPushButton *mResetAllButton;
		BBAlbumSongModel *mModel;
		BBCollectionTab *mCollectionTab;
		QMenu *mContextMenu;

		bool mAutoExpandInProgress;
};

#endif // BBRESULTVIEW_H
