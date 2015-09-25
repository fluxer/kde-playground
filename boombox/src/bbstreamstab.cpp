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

#include "bbstreamstab.h"
#include "bbsettings.h"
#include "bbstreameditform.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QLayout>
#include <QMimeData>
#include <QTreeView>

#include <KAction>
#include <KActionCollection>
#include <KInputDialog>
#include <KStandardAction>
#include <KToolBar>

Q_DECLARE_METATYPE(QPersistentModelIndex)

class BBStreamItem
{
public:
	explicit BBStreamItem(BBStreamItem *pParent, int pRow, QDomElement &pElement);
	~BBStreamItem();

	BBStreamItem *mParent;
	int mRow;
	QDomElement mElement;
	QList<BBStreamItem *> mChildren;
};

BBStreamItem::BBStreamItem(BBStreamItem *pParent, int pRow, QDomElement &pElement)
	: mParent(pParent), mRow(pRow), mElement(pElement)
{
	int lCurrentRow = 0;
	QDomElement lCurrent = pElement.firstChildElement();
	for(; !lCurrent.isNull(); lCurrent = lCurrent.nextSiblingElement())
		mChildren.append(new BBStreamItem(this, lCurrentRow++, lCurrent));
}

BBStreamItem::~BBStreamItem()
{
	foreach(BBStreamItem *lCurrent, mChildren)
		delete lCurrent;
}

BBStreamModel::BBStreamModel(const QString &pFileName, QObject *pParent)
	: QAbstractItemModel(pParent), mFileName(pFileName)
{
	QFile lXmlFile(pFileName);
	if(!lXmlFile.exists() || !lXmlFile.open(QIODevice::ReadOnly) || !mDocument.setContent(&lXmlFile))
	{
		lXmlFile.close();
		if(!lXmlFile.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate))
			return;
		QTextStream lTextStream(&lXmlFile);
		lTextStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
		lTextStream << "<boombox_streams_db>" << endl;
		lTextStream << "<folder name=\"Example Streams\">" << endl;
		lTextStream << "\t<entry href=\"http://scfire-dtc-aa01.stream.aol.com:80/stream/1065\" name=\"Digitally Imported - Vocal Trance\"></entry>" << endl;
		lTextStream << "\t<entry href=\"http://tai-03.egihosting.com/amstrance-64k-aac\" name=\"1.FM - Amsterdam Trance Radio\"></entry>" << endl;
		lTextStream << "\t<entry href=\"http://stream.pulsradio.com:5000\" name=\"PulsRadio - www.pulsradio.com\"></entry>" << endl;
		lTextStream << "\t<entry href=\"http://m1live-shoutcast.returning.net:8000\" name=\"MusicOne\"></entry>" << endl;
		lTextStream << "</folder>" << endl;
		lTextStream << "</boombox_streams_db>" << endl;
		lXmlFile.seek(0);
		if(!mDocument.setContent(&lXmlFile))
		{
			qWarning() <<"Giving up getting a valid XML file.";
			return;
		}
	}
	QDomElement lDocElement = mDocument.documentElement();
	mRoot = new BBStreamItem(0, 0, lDocElement);
}

BBStreamModel::~BBStreamModel()
{
	delete mRoot;
}

QModelIndex BBStreamModel::index(int pRow, int pColumn, const QModelIndex &pParent) const
{
	BBStreamItem *lParentItem;
	if(!pParent.isValid())
		lParentItem = mRoot;
	else
		lParentItem = static_cast<BBStreamItem *>(pParent.internalPointer());

	if(pRow < 0 || pRow >= lParentItem->mChildren.count() || pColumn != 0)
		return QModelIndex();
	else
		return createIndex(pRow, pColumn, lParentItem->mChildren.at(pRow));
}

QModelIndex BBStreamModel::parent(const QModelIndex &pChild) const
{
	if(!pChild.isValid())
		return QModelIndex();
	else
	{
		BBStreamItem *lChildItem = static_cast<BBStreamItem *>(pChild.internalPointer());
		if(lChildItem->mParent == mRoot)
			return QModelIndex();
		else
			return createIndex(lChildItem->mParent->mRow, 0, lChildItem->mParent);
	}
}

