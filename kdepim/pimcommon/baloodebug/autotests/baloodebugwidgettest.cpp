/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "baloodebugwidgettest.h"
#include "pimcommon/texteditor/plaintexteditor/plaintexteditorwidget.h"
#include "../baloodebugwidget.h"
#include "../baloodebugsearchpathcombobox.h"
#include <qtest_kde.h>
#include <KLineEdit>
#include <QPushButton>

BalooDebugWidgetTest::BalooDebugWidgetTest(QObject *parent)
    : QObject(parent)
{

}

BalooDebugWidgetTest::~BalooDebugWidgetTest()
{

}

void BalooDebugWidgetTest::shouldHaveDefaultValue()
{
    PimCommon::BalooDebugWidget widget;
    QPushButton *button = qFindChild<QPushButton *>(&widget, QLatin1String("searchbutton"));
    QVERIFY(button);
    QVERIFY(!button->isEnabled());
    KLineEdit *lineEdit = qFindChild<KLineEdit *>(&widget, QLatin1String("lineedit"));
    QVERIFY(lineEdit);
    QVERIFY(lineEdit->text().isEmpty());
    QVERIFY(lineEdit->trapReturnKey());
    QVERIFY(lineEdit->isClearButtonShown());
    PimCommon::PlainTextEditorWidget *editorWidget = qFindChild<PimCommon::PlainTextEditorWidget *>(&widget, QLatin1String("plaintexteditor"));
    QVERIFY(editorWidget->isReadOnly());
    QVERIFY(editorWidget);
    QVERIFY(editorWidget->toPlainText().isEmpty());
    PimCommon::BalooDebugSearchPathComboBox *searchCombo = qFindChild<PimCommon::BalooDebugSearchPathComboBox *>(&widget, QLatin1String("searchpathcombo"));
    QVERIFY(searchCombo);
}

void BalooDebugWidgetTest::shouldFillLineEditWhenWeWantToSearchItem()
{
    PimCommon::BalooDebugWidget widget;
    KLineEdit *lineEdit = qFindChild<KLineEdit *>(&widget, QLatin1String("lineedit"));
    const int value = 42;
    const QString akonadiItem = QString::number(value);
    widget.setAkonadiId(value);
    QCOMPARE(lineEdit->text(), akonadiItem);
}

void BalooDebugWidgetTest::shouldEnabledPushButtonWhenLineEditIsNotEmpty()
{
    PimCommon::BalooDebugWidget widget;
    const int value = 42;
    widget.setAkonadiId(value);
    QPushButton *button = qFindChild<QPushButton *>(&widget, QLatin1String("searchbutton"));
    QVERIFY(button->isEnabled());

    KLineEdit *lineEdit = qFindChild<KLineEdit *>(&widget, QLatin1String("lineedit"));
    lineEdit->setText(QLatin1String(""));
    QVERIFY(!button->isEnabled());

    //trimmed string
    lineEdit->setText(QLatin1String(" "));
    QVERIFY(!button->isEnabled());

}

void BalooDebugWidgetTest::shouldChangeSearchType()
{
    PimCommon::BalooDebugWidget widget;
    PimCommon::BalooDebugSearchPathComboBox::SearchType type = PimCommon::BalooDebugSearchPathComboBox::Emails;
    widget.setSearchType(type);
    PimCommon::BalooDebugSearchPathComboBox *searchCombo = qFindChild<PimCommon::BalooDebugSearchPathComboBox *>(&widget, QLatin1String("searchpathcombo"));
    const QString path = searchCombo->pathFromEnum(type);
    QCOMPARE(searchCombo->searchPath(), path);


}

QTEST_KDEMAIN(BalooDebugWidgetTest, GUI)
