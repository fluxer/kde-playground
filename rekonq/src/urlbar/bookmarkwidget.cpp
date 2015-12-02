/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2010-2011 by Yoann Laissus <yoann dot laissus at gmail dot com>
* Copyright (C) 2012 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (c) 2011-2012 by Phaneendra Hegde <pnh.pes@gmail.com>
*
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy
* defined in Section 14 of version 3 of the license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
* ============================================================ */


// Self Includes
#include "bookmarkwidget.h"
#include "moc_bookmarkwidget.cpp"

// Local includes
#include "bookmarkmanager.h"
#include "bookmarkowner.h"

// KDE Includes
#include <KComboBox>
#include <KLocalizedString>
#include <KIcon>
#include <KLineEdit>
#include <KRatingWidget>

// Qt Includes
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QCompleter>
#include <QTextCursor>
#include <QDBusConnection>
#include <QDBusConnectionInterface>


BookmarkWidget::BookmarkWidget(const KBookmark &bookmark, QWidget *parent)
    : QMenu(parent)
    , m_bookmark(new KBookmark(bookmark))
    , m_tagLine(new KLineEdit(this))
    , m_commentEdit(new QPlainTextEdit(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedWidth(320);

    QFormLayout *layout = new QFormLayout(this);
    layout->setHorizontalSpacing(20);

    // Title
    QHBoxLayout *hLayout = new QHBoxLayout;
    QLabel *bookmarkInfo = new QLabel(this);
    bookmarkInfo->setText(i18n(" Bookmark"));
    QFont f = bookmarkInfo->font();
    f.setBold(true);
    bookmarkInfo->setFont(f);

    // Remove button
    QLabel *removeLabel = new QLabel(this);
    removeLabel->setText(i18n("<a href='Remove'>Remove</a>"));
    removeLabel->setAlignment(Qt::AlignRight);
    hLayout->addWidget(bookmarkInfo);
    hLayout->addWidget(removeLabel);
    layout->addRow(hLayout);

    connect(removeLabel, SIGNAL(linkActivated(QString)), this, SLOT(removeBookmark()));

    //Bookmark Folder
    QLabel *folderLabel = new QLabel(this);
    folderLabel->setText(i18n("Folder:"));

    m_folder = new KComboBox(this);
    layout->addRow(folderLabel, m_folder);
    setupFolderComboBox();

    // Bookmark name
    QLabel *nameLabel = new QLabel(this);
    nameLabel->setText(i18n("Name:"));
    m_name = new KLineEdit(this);
    if (m_bookmark->isNull())
    {
        m_name->setEnabled(false);
    }
    else
    {
        m_name->setText(m_bookmark->text());
        m_name->setFocus();
    }
    layout->addRow(nameLabel, m_name);

    // Ok & Cancel buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    layout->addWidget(buttonBox);
}


BookmarkWidget::~BookmarkWidget()
{
    delete m_bookmark;
}


void BookmarkWidget::showAt(const QPoint &pos)
{
    adjustSize();

    QPoint p(pos.x() - width(), pos.y() + 10);
    move(p);
    show();
}


void BookmarkWidget::accept()
{
    if (!m_bookmark->isNull() && m_name->text() != m_bookmark->fullText())
    {
        m_bookmark->setFullText(m_name->text());
        BookmarkManager::self()->emitChanged();
    }
    QString folderAddress = m_folder->itemData(m_folder->currentIndex()).toString();
    KBookmarkGroup a = BookmarkManager::self()->manager()->findByAddress(folderAddress).toGroup();

    KBookmarkGroup parent = m_bookmark->parentGroup();
    parent.deleteBookmark(*m_bookmark);
    a.addBookmark(*m_bookmark);
    BookmarkManager::self()->manager()->emitChanged(a);

    close();
}


void BookmarkWidget::setupFolderComboBox()
{
    KBookmarkGroup toolBarRoot = BookmarkManager::self()->manager()->toolbar();
    KBookmarkGroup root = BookmarkManager::self()->rootGroup();

    if (toolBarRoot.address() == root.address())
    {
        m_folder->addItem(KIcon("bookmark-toolbar"),
                          i18n("Bookmark Toolbar"),
                          toolBarRoot.address());
    }
    else
    {
        m_folder->addItem(KIcon("bookmark-toolbar"),
                          toolBarRoot.text(),
                          toolBarRoot.address());
    }
    m_folder->insertSeparator(1);

    if (m_bookmark->parentGroup().address() != toolBarRoot.address())
    {
        QString parentText = m_bookmark->parentGroup().text();

        if (m_bookmark->parentGroup().address() == root.address())
        {
            parentText = i18n("Root folder");
        }

        m_folder->addItem(parentText,
                          m_bookmark->parentGroup().address());
        m_folder->insertSeparator(3);
    }

    for (KBookmark bookmark = toolBarRoot.first(); !bookmark.isNull(); bookmark = toolBarRoot.next(bookmark))
    {
        if (bookmark.isGroup() && bookmark.address() != m_bookmark->parentGroup().address())
        {
            m_folder->addItem(bookmark.text(), bookmark.address());
        }
    }

    int index =  m_folder->findData(m_bookmark->parentGroup().address());
    m_folder->setCurrentIndex(index);
}


void BookmarkWidget::removeBookmark()
{
    BookmarkManager::self()->owner()->deleteBookmark(*m_bookmark);
    close();

    emit updateIcon();
}