int BBStreamModel::rowCount(const QModelIndex &pParent) const
{
	BBStreamItem *lParentItem;
	if(!pParent.isValid())
		lParentItem = mRoot;
	else
		lParentItem = static_cast<BBStreamItem *>(pParent.internalPointer());

	return lParentItem->mChildren.count();
}

int BBStreamModel::columnCount(const QModelIndex &pParent) const
{
	Q_UNUSED(pParent)
	return 1;
}

QVariant BBStreamModel::data(const QModelIndex &pIndex, int pRole) const
{
	if(!pIndex.isValid())
		return QVariant();
	BBStreamItem *lStreamItem = static_cast<BBStreamItem *>(pIndex.internalPointer());
	QString lType = lStreamItem->mElement.tagName();
	if(lType == "folder")
	{
		switch(pRole)
		{
		case Qt::DisplayRole:
			return lStreamItem->mElement.attribute("name");
		case BBIsFolderRole:
			return true;
		case Qt::FontRole:
			if(mCurrentSong.parent() == pIndex)
			{
				QFont lFont = QApplication::font();
				lFont.setBold(true);
				return lFont;
			}
			else
				return QVariant();
		}
	}
	else if(lType == "entry")
	{
		switch(pRole)
		{
		case Qt::DisplayRole:
			return lStreamItem->mElement.attribute("name");
		case BBIsFolderRole:
			return false;
		case BBUrlRole:
			return lStreamItem->mElement.attribute("href");
		case Qt::FontRole:
			if(mCurrentSong == pIndex)
			{
				QFont lFont = QApplication::font();
				lFont.setBold(true);
				return lFont;
			}
			else
				return QVariant();
		}
	}
	return QVariant();
}

QModelIndex BBStreamModel::createNewEntry(const QModelIndex &pParent, const QString &pName, const QString &pUrl, int pRow)
{
	QDomElement lNewEntry = mDocument.createElement("entry");
	lNewEntry.setAttribute("name", pName);
	lNewEntry.setAttribute("href", pUrl);

	BBStreamItem *lParentItem;
	QModelIndex lParentIndex = pParent;
	if(!lParentIndex.isValid())
		lParentItem = mRoot;
	else
	{
		lParentItem = static_cast<BBStreamItem *>(pParent.internalPointer());
		if(lParentItem->mElement.tagName() == "entry")
		{
			lParentItem = lParentItem->mParent;
			lParentIndex = lParentIndex.parent();
		}
	}
	int lRow = (pRow == -1 ? rowCount(lParentIndex): pRow);
	beginInsertRows(lParentIndex, lRow, lRow);
	if(pRow == -1)
	{
		lParentItem->mElement.appendChild(lNewEntry);
		lParentItem->mChildren.append(new BBStreamItem(lParentItem, lRow, lNewEntry));
	}
	else
	{
		lParentItem->mElement.insertBefore(lNewEntry, lParentItem->mElement.childNodes().item(lRow));
		lParentItem->mChildren.insert(lRow, new BBStreamItem(lParentItem, lRow, lNewEntry));
		for(int i = lRow + 1; i < lParentItem->mChildren.count(); ++i)
			lParentItem->mChildren[i]->mRow++;
	}
	endInsertRows();
	return index(lRow, 0, lParentIndex);
}

void BBStreamModel::editEntry(const QModelIndex &pEntry, const QString &pName, const QString &pUrl)
{
	if(!pEntry.isValid())
		return;
	BBStreamItem *lItem = static_cast<BBStreamItem *>(pEntry.internalPointer());
	if(!pName.isEmpty())
		lItem->mElement.setAttribute("name", pName);
	if(!pUrl.isEmpty())
		lItem->mElement.setAttribute("href", pUrl);
}

void BBStreamModel::deleteEntry(const QModelIndex &pEntry)
{
	if(!pEntry.isValid())
		return;
	BBStreamItem *lItem = static_cast<BBStreamItem *>(pEntry.internalPointer());
	beginRemoveRows(pEntry.parent(), lItem->mRow, lItem->mRow);
	lItem->mParent->mChildren.removeAt(lItem->mRow);
	lItem->mParent->mElement.removeChild(lItem->mElement);
	for(int i = lItem->mRow; i < lItem->mParent->mChildren.count(); ++i)
		lItem->mParent->mChildren[i]->mRow--;
	delete lItem;
	endRemoveRows();
}

