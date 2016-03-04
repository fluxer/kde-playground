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

#ifndef IMAGE_SORTER
#define IMAGE_SORTER

#include <QString>
#include <QDir>

#include "binding.h"


class ImageSorter : public QObject
{
    Q_OBJECT
public:
    explicit ImageSorter( QObject* parent = 0 );
    
    void setCurrentFile( const QString& file );
    
    QString getNextFileName();
    QString getPrevFileName();
    
    void loadSettings();
    
    QList<binding>& getBindings();
    QString& getMainComb();
    
    int getNoBindings();
    QString getBindSeq( int index );
    
    bool performSort( int index );

public slots:
    void mainCombChanged( const QString& );
    void saveSettings();
    void bindingRemoved( int );
    void settingChanged( int row, const binding& bin );
    
signals:
    void noNext();
    void noPrevious();
    void gotNext();
    void gotPrevious();
    
    void changeShortcuts();
    
    void illegalBinding( int row, const binding& changed, const binding& prev );
    void addNewAction( int );
    
private:
    void parseBindings( const QString& bindings );
    
    QDir m_curDirectory;
    QFileInfoList m_dirFiles;
    int m_curFileIndex;
    int m_nextFileIndex;
    QString m_mainComb;
    QList<binding> m_bindings;
};

#endif
