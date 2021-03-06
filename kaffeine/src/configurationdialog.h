/*
 * configurationdialog.h
 *
 * Copyright (C) 2011 Christoph Pfister <christophpfister@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef CONFIGURATIONDIALOG_H
#define CONFIGURATIONDIALOG_H

#include <KPageDialog>

#include <QSpinBox>
class KComboBox;

class ConfigurationDialog : public KPageDialog
{
	Q_OBJECT
public:
	explicit ConfigurationDialog(QWidget *parent);
	~ConfigurationDialog();

	void accept();

private slots:
	void showDmesg();

private:
	KComboBox *startupDisplayModeBox;
	QSpinBox *shortSkipBox;
	QSpinBox *longSkipBox;
};

#endif /* CONFIGURATIONDIALOG_H */