QModelIndex BBStreamModel::createNewFolder(const QModelIndex &pParent, const QString &pName, int pRow)
{
	QDomElement lNewEntry = mDocument.createElement("folder");
	lNewEntry.setAttribute("name", pName);

	BBStreamItem *lParentItem;
	QModelIndex lParentIndex = pParent;
	if(!lParentIndex.isValid())
		lParentItem = mRoot;
	else
	{
		lParentItem = static_cast<BBStreamItem *>(pParent.internalPointer());
		if(lParentItem->mElement.tagName() == "entry")
		{
			lParentItem = lParentItem->mParent;
			lParentIndex = lParentIndex.parent();
		}
	}
	int lRow = (pRow == -1 ? rowCount(lParentIndex) : pRow);
	beginInsertRows(lParentIndex, lRow, lRow);
	if(pRow == -1)
	{
		lParentItem->mElement.appendChild(lNewEntry);
		lParentItem->mChildren.append(new BBStreamItem(lParentItem, lRow, lNewEntry));
	}
	else
	{
		lParentItem->mElement.insertBefore(lNewEntry, lParentItem->mElement.childNodes().item(lRow));
		lParentItem->mChildren.insert(pRow, new BBStreamItem(lParentItem, lRow, lNewEntry));
	}
	endInsertRows();
	return index(lRow, 0, lParentIndex);
}

void BBStreamModel::saveDocumentToFile()
{
	QFile lXmlFile(mFileName);
	if(lXmlFile.open(QIODevice::WriteOnly))
	{
		QTextStream lTextStream(&lXmlFile);
		mDocument.save(lTextStream, 4);
	}
}

void BBStreamModel::getInfo(const QModelIndex &pModelIndex, QString &pName, QString &pUrl)
{
	if(!pModelIndex.isValid())
		return;
	BBStreamItem *lItem = static_cast<BBStreamItem *>(pModelIndex.internalPointer());

	pName = lItem->mElement.attribute("name");
	pUrl = lItem->mElement.attribute("href");
}

Qt::ItemFlags BBStreamModel::flags(const QModelIndex &pIndex) const
{
	Qt::ItemFlags lFlags = QAbstractItemModel::flags(pIndex);
	if(pIndex.isValid())
	{
		lFlags |= Qt::ItemIsDragEnabled;
		if(pIndex.data(BBIsFolderRole).toBool())
			lFlags |= Qt::ItemIsDropEnabled;
		return lFlags;
	}
	else
		return Qt::ItemIsDropEnabled | lFlags;
}

Qt::DropActions BBStreamModel::supportedDropActions() const
{
	return Qt::MoveAction;
}

bool BBStreamModel::removeRows(int pRow, int pCount, const QModelIndex &pParent)
{
	BBStreamItem *lParentItem;
	if(!pParent.isValid())
		lParentItem = mRoot;
	else
		lParentItem = static_cast<BBStreamItem *>(pParent.internalPointer());

	beginRemoveRows(pParent, pRow, pRow + pCount - 1);
	for(int i = pRow; i < pRow + pCount && i < lParentItem->mChildren.count(); ++i)
	{
		BBStreamItem *lItem = lParentItem->mChildren.takeAt(i);
		lParentItem->mElement.removeChild(lItem->mElement);
		delete lItem;
	}
	for(int i = pRow + pCount - 1; i < lParentItem->mChildren.count(); ++i)
		lParentItem->mChildren[i]->mRow -= pCount;

	endRemoveRows();
	saveDocumentToFile();
	return true;
}

void BBStreamModel::encodeNode(QDataStream &lDataStream, QModelIndex pIndex) const
{
	if(pIndex.data(BBIsFolderRole).toBool())
	{
		lDataStream << QString::fromAscii("folder");
		lDataStream << pIndex.data(Qt::DisplayRole).toString();
		qint32 lRowCount = rowCount(pIndex);
		lDataStream << lRowCount;
		for(qint32 i = 0; i < lRowCount; ++i)
			encodeNode(lDataStream, index(i, 0, pIndex));
	}
	else
	{
		lDataStream << QString::fromAscii("entry");
		lDataStream << pIndex.data(Qt::DisplayRole).toString();
		lDataStream << pIndex.data(BBUrlRole).toString();
	}
}

