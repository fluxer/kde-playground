/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2011, Dmitry Chernov diman4ik.chernov@gmail.com
 */

#include "main_window.h"

#include <KAction>
#include <KStandardAction>
#include <KApplication>
#include <KActionCollection>
#include <klocale.h>
#include <KStatusBar>
#include <KFileDialog>
#include <KWindowSystem>
#include <QActionGroup>

#include <KDebug>

#include "picture_widget.h"
#include "image_sorter.h"
#include "settings_dialog.h"


MainWindow::MainWindow(QWidget *parent) : KXmlGuiWindow(parent), m_settingsDialog(0), m_canAct(false)
{
    setAttribute( Qt::WA_DeleteOnClose );

    setMinimumSize( 480, 320 );
    
    m_picWidget = new PictureWidget(this);
    
    setCentralWidget( m_picWidget );
    
    connect( m_picWidget, SIGNAL( changed() ), SLOT( changed() ) );
    connect( m_picWidget, SIGNAL( cantChange() ), SLOT( cantChange() ) );
    
    statusBar()->setSizeGripEnabled(true);
    
    m_imSorter = new ImageSorter(this);
    
    connect( m_imSorter, SIGNAL( noPrevious() ), SLOT( noPrevious() ) );
    connect( m_imSorter, SIGNAL( noNext() ), SLOT( noNext() ) );
    connect( m_imSorter, SIGNAL( gotPrevious() ), SLOT( gotPrevious() ) );
    connect( m_imSorter, SIGNAL( gotNext() ), SLOT( gotNext() ) );
    connect( m_imSorter, SIGNAL( addNewAction( int ) ), SLOT( addNewAction( int ) ) );
    connect( m_imSorter, SIGNAL( changeShortcuts() ), SLOT( setCustomActions() ) );
    
    setupActions();
}

void MainWindow::openFile(KUrl url)
{
    if( !url.isEmpty() )
    {
        m_canAct = false;
        statusBar()->showMessage( "loading ... " );
        m_curFileName = url.path();
        m_picWidget->setImage( m_curFileName );
    }

    m_imSorter->setCurrentFile( url.path() );
}

void MainWindow::setupActions()
{
    KAction* prevAction = new KAction(this);
    prevAction->setText( i18n("Prev") );
    prevAction->setIcon( KIcon("media-skip-backward") );
    prevAction->setShortcut( Qt::Key_Backspace );
    prevAction->setDisabled(true);
    actionCollection()->addAction( "previous", prevAction );
    
    connect( prevAction, SIGNAL( triggered(bool) ), SLOT( prevFile() ) );
    
    KAction* nextAction = new KAction(this);
    nextAction->setText( i18n("Next") );
    nextAction->setIcon( KIcon("media-skip-forward") );
    nextAction->setShortcut( Qt::Key_Space );
    nextAction->setDisabled(true);
    actionCollection()->addAction( "next", nextAction );
    
    connect( nextAction, SIGNAL( triggered(bool) ), SLOT( nextFile() ) );
    
    KAction* configure = new KAction(this);
    configure->setText( i18n("Sort settings") );
    actionCollection()->addAction( "configure", configure );
    
    connect( configure, SIGNAL( triggered(bool) ), SLOT( configureSort() ) );
    
    KStandardAction::quit( kapp, SLOT( quit() ), actionCollection() );
 
    KStandardAction::open( this, SLOT( openFile() ), actionCollection() );
    
    m_sortActions = new QActionGroup(this);
    connect( m_sortActions, SIGNAL( triggered( QAction* ) ), SLOT( actionTriggered( QAction* ) ) );
    
    setCustomActions();
 
    setupGUI();
}

void MainWindow::openFile()
{
    QString fileNameFromDialog = KFileDialog::getOpenFileName();
    
    if( !fileNameFromDialog.isEmpty() )
    {
        m_canAct = false;
        statusBar()->showMessage( "loading ... " );
        m_curFileName = fileNameFromDialog;
        m_picWidget->setImage( m_curFileName );
    }
    
    m_imSorter->setCurrentFile( fileNameFromDialog );
}

