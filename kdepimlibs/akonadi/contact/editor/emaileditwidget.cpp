/*
    This file is part of Akonadi Contact.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "emaileditwidget.h"

#include "autoqpointer_p.h"

#include <QtCore/QEvent>
#include <QtCore/QString>
#include <QGridLayout>
#include <QPushButton>
#include <QToolButton>

#include <kabc/addressee.h>
#include <kacceleratormanager.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <KListWidget>
#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <kpimutils/email.h>

class EmailAddressExtracter : public QObject
{
public:
    EmailAddressExtracter(KLineEdit *lineEdit)
        : QObject(lineEdit)
        , mLineEdit(lineEdit)
    {
        lineEdit->installEventFilter(this);
    }

    virtual bool eventFilter(QObject *watched, QEvent *event)
    {
        if (watched == mLineEdit && event->type() == QEvent::FocusOut) {
            const QString fullEmailAddress = mLineEdit->text();
            const QString extractedEmailAddress = KPIMUtils::extractEmailAddress(fullEmailAddress);
            mLineEdit->setText(extractedEmailAddress);
        }

        return QObject::eventFilter(watched, event);
    }

private:
    KLineEdit *mLineEdit;
};

class EmailItem : public QListWidgetItem
{
public:
    EmailItem(const QString &text, QListWidget *parent, bool preferred)
        : QListWidgetItem(text, parent)
        , mPreferred(preferred)
    {
        format();
    }

    void setPreferred(bool preferred)
    {
        mPreferred = preferred;
        format();
    }
    bool preferred() const
    {
        return mPreferred;
    }

private:
    void format()
    {
        QFont f = font();
        f.setBold(mPreferred);
        setFont(f);
    }

private:
    bool mPreferred;
};

EmailEditWidget::EmailEditWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());

    mEmailEdit = new KLineEdit;
    new EmailAddressExtracter(mEmailEdit);
    connect(mEmailEdit, SIGNAL(textChanged(QString)),
            SLOT(textChanged(QString)));
    layout->addWidget(mEmailEdit);

    mEditButton = new QToolButton;
    mEditButton->setText(QLatin1String("..."));
    connect(mEditButton, SIGNAL(clicked()), SLOT(edit()));
    layout->addWidget(mEditButton);
    setFocusProxy(mEditButton);
    setFocusPolicy(Qt::StrongFocus);
}

EmailEditWidget::~EmailEditWidget()
{
}

void EmailEditWidget::setReadOnly(bool readOnly)
{
    mEmailEdit->setReadOnly(readOnly);
    mEditButton->setEnabled(!readOnly);
}

void EmailEditWidget::loadContact(const KABC::Addressee &contact)
{
    mEmailList = contact.emails();

    if (!mEmailList.isEmpty()) {
        mEmailEdit->setText(mEmailList.first());
    } else {
        mEmailEdit->setText(QString());
    }
}

void EmailEditWidget::storeContact(KABC::Addressee &contact) const
{
    QStringList emails(mEmailList);

    // the preferred address is always the first one, remove it...
    if (!emails.isEmpty()) {
        emails.removeFirst();
    }

    // ... and prepend the one from the line edit
    if (!mEmailEdit->text().isEmpty()) {
        emails.prepend(mEmailEdit->text().toLower());
    }

    contact.setEmails(emails);
}

void EmailEditWidget::edit()
{
    AutoQPointer<EmailEditDialog> dlg = new EmailEditDialog(mEmailList, this);

    if (dlg->exec()) {
        if (dlg->changed()) {
            mEmailList = dlg->emails();
            if (!mEmailList.isEmpty()) {
                mEmailEdit->setText(mEmailList.first());
            } else {
                mEmailEdit->setText(QString());
            }
        }
    }
}

void EmailEditWidget::textChanged(const QString &text)
{
    if (!mEmailList.isEmpty()) {
        mEmailList.removeFirst();
    }

    mEmailList.prepend(text);
}

EmailEditDialog::EmailEditDialog(const QStringList &list, QWidget *parent)
    : KDialog(parent)
{
    setCaption(i18n("Edit Email Addresses"));
    setButtons(KDialog::Ok | KDialog::Cancel);
    setDefaultButton(KDialog::Cancel);

    QWidget *page = new QWidget(this);
    setMainWidget(page);

    QGridLayout *topLayout = new QGridLayout(page);
    topLayout->setSpacing(spacingHint());
    topLayout->setMargin(0);

    mEmailListBox = new KListWidget(page);
    mEmailListBox->setSelectionMode(QAbstractItemView::SingleSelection);

    // Make sure there is room for the scrollbar
    mEmailListBox->setMinimumHeight(mEmailListBox->sizeHint().height() + 30);
    connect(mEmailListBox, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            SLOT(selectionChanged()));
    connect(mEmailListBox, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            SLOT(edit()));
    topLayout->addWidget(mEmailListBox, 0, 0, 5, 2);

    mAddButton = new QPushButton(i18n("Add..."), page);
    connect(mAddButton, SIGNAL(clicked()), SLOT(add()));
    topLayout->addWidget(mAddButton, 0, 2);

    mEditButton = new QPushButton(i18n("Edit..."), page);
    mEditButton->setEnabled(false);
    connect(mEditButton, SIGNAL(clicked()), SLOT(edit()));
    topLayout->addWidget(mEditButton, 1, 2);

    mRemoveButton = new QPushButton(i18n("Remove"), page);
    mRemoveButton->setEnabled(false);
    connect(mRemoveButton, SIGNAL(clicked()), SLOT(remove()));
    topLayout->addWidget(mRemoveButton, 2, 2);

    mStandardButton = new QPushButton(i18n("Set as Standard"), page);
    mStandardButton->setEnabled(false);
    connect(mStandardButton, SIGNAL(clicked()), SLOT(standard()));
    topLayout->addWidget(mStandardButton, 3, 2);

    topLayout->setRowStretch(4, 1);

    QStringList items = list;
    if (items.removeAll(QLatin1String("")) > 0) {
        mChanged = true;
    } else {
        mChanged = false;
    }

    QStringList::ConstIterator it;
    bool preferred = true;
    QStringList::ConstIterator end(items.constEnd());
    for (it = items.constBegin(); it != end; ++it) {
        new EmailItem(*it, mEmailListBox, preferred);
        preferred = false;
    }

    // set default state
    KAcceleratorManager::manage(this);

    readConfig();
}

EmailEditDialog::~EmailEditDialog()
{
    writeConfig();
}

void EmailEditDialog::readConfig()
{
    KConfigGroup group(KGlobal::config(), "EmailEditDialog");
    const QSize sizeDialog = group.readEntry("Size", QSize(400, 200));
    if (sizeDialog.isValid()) {
        resize(sizeDialog);
    }
}

void EmailEditDialog::writeConfig()
{
    KConfigGroup group(KGlobal::config(), "EmailEditDialog");
    group.writeEntry("Size", size());
}

QStringList EmailEditDialog::emails() const
{
    QStringList emails;

    for (int i = 0; i < mEmailListBox->count(); ++i) {
        EmailItem *item = static_cast<EmailItem *>(mEmailListBox->item(i));
        if (item->preferred()) {
            emails.prepend(item->text());
        } else {
            emails.append(item->text());
        }
    }

    return emails;
}

void EmailEditDialog::add()
{
    bool ok = false;

    QString email = KInputDialog::getText(i18n("Add Email"), i18n("New Email:"),
                                          QString(), &ok, this);

    if (!ok) {
        return;
    }

    email = KPIMUtils::extractEmailAddress(email.toLower());

    // check if item already available, ignore if so...
    for (int i = 0; i < mEmailListBox->count(); ++i) {
        if (mEmailListBox->item(i)->text() == email) {
            return;
        }
    }

    new EmailItem(email, mEmailListBox, (mEmailListBox->count() == 0));

    mChanged = true;
}

void EmailEditDialog::edit()
{
    bool ok = false;

    QListWidgetItem *item = mEmailListBox->currentItem();

    QString email = KInputDialog::getText(i18n("Edit Email"),
                                          i18nc("@label:textbox Inputfield for an email address", "Email:"),
                                          item->text(), &ok, this);

    if (!ok) {
        return;
    }

    email = KPIMUtils::extractEmailAddress(email.toLower());

    // check if item already available, ignore if so...
    for (int i = 0; i < mEmailListBox->count(); ++i) {
        if (mEmailListBox->item(i)->text() == email) {
            return;
        }
    }

    EmailItem *eitem = static_cast<EmailItem *>(item);
    eitem->setText(email);

    mChanged = true;
}

void EmailEditDialog::remove()
{
    const QString address = mEmailListBox->currentItem()->text();

    const QString text = i18n("<qt>Are you sure that you want to remove the email address <b>%1</b>?</qt>", address);
    const QString caption = i18n("Confirm Remove");

    if (KMessageBox::warningContinueCancel(this, text, caption, KGuiItem(i18n("&Delete"), QLatin1String("edit-delete"))) == KMessageBox::Continue) {
        EmailItem *item = static_cast<EmailItem *>(mEmailListBox->currentItem());

        const bool preferred = item->preferred();
        mEmailListBox->takeItem(mEmailListBox->currentRow());
        if (preferred) {
            item = dynamic_cast<EmailItem *>(mEmailListBox->item(0));
            if (item) {
                item->setPreferred(true);
            }
        }

        mChanged = true;
    }
}

bool EmailEditDialog::changed() const
{
    return mChanged;
}

void EmailEditDialog::standard()
{
    for (int i = 0; i < mEmailListBox->count(); ++i) {
        EmailItem *item = static_cast<EmailItem *>(mEmailListBox->item(i));
        if (i == mEmailListBox->currentRow()) {
            item->setPreferred(true);
        } else {
            item->setPreferred(false);
        }
    }

    mChanged = true;
}

void EmailEditDialog::selectionChanged()
{
    int index = mEmailListBox->currentRow();
    bool value = (index >= 0);   // An item is selected

    mRemoveButton->setEnabled(value);
    mEditButton->setEnabled(value);
    mStandardButton->setEnabled(value);
}