bool BBStreamModel::decodeNode(QDataStream &lDataStream, QModelIndex pParent, int pRow)
{
	qint32 lRowCount;
	QString lType, lUrl, lName;
	lDataStream >> lType;
	if(lType == "entry")
	{
		lDataStream >> lName;
		lDataStream >> lUrl;
		createNewEntry(pParent, lName, lUrl, pRow);
	}
	else
	{
		lDataStream >> lName;
		QModelIndex i = pParent;
		while(i.isValid())
		{
			if(i.data().toString() == lName)
				return false;
			i = i.parent();
		}
		QModelIndex lNewFolderIndex = createNewFolder(pParent, lName, pRow);
		lDataStream >> lRowCount;
		for(qint32 i = 0; i < lRowCount; ++i)
			decodeNode(lDataStream, lNewFolderIndex, i);
	}
	return true;
}


QMimeData *BBStreamModel::mimeData(const QModelIndexList &pIndexes) const
{
	QMimeData *lMimeData = new QMimeData();
	QByteArray lByteArray;
	QDataStream lDataStream(&lByteArray, QIODevice::WriteOnly);
	foreach(QModelIndex lIndex, pIndexes)
		encodeNode(lDataStream, lIndex);
	lMimeData->setData("application/x-boombox-streamtab", lByteArray);
	return lMimeData;
}

bool BBStreamModel::dropMimeData(const QMimeData *pData, Qt::DropAction pAction, int pRow, int pColumn, const QModelIndex &pParent)
{
	Q_UNUSED(pAction)
	Q_UNUSED(pColumn)
	if(!pData->hasFormat("application/x-boombox-streamtab"))
		return false;
	QByteArray lByteArray = pData->data("application/x-boombox-streamtab");
	QDataStream lDataStream(&lByteArray, QIODevice::ReadOnly);
	return decodeNode(lDataStream, pParent, pRow);
}

QStringList BBStreamModel::mimeTypes() const
{
	QStringList lList;
	lList << "application/x-boombox-streamtab";
	return lList;
}

BBStreamsTab::BBStreamsTab(int pTabNumber)
   :BBPlaylistSystem(pTabNumber)
{
	setWindowTitle(i18n("Internet Streams"));
}

void BBStreamsTab::readSession(KConfigGroup &pConfigGroup)
{
	Q_UNUSED(pConfigGroup)
	mActionCollection = new KActionCollection(this);
	KAction *lNewStreamAction = mActionCollection->addAction("new_stream");
	lNewStreamAction->setText(i18n("New Stream"));
	lNewStreamAction->setIcon(KIcon("document-new"));
	connect(lNewStreamAction, SIGNAL(triggered()), this, SLOT(createNewStream()));

	KAction *lNewFolderAction = mActionCollection->addAction("new_folder");
	lNewFolderAction->setText(i18n("New Folder"));
	lNewFolderAction->setIcon(KIcon("folder-new"));
	connect(lNewFolderAction, SIGNAL(triggered()), this, SLOT(createNewFolder()));


	KAction *lEnqueueAction = new KAction(i18n("Add to Queue"), this);
	lEnqueueAction->setShortcut(Qt::CTRL + Qt::Key_E);
	lEnqueueAction->setShortcutContext(Qt::WidgetShortcut);
	connect(lEnqueueAction, SIGNAL(triggered()), SLOT(enqueueTriggered()));

	KAction *lEditAction = new KAction(i18n("Edit"), this);
	lEditAction->setShortcut(Qt::Key_F2);
	lEditAction->setShortcutContext(Qt::WidgetShortcut);
	connect(lEditAction, SIGNAL(triggered()), SLOT(editTriggered()));

	KAction *lDeleteAction = new KAction(i18n("Delete"), this);
	lDeleteAction->setShortcut(Qt::Key_Delete);
	lDeleteAction->setShortcutContext(Qt::WidgetShortcut);
	connect(lDeleteAction, SIGNAL(triggered()), SLOT(deleteTriggered()));

	mContextMenu = new QMenu(this);
	mContextMenu->addAction(lEnqueueAction);
	mContextMenu->addAction(lEditAction);
	mContextMenu->addAction(lDeleteAction);

	mControlToolBar = toolBar("fs_control_toolbar");
	mControlToolBar->setWindowTitle(i18n("Playback Control Toolbar"));
	mControlToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
	mControlsContainer = new QWidget(this);
	mControlsLayout = new QHBoxLayout(mControlsContainer);
	mControlToolBar->addAction(lNewStreamAction);
	mControlToolBar->addAction(lNewFolderAction);
	mControlToolBar->addWidget(mControlsContainer);

	mCenterWidget = new QWidget(this);
	mTreeView = new QTreeView(this);
	mTreeView->setHeaderHidden(true);
	mTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
	mTreeView->setDragDropMode(QAbstractItemView::InternalMove);
	mTreeView->setAcceptDrops(true);
	mTreeView->setDragEnabled(true);
	mTreeView->setDropIndicatorShown(true);
	QVBoxLayout *lCenterLayout = new QVBoxLayout(mCenterWidget);
	mModel = new BBStreamModel(BBSettings::fileNameStreams(), this);
	mTreeView->setModel(mModel);
	mTreeView->expandAll();
	mTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
	mTreeView->addAction(lEnqueueAction);
	mTreeView->addAction(lEditAction);
	mTreeView->addAction(lDeleteAction);
	lCenterLayout->addWidget(mTreeView);
	setCentralWidget(mCenterWidget);

	connect(mTreeView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showPopupMenu(QPoint)));
	connect(mTreeView, SIGNAL(activated(QModelIndex)), this, SLOT(entryClicked(QModelIndex)));
}