void MainWindow::nextFile()
{
    QString nextFile = m_imSorter->getNextFileName();
    
    m_canAct = false;
    statusBar()->showMessage( "loading ... " );
    m_curFileName = nextFile;
    m_picWidget->setImage( m_curFileName );
}

void MainWindow::prevFile()
{
    QString prevFile = m_imSorter->getPrevFileName();
    
    m_canAct = false;
    statusBar()->showMessage( "loading ... " );
    m_curFileName = prevFile;
    m_picWidget->setImage( m_curFileName );
}

void MainWindow::noNext()
{
    actionCollection()->action("next")->setEnabled(false);
}

void MainWindow::noPrevious()
{
    actionCollection()->action("previous")->setEnabled(false);
}

void MainWindow::changed()
{
    if( !m_canAct )
        m_canAct = true;
        
    statusBar()->showMessage( m_curFileName );
}

void MainWindow::cantChange()
{
    statusBar()->showMessage( "can't load " + m_curFileName );
}

void MainWindow::gotNext()
{
    actionCollection()->action("next")->setEnabled(true);    
}

void MainWindow::gotPrevious()
{
    actionCollection()->action("previous")->setEnabled(true);
}

void MainWindow::configureSort()
{
    if( !m_settingsDialog )
    {
        m_settingsDialog = new SortSettingsDialog(  m_imSorter->getMainComb(), m_imSorter->getBindings(), this );

        connect( m_settingsDialog, SIGNAL( bindingChangedSignal( int, const binding& ) ),
                 m_imSorter, SLOT( settingChanged( int, const binding& ) ) );
        
        connect( m_settingsDialog, SIGNAL( mainCombChanged( QString ) ), m_imSorter, SLOT( mainCombChanged( QString ) ) );
        
        connect( m_settingsDialog, SIGNAL( saveSettings() ), m_imSorter, SLOT( saveSettings() ) );
        
        connect( m_settingsDialog, SIGNAL( bindingRemoved( int ) ), m_imSorter, SLOT( bindingRemoved( int ) ) );
        
        connect( m_imSorter, SIGNAL( illegalBinding( int, const binding&, const binding& ) ), m_settingsDialog, 
                 SLOT( illegalBinding( int, const binding&, const binding& ) ) );
        
        m_settingsDialog->show();
    }
    else
    {
        if( m_settingsDialog->isMinimized() )
            KWindowSystem::unminimizeWindow( m_settingsDialog->winId() );
        
        KWindowSystem::activateWindow( m_settingsDialog->winId() );
        
        m_settingsDialog->show();
    }
}

void MainWindow::addNewAction( int row )
{
    QString comb = m_imSorter->getBindSeq(row);
    
    KAction* action = new KAction( this );
    action->setShortcut( QKeySequence( comb ) );
    action->setActionGroup( m_sortActions );
    action->setData(row);
    actionCollection()->addAction( comb, action );
}

void MainWindow::actionTriggered( QAction* act )
{
    kDebug() << "action triggered " << act->shortcut().toString();
    
    if( m_canAct )
    {
        if( m_imSorter->performSort( act->data().toInt() ) )
            nextFile();
        else
            statusBar()->showMessage( "can't move " + m_curFileName + " to " 
                                      + m_imSorter->getBindings().at( act->data().toInt() ).path );
    }
}

void MainWindow::setCustomActions()
{
    for( int i = 0; i < m_imSorter->getNoBindings(); ++i )
    {
        QString comb = m_imSorter->getBindSeq(i);
        KAction* action = new KAction(this);
        action->setShortcut( QKeySequence( comb ) );
        action->setActionGroup( m_sortActions );
        action->setData(i);
        actionCollection()->addAction( comb, action );
    }
}
