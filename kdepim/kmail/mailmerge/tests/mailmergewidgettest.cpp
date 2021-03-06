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

#include "mailmergewidgettest.h"
#include "../mailmergewidget.h"
#include "pimcommon/widgets/simplestringlisteditor.h"
#include <qtest_kde.h>
#include <KComboBox>
#include <QStackedWidget>

Q_DECLARE_METATYPE(MailMergeWidget::SourceType)
MailMergeWidgetTest::MailMergeWidgetTest()
{
    qRegisterMetaType<MailMergeWidget::SourceType>();
}

void MailMergeWidgetTest::shouldHaveDefaultValueOnCreation()
{
    MailMergeWidget mailmerge;
    KComboBox *source = qFindChild<KComboBox *>(&mailmerge, QLatin1String("source"));
    QVERIFY(source);
    QCOMPARE(source->currentIndex(), 0);

    QStackedWidget *stackedwidget = qFindChild<QStackedWidget *>(&mailmerge, QLatin1String("stackedwidget"));
    QVERIFY(stackedwidget);
    QCOMPARE(stackedwidget->count(), 2);
    QCOMPARE(stackedwidget->currentIndex(), 0);

    PimCommon::SimpleStringListEditor *listEditor = qFindChild<PimCommon::SimpleStringListEditor *>(&mailmerge, QLatin1String("attachment-list"));
    QVERIFY(listEditor);
    QCOMPARE(listEditor->stringList().count(), 0);
}

void MailMergeWidgetTest::shouldEmitSourceModeChanged()
{
    MailMergeWidget mailmerge;
    KComboBox *source = qFindChild<KComboBox *>(&mailmerge, QLatin1String("source"));
    QCOMPARE(source->currentIndex(), 0);
    QSignalSpy spy(&mailmerge, SIGNAL(sourceModeChanged(MailMergeWidget::SourceType)));
    source->setCurrentIndex(1);
    QCOMPARE(spy.count(), 1);
}

void MailMergeWidgetTest::shouldDontEmitSourceModeChangedWhenIndexIsInvalid()
{
    MailMergeWidget mailmerge;
    KComboBox *source = qFindChild<KComboBox *>(&mailmerge, QLatin1String("source"));
    QCOMPARE(source->currentIndex(), 0);
    QSignalSpy spy(&mailmerge, SIGNAL(sourceModeChanged(MailMergeWidget::SourceType)));
    source->setCurrentIndex(-1);
    QCOMPARE(spy.count(), 0);
}

void MailMergeWidgetTest::shouldChangeStackedWidgetIndexWhenChangeComboboxIndex()
{
    MailMergeWidget mailmerge;
    KComboBox *source = qFindChild<KComboBox *>(&mailmerge, QLatin1String("source"));
    QCOMPARE(source->currentIndex(), 0);

    QStackedWidget *stackedwidget = qFindChild<QStackedWidget *>(&mailmerge, QLatin1String("stackedwidget"));
    QCOMPARE(stackedwidget->currentIndex(), 0);
    source->setCurrentIndex(0);
    QCOMPARE(stackedwidget->currentIndex(), 0);
    source->setCurrentIndex(1);
    QCOMPARE(stackedwidget->currentIndex(), 1);
}


QTEST_KDEMAIN( MailMergeWidgetTest, GUI )