void BBStreamsTab::saveSession(KConfigGroup &pConfigGroup)
{
	Q_UNUSED(pConfigGroup)
	mModel->saveDocumentToFile();
}

void BBStreamsTab::embedControls(QWidget *pControls)
{
	pControls->setParent(mControlsContainer);
	mControlsLayout->addWidget(pControls);
	pControls->show();
}

void BBStreamsTab::buildSearchStack(bool pSearchForward)
{
	QModelIndex lCurrent = mModel->mCurrentSong;
	if(lCurrent.isValid())
	{
		if(pSearchForward)
		{
			do
			{
				int lStartRow = lCurrent.row() + 1;
				lCurrent = lCurrent.parent();
				int lMax = mModel->rowCount(lCurrent);
				for(int i = lStartRow; i < lMax; ++i)
					mSearchStack.append(mModel->index(i, 0, lCurrent));
			}while(lCurrent.isValid());
		}
		else
		{
			do
			{
				int lStartRow = lCurrent.row() - 1;
				lCurrent = lCurrent.parent();
				for(int i = lStartRow; i >= 0; --i)
					mSearchStack.append(mModel->index(i, 0, lCurrent));
			}while(lCurrent.isValid());
		}
	}
	else
	{
		if(pSearchForward)
		{
			int lMax = mModel->rowCount();
			for(int i = 0; i < lMax; ++i)
				mSearchStack.append(mModel->index(i, 0));
		}
		else
		{
			for(int i = mModel->rowCount() - 1; i >= 0; --i)
				mSearchStack.append(mModel->index(i, 0));
		}
	}
}

void BBStreamsTab::findSongInSearchStack(bool pSearchForward, BBSongQueryJob &pJob)
{
	while(!mSearchStack.isEmpty())
	{
		QPersistentModelIndex lCurrent = mSearchStack.takeFirst();
		if(!lCurrent.isValid())
			continue;
		if(!lCurrent.data(BBIsFolderRole).toBool())
		{
			pJob.mSong = QVariant::fromValue(lCurrent);
			emit songQueryReady(pJob);
			mSearchStack.clear();
			return;
		}
		else
		{
			if(pSearchForward)
			{
				for(int i = mModel->rowCount(lCurrent) - 1; i >= 0; --i)
					mSearchStack.prepend(mModel->index(i, 0, lCurrent));
			}
			else
			{
				for(int i = 0; i < mModel->rowCount(lCurrent); ++i)
					mSearchStack.prepend(mModel->index(i, 0, lCurrent));
			}
		}
	}
}

void BBStreamsTab::queryNextSong(BBSongQueryJob &pJob)
{
	mSearchStack.clear();
	buildSearchStack(true);
	findSongInSearchStack(true, pJob);
}

void BBStreamsTab::queryPreviousSong(BBSongQueryJob &pJob)
{
	mSearchStack.clear();
	buildSearchStack(false);
	findSongInSearchStack(false, pJob);
}

QVariant BBStreamsTab::currentSong()
{
	return QVariant::fromValue(mModel->mCurrentSong);
}

