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
#ifndef BBSTREAMSTAB_H
#define BBSTREAMSTAB_H

#include "bbplaylistsystem.h"

#include <QAbstractItemModel>
#include <QDomDocument>

class QHBoxLayout;
class QTreeView;
class KActionCollection;
class KToolBar;

class BBStreamItem;

class BBStreamModel : public QAbstractItemModel
{
public:
	explicit BBStreamModel(const QString &pFileName, QObject *pParent = 0);
	virtual ~BBStreamModel();

	virtual QModelIndex index(int pRow, int pColumn, const QModelIndex &pParent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex &pChild) const;
	virtual int rowCount(const QModelIndex &pParent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &pParent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &pIndex, int pRole = Qt::DisplayRole) const;
	virtual Qt::ItemFlags flags(const QModelIndex &pIndex) const;
	virtual Qt::DropActions supportedDropActions() const;
	virtual bool removeRows(int pRow, int pCount, const QModelIndex &pParent);
	virtual QMimeData *mimeData(const QModelIndexList &pIndexes) const;
	virtual bool dropMimeData(const QMimeData *pData, Qt::DropAction pAction, int pRow, int pColumn, const QModelIndex &pParent);
	virtual QStringList mimeTypes() const;

	QPersistentModelIndex mCurrentSong;

	QModelIndex createNewEntry(const QModelIndex &pParent, const QString &pName, const QString &pUrl, int pRow = -1);
	void editEntry(const QModelIndex &pEntry, const QString &pName, const QString &pUrl);
	void deleteEntry(const QModelIndex &pEntry);
	QModelIndex createNewFolder(const QModelIndex &pParent, const QString &pName, int pRow = -1);
	void saveDocumentToFile();
	void getInfo(const QModelIndex &pModelIndex, QString &pName, QString &pUrl);

protected:
	void encodeNode(QDataStream &lDataStream, QModelIndex pIndex) const;
	bool decodeNode(QDataStream &lDataStream, QModelIndex pParent, int pRow);

	QDomDocument mDocument;
	QString mFileName;
	BBStreamItem *mRoot;
};

class BBStreamsTab : public BBPlaylistSystem
{
Q_OBJECT
public:
	explicit BBStreamsTab(int pTabNumber);
	virtual void queryNextSong(BBSongQueryJob &pJob);
	virtual void queryPreviousSong(BBSongQueryJob &pJob);
	virtual QVariant currentSong();
	virtual void setCurrentSong(const QVariant &pSong);
	virtual QString displayString(const QVariant &pSong);
	virtual KUrl songUrl(const QVariant &pSong);

	void addManualUrl(const KUrl &pUrl, const QString &pTitle);

	virtual void readSession(KConfigGroup &pConfigGroup);
	virtual void saveSession(KConfigGroup &pConfigGroup);
	virtual void embedControls(QWidget *pControls);

protected slots:
	void createNewStream();
	void createNewFolder();
	void showPopupMenu(const QPoint &pPoint);
	void editTriggered();
	void deleteTriggered();
	void entryClicked(QModelIndex pIndex);
	void enqueueTriggered();

protected:
	void buildSearchStack(bool pSearchForward);
	void findSongInSearchStack(bool pSearchForward, BBSongQueryJob &pJob);
	QList<QModelIndex> mSearchStack;

	KToolBar *mControlToolBar;
	QWidget *mControlsContainer;
	QHBoxLayout *mControlsLayout;
	QWidget *mCenterWidget;
	QTreeView *mTreeView;
	BBStreamModel *mModel;
	KActionCollection *mActionCollection;
	QMenu *mContextMenu;
};

#endif // BBSTREAMSTAB_H
