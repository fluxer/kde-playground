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

#ifndef PLAINTEXTEDITOR_H
#define PLAINTEXTEDITOR_H

#include "pimcommon_export.h"

#include <QPlainTextEdit>

namespace PimCommon {
class PIMCOMMON_EXPORT PlainTextEditor : public QPlainTextEdit
{
    Q_OBJECT
    Q_PROPERTY(bool searchSupport READ searchSupport WRITE setSearchSupport)
public:
    explicit PlainTextEditor(QWidget *parent=0);
    ~PlainTextEditor();

    void setSearchSupport(bool b);
    bool searchSupport() const;

    bool spellCheckingSupport() const;
    void setSpellCheckingSupport( bool check );

    virtual void setReadOnly( bool readOnly );

public slots:
    void slotDisplayMessageIndicator(const QString &message);

private Q_SLOTS:
    void slotUndoableClear();
    void slotSpeakText();
    void slotCheckSpelling();
    void slotSpellCheckerMisspelling( const QString &text, int pos );
    void slotSpellCheckerCorrected( const QString &, int,const QString &);
    void slotSpellCheckerAutoCorrect(const QString&, const QString&);
    void slotSpellCheckerCanceled();
    void slotSpellCheckerFinished();

protected:
    virtual void addExtraMenuEntry(QMenu *menu, const QPoint &pos);

protected:
    void contextMenuEvent( QContextMenuEvent *event );
    void wheelEvent( QWheelEvent *event );
    bool event(QEvent* ev);    
    void keyPressEvent(QKeyEvent *event);
    bool overrideShortcut(const QKeyEvent* event);
    bool handleShortcut(const QKeyEvent *event);

Q_SIGNALS:
    void findText();
    void replaceText();
    void spellCheckerAutoCorrect(const QString& currentWord, const QString& autoCorrectWord);
    void checkSpellingChanged(bool);
    void languageChanged(const QString &);
    void spellCheckStatus(const QString &);

private:
    void deleteWordBack();
    void deleteWordForward();
    void highlightWord( int length, int pos );
    class PlainTextEditorPrivate;
    PlainTextEditorPrivate *const d;
};
}
#endif // PLAINTEXTEDITOR_H