void BBStreamsTab::setCurrentSong(const QVariant &pSong)
{
	mTreeView->update(mModel->mCurrentSong);
	mTreeView->update(mModel->mCurrentSong.parent());
	mModel->mCurrentSong = pSong.value<QPersistentModelIndex>();
	mTreeView->scrollTo(mModel->mCurrentSong);
	mTreeView->update(mModel->mCurrentSong);
	mTreeView->update(mModel->mCurrentSong.parent());
}

QString BBStreamsTab::displayString(const QVariant &pSong)
{
	return pSong.value<QPersistentModelIndex>().data(Qt::DisplayRole).toString();
}

KUrl BBStreamsTab::songUrl(const QVariant &pSong)
{
	QPersistentModelIndex lSong = pSong.value<QPersistentModelIndex>();
	if(!lSong.isValid())
		return KUrl();
	else
		return KUrl(lSong.data(BBUrlRole).toString());
}

void BBStreamsTab::createNewStream()
{
	QString lName, lUrl;
	BBStreamEditForm *lDialog = new BBStreamEditForm(lName, lUrl, this);
	if(QDialog::Accepted == lDialog->exec())
	{
		mModel->createNewEntry(mTreeView->selectionModel()->currentIndex(), lDialog->mName, lDialog->mUrl);
		mModel->saveDocumentToFile();
	}
	delete lDialog;
}

void BBStreamsTab::createNewFolder()
{
	QString lNewName = KInputDialog::getText(i18nc("dialog caption", "New Folder"),
	                                         i18n("Please provide a name for the new folder:"),
	                                         i18nc("default text", "New Folder"));
	if(lNewName.isNull())
		return;
	mModel->createNewFolder(mTreeView->selectionModel()->currentIndex(), lNewName);
	mModel->saveDocumentToFile();
}

void BBStreamsTab::showPopupMenu(const QPoint &pPoint)
{
	QModelIndex lIndex = mTreeView->indexAt(pPoint);
	if(lIndex.isValid())
		mContextMenu->popup(mTreeView->mapToGlobal(pPoint));
}

void BBStreamsTab::editTriggered()
{
	QString lName, lUrl;
	mModel->getInfo(mTreeView->selectionModel()->currentIndex(), lName, lUrl);
	if(mTreeView->selectionModel()->currentIndex().data(BBIsFolderRole).toBool())
	{
		QString lNewName = KInputDialog::getText(i18nc("dialog caption", "New Folder"),
		                                         i18n("Please provide a new name for the folder:"), lName);
		if(lNewName.isNull())
			return;
		mModel->editEntry(mTreeView->selectionModel()->currentIndex(), lNewName, lUrl);
		mModel->saveDocumentToFile();
	}
	else
	{
		BBStreamEditForm *lDialog = new BBStreamEditForm(lName, lUrl, this);
		if(QDialog::Accepted == lDialog->exec())
		{
			mModel->editEntry(mTreeView->selectionModel()->currentIndex(), lDialog->mName, lDialog->mUrl);
			mModel->saveDocumentToFile();
		}
		delete lDialog;
	}
}

void BBStreamsTab::deleteTriggered()
{
	mModel->deleteEntry(mTreeView->selectionModel()->currentIndex());
	mModel->saveDocumentToFile();
}

void BBStreamsTab::entryClicked(QModelIndex pIndex)
{
	if(pIndex.isValid())
	{
		QPersistentModelIndex lIndex = pIndex;
		gMainWindow->setCurrentPlaylistSystem(tabNumber());
		gMainWindow->setCurrentSong(QVariant::fromValue(lIndex));
	}
}

void BBStreamsTab::enqueueTriggered()
{
	QModelIndex lIndex = mTreeView->selectionModel()->currentIndex();
	if(!lIndex.data(BBIsFolderRole).toBool())
	{
		gMainWindow->addToPlayQueue(QVariant::fromValue(QPersistentModelIndex(lIndex)),
		                            lIndex.data().toString(), tabNumber());
	}
}

void BBStreamsTab::addManualUrl(const KUrl &pUrl, const QString &pTitle)
{ //TODO: search for URL match in existing entries, don't add if already there.
	QModelIndex lIndex = mModel->createNewEntry(QModelIndex(), pTitle, pUrl.url());
	gMainWindow->addToPlayQueue(QVariant::fromValue(QPersistentModelIndex(lIndex)), pTitle, tabNumber());
}
