/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef NOTEUTILS_H
#define NOTEUTILS_H

#include "noteshared_export.h"
#include <Akonadi/Item>
#include <QString>
#include <QWidget>
namespace NoteShared
{
namespace NoteUtils {
NOTESHARED_EXPORT bool sendToMail(QWidget *parent, const QString &title, const QString &message);
NOTESHARED_EXPORT void sendToNetwork(QWidget *parent, const QString &title, const QString &message);
NOTESHARED_EXPORT QString createToolTip(const Akonadi::Item &item);
}
}

#endif // NOTEUTILS_H
