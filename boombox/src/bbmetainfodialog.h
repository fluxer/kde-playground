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
#ifndef BBMETAINFODIALOG_H
#define BBMETAINFODIALOG_H

#include "bbmetadata.h"
#include "bbdatabase.h"
#include <QDialog>
#include <QFileInfo>
#include <QModelIndex>

class BBCollectionTab;

namespace Ui {
    class BBMetaInfoDialog;
}

class BBMetaInfoDialog : public QDialog
{
	Q_OBJECT
	Q_DISABLE_COPY(BBMetaInfoDialog)
	public:
		BBMetaInfoDialog(QWidget *parent, BBCollectionTab *pCollectionTab);
		virtual ~BBMetaInfoDialog();
		bool fillInValues(const QModelIndex &pModelIndex);

	protected:
		virtual void changeEvent(QEvent *e);

	protected slots:
		void saveValues();
		void guessFieldNames();

	private:
		QStringList splitName(QString lName);

		Ui::BBMetaInfoDialog *mUI;
		BBMetaData mMetaData;
		BBDatabase mDatabase;
		QModelIndex mIndex;
		QFileInfo mFileInfo;
		BBCollectionTab *mCollectionTab;
};

#endif // BBMETAINFODIALOG_H
