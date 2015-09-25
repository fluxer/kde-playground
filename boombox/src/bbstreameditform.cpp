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
#include "bbstreameditform.h"
#include "ui_bbstreameditform.h"

BBStreamEditForm::BBStreamEditForm(const QString &pName, const QString &pUrl, QWidget *parent)
	:QDialog(parent), ui(new Ui::BBStreamEditForm)
{
	ui->setupUi(this);
	ui->nameEdit->setText(pName);
	ui->urlEdit->setText(pUrl);
}

BBStreamEditForm::~BBStreamEditForm()
{
	delete ui;
}

void BBStreamEditForm::changeEvent(QEvent *e)
{
	QDialog::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}

void BBStreamEditForm::accept()
{
	mName = ui->nameEdit->text();
	mUrl = ui->urlEdit->text();
	QDialog::accept();
}
