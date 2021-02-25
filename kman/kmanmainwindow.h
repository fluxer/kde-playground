/*  This file is part of KMan
    Copyright (C) 2018 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KMANWINDOW_H
#define KMANWINDOW_H

#include <KXmlGuiWindow>
#include <KAction>

QT_BEGIN_NAMESPACE
class Ui_KManWindow;
QT_END_NAMESPACE

class KManLister;

class KManMainWindow: public KXmlGuiWindow
{
    Q_OBJECT
public:
    KManMainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~KManMainWindow();

    void changePath(const QString path);

public slots:
    void slotOpenAction();
    void slotQuitAction();
    void slotNextAction();
    void slotPreviousAction();
    void slotFindAction();
    void slotFindNextAction();
    void slotFindPreviousAction();
    void slotSearchChanged(const QString &text);
    void slotListChanged(const QString &text);

private slots:
    void slotListResult(const QString path, const QString fancy);
    void slotBusyStart();
    void slotBusyFinish();

private:
    void listManPages();
    QString manContent(const QString path);

private:
    Ui_KManWindow *m_kmanui;

    KAction *m_actionopen;
    KAction *m_actionquit;
    KAction *m_actionnext;
    KAction *m_actionprevious;
    KAction *m_actionfind;
    KAction *m_actionfindnext;
    KAction *m_actionfindprevious;

    QString m_path;
    QString m_search;
    KManLister *m_lister;
};

#endif // KMANWINDOW_H