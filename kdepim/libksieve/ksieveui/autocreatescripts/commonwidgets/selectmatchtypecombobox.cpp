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

#include "selectmatchtypecombobox.h"
#include "autocreatescripts/sieveeditorgraphicalmodewidget.h"
#include "autocreatescripts/autocreatescriptutil_p.h"

#include <KLocalizedString>

using namespace KSieveUi;

SelectMatchTypeComboBox::SelectMatchTypeComboBox(QWidget *parent)
    : KComboBox(parent)
{
    mHasRegexCapability = SieveEditorGraphicalModeWidget::sieveCapabilities().contains(QLatin1String("regex"));
    initialize();
    connect(this, SIGNAL(activated(int)), this, SIGNAL(valueChanged()));
}

SelectMatchTypeComboBox::~SelectMatchTypeComboBox()
{
}

void SelectMatchTypeComboBox::initialize()
{
    addItem(i18n("is"), QLatin1String(":is"));
    addItem(i18n("not is"), QLatin1String("[NOT]:is"));
    addItem(i18n("contains"), QLatin1String(":contains"));
    addItem(i18n("not contains"), QLatin1String("[NOT]:contains"));
    addItem(i18n("matches"), QLatin1String(":matches"));
    addItem(i18n("not matches"), QLatin1String("[NOT]:matches"));
    if (mHasRegexCapability) {
        addItem(i18n("regex"), QLatin1String(":regex"));
        addItem(i18n("not regex"), QLatin1String("[NOT]:regex"));
    }
}

QString SelectMatchTypeComboBox::code(bool &negative) const
{
    QString value = itemData(currentIndex()).toString();
    negative = value.startsWith(QLatin1String("[NOT]"));
    if (negative)
        value = value.remove(QLatin1String("[NOT]"));
    return value;
}

void SelectMatchTypeComboBox::setCode(const QString &code, const QString &name, QString &error)
{
    const int index = findData(code);
    if (index != -1) {
        setCurrentIndex(index);
    } else {
        AutoCreateScriptUtil::comboboxItemNotFound(code, name, error);
        setCurrentIndex(0);
    }
}


