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

#include "settings_dialog.h"

#include <QDialog>
#include <KKeySequenceWidget>
#include <KMessageBox>

#include <KDebug>


SortSettingsDialog::SortSettingsDialog( const QString& mainComb,
                                        QList<binding>& bindings, QWidget* parent ) : QDialog( parent )
{
    m_ui.setupUi(this);
    
    int nbindings = bindings.size();
    
    m_ui.tableWidget->setRowCount(nbindings);
    
    for( int i = 0; i < nbindings; ++i )
    {
        QTableWidgetItem *keyItem = new QTableWidgetItem( bindings[i].key );
        m_ui.tableWidget->setItem( i, 0, keyItem );
        QTableWidgetItem *pathItem = new QTableWidgetItem( bindings[i].path );
        m_ui.tableWidget->setItem( i, 1, pathItem );
    }
    
    m_ui.tableWidget->resizeColumnsToContents();
    m_ui.tableWidget->horizontalHeader()->stretchLastSection();
    
    connect( m_ui.tableWidget, SIGNAL( cellChanged( int, int ) ), SLOT( bindingChanged( int, int ) ) );
    connect( m_ui.kkeysequencewidget, SIGNAL( keySequenceChanged( const QKeySequence& ) ), 
             SLOT( mainSeqChanged( const QKeySequence& ) ) );
    
    m_ui.kkeysequencewidget->setKeySequence( mainComb );
    
    m_ui.pushButton->setIcon( KIcon("list-add") );
    m_ui.pushButton_2->setIcon( KIcon("list-remove") );
    
    connect( m_ui.pushButton, SIGNAL( clicked(bool) ), SLOT( plusClicked(bool) ) );
    connect( m_ui.pushButton_2, SIGNAL( clicked(bool) ), SLOT( minusClicked(bool) ) );
}

void SortSettingsDialog::accept()
{
    emit( saveSettings() );
    done(0);
}

void SortSettingsDialog::reject()
{
    done(1);
}

void SortSettingsDialog::bindingChanged( int row, int )
{
    bool needAdd =  ( m_ui.tableWidget->item( row, 0 )->text().compare( "." ) != 0 ) &&
                    ( m_ui.tableWidget->item( row, 1 )->text().compare( "..." ) != 0 );
    
    if( needAdd )
    {
        kDebug() << "needs add " << m_ui.tableWidget->item( row, 0 )->text().at(0) << " " <<
                                    m_ui.tableWidget->item( row, 1 )->text();
        
        binding bin = {     m_ui.tableWidget->item( row, 0 )->text().at(0), 
                            m_ui.tableWidget->item( row, 1 )->text() };
        
        emit( bindingChangedSignal( row, bin ) );
    }
}

void SortSettingsDialog::mainSeqChanged( const QKeySequence& seq )
{
    emit( mainCombChanged( seq.toString() ) );
}

void SortSettingsDialog::plusClicked( bool )
{
    m_ui.tableWidget->insertRow(  m_ui.tableWidget->rowCount() );
    
    m_ui.tableWidget->setItem( m_ui.tableWidget->rowCount() - 1, 0, new QTableWidgetItem( "." ) );
    m_ui.tableWidget->setItem( m_ui.tableWidget->rowCount() - 1, 1, new QTableWidgetItem( "..." ) );
    m_ui.tableWidget->scrollToItem( m_ui.tableWidget->item( m_ui.tableWidget->rowCount() - 1, 0 ) );
}

void SortSettingsDialog::minusClicked( bool )
{
    int selectedRow = m_ui.tableWidget->currentRow();
    
    int res = KMessageBox::questionYesNo(    this, "Are you sure you want to delete binding " + 
                                            m_ui.tableWidget->item( selectedRow, 0 )->text() + " "
                                            + m_ui.tableWidget->item( selectedRow, 1 )->text() + " ?" );
                                            
    if(  res == KMessageBox::Yes )
    {
        m_ui.tableWidget->removeRow( selectedRow );
        emit( bindingRemoved( selectedRow ) );
    }
}

void SortSettingsDialog::illegalBinding( int row, const binding& , const binding& prev )
{
    m_ui.tableWidget->item( row, 0 )->setText( prev.key );
    m_ui.tableWidget->item( row, 1 )->setText( prev.path );
}
