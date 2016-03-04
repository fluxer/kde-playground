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

#include "image_sorter.h"

#include <KConfig>
#include <KConfigGroup>
#include <KDebug>

#include <kio/copyjob.h>


ImageSorter::ImageSorter( QObject* parent ) : QObject(parent), m_curFileIndex(0), m_nextFileIndex(1)
{
    loadSettings();
}

void ImageSorter::setCurrentFile( const QString& name )
{
    QFileInfo fi( name );
    m_curDirectory.setPath( fi.path() );
    
    m_dirFiles = m_curDirectory.entryInfoList( QDir::Files );
    
    m_curFileIndex = m_dirFiles.indexOf(fi);
    
    if( m_curFileIndex > 0 )
        emit( gotPrevious() );
    else
        emit( noPrevious() );
    
    if( m_curFileIndex < m_dirFiles.length() - 1 )
        emit( gotNext() );
    else
        emit( noNext() );
}

QString ImageSorter::getNextFileName()
{
    if( m_dirFiles.length() == 0 )
    {
        emit( noNext() );
        return "";
    }
    
    if( m_nextFileIndex < m_dirFiles.length() - 1 )
        m_nextFileIndex = m_curFileIndex + 1;
    else
        m_nextFileIndex = m_dirFiles.length() - 1;
    
    if( m_nextFileIndex == -1 )
        m_nextFileIndex += 2;
    
    if( m_nextFileIndex == m_dirFiles.length() - 1 )
        emit( noNext() );
    
    if( m_nextFileIndex > 0 )
        emit( gotPrevious() );
    
    m_curFileIndex = m_nextFileIndex;

    return m_dirFiles.at( m_nextFileIndex ).absoluteFilePath();
}

QString ImageSorter::getPrevFileName()
{
    m_nextFileIndex = m_curFileIndex - 1;
    
    if( m_nextFileIndex == m_dirFiles.length() )
        m_nextFileIndex -= 2;
    
    if( m_nextFileIndex == 0 )
        emit( noPrevious() );
    
    if( m_nextFileIndex < m_dirFiles.length() - 1 )
        emit( gotNext() );
    
    m_curFileIndex = m_nextFileIndex;
    
    return m_dirFiles.at( m_nextFileIndex ).absoluteFilePath();
}

void ImageSorter::saveSettings()
{
    KConfig config("sortsettingsrc");
    KConfigGroup group = config.group("SortImages Settings");
    
    QString entry = "";
    
    for( int i = 0; i < m_bindings.size(); ++i )
    {
        if( m_bindings[i].key.length() > 0 )
        {
            entry += m_bindings[i].key + "+" + m_bindings[i].path;
            entry += ";";
        }
    }
    
    group.writeEntry( "keybindings", entry );
    group.writeEntry( "mainComb", m_mainComb );
    
    config.sync();
    
    kDebug() << entry;   
}
    
void ImageSorter::bindingRemoved( int row )
{
    m_bindings.removeAt( row );
}

void ImageSorter::loadSettings()
{
    KConfig config("sortsettingsrc");
    KConfigGroup group = config.group("SortImages Settings");
    
    m_mainComb = group.readEntry( "mainComb", "Ctrl+S" );

    QString keyBindings = group.readEntry( "keybindings", "" );
    
    kDebug() << "keyBindings = " << keyBindings;
    
    parseBindings( keyBindings ); 
}

void ImageSorter::parseBindings( const QString& data )
{
    int prevSepIndex = 0;
    int sepIndex = 0;
    
    while( ( sepIndex = data.indexOf( QChar(';'), prevSepIndex ) ) != -1 )
    {
        QString next = data.mid( prevSepIndex, sepIndex - prevSepIndex );
        prevSepIndex = sepIndex + 1;
        
        int div = next.indexOf( QChar('+') );
        
        if( div != -1 )
        {
            binding bind;
            bind.key = next.left( div );
            bind.path = next.mid( div + 1 );
            
            m_bindings.append( bind );
        }
        
        if( sepIndex == data.length() - 1 )
            break;
    }
    
    if( prevSepIndex != data.length() - 1 )
    {
        QString next = data.mid( prevSepIndex );
    
        int div = next.indexOf( QChar(' ') );
        
        if( div != -1 )
        {
            binding bind;
            bind.key = next.left( div );
            bind.path = next.mid( div + 1 );
        
            m_bindings.append( bind );
        }
    }    
}

QList<binding>& ImageSorter::getBindings()
{
    return m_bindings;
}
    
QString& ImageSorter::getMainComb()
{
    return m_mainComb;
}

void ImageSorter::settingChanged( int row, const binding& bin )
{
    int size = m_bindings.size();
    
    for( int i = 0; i < size; ++i )
    {
        if( m_bindings[i].key.compare( bin.key ) == 0 )
        {
            if( m_bindings[i].path.compare( "..." ) != 0 )
            {
                if( row == i )
                    break;
                
                if( row < size )
                    emit( illegalBinding( row, bin, m_bindings[row] ) );
                else
                    emit( illegalBinding( row, bin, { ".", "..." } ) );
                return;
            }
        }
    }
    
    if( row < size )
    {
        m_bindings[row] = bin;
    }
    else
    {
        m_bindings.append(bin);
    }
    
    emit( addNewAction( row ) );
}

void ImageSorter::mainCombChanged( const QString& newComb )
{
    m_mainComb = newComb;
    emit( changeShortcuts() );
}

int ImageSorter::getNoBindings()
{
    return m_bindings.length();
}

QString ImageSorter::getBindSeq( int index )
{
    QString ret;
    QString first = m_mainComb.left( m_mainComb.indexOf("+") );
    ret = m_mainComb + ", " + first + "+" + m_bindings[index].key;
    return ret;
}

bool ImageSorter::performSort( int index )
{
    QString path = m_bindings[index].path;
    
    kDebug()    << "move " << m_dirFiles.at( m_curFileIndex ).absoluteFilePath() << " to "
                << path + QDir::separator() + m_dirFiles.at( m_curFileIndex ).fileName();
                
    KJob * job = KIO::move( KUrl( m_dirFiles.at( m_curFileIndex ).absoluteFilePath() ),
                            KUrl( path + QDir::separator() + m_dirFiles.at( m_curFileIndex ).fileName() ) );
    /*connect( job, SIGNAL( result( KJob * ) ),
            this, SLOT( moveResult( KJob * ) ) );
    job->start();*/
    if( job->exec() )
    {
        m_dirFiles.removeAt( m_curFileIndex );
        --m_curFileIndex;
        
        return true;
    }
    
    return false;
}

/*void ImageSorter::moveResult( KJob* jb )
{

}*/

