/*
    This file is part of libkdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "htmldiffalgodisplay.h"
#include <KColorScheme>
#include <QTextDocument>

using namespace KPIM;

static QString textToHTML( const QString &text )
{
  return Qt::convertFromPlainText( text );
}

HTMLDiffAlgoDisplay::HTMLDiffAlgoDisplay( QWidget *parent )
  : KTextBrowser( parent )
{
  setWordWrapMode( QTextOption::WrapAtWordBoundaryOrAnywhere );
  setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff );
}

void HTMLDiffAlgoDisplay::begin()
{
  clear();
  mText.clear();

  mText.append( QLatin1String("<html>") );
  mText.append( QString::fromLatin1( "<body text=\"%1\" bgcolor=\"%2\">" )
               .arg( KColorScheme( QPalette::Active, KColorScheme::View ).foreground().color().name() )
               .arg( KColorScheme( QPalette::Active, KColorScheme::View ).background().color().name() ) );

  mText.append( QLatin1String("<center><table>") );
  mText.append( QString::fromLatin1( "<tr><th></th><th align=\"center\">%1</th><td>         </td><th align=\"center\">%2</th></tr>" )
               .arg( mLeftTitle )
               .arg( mRightTitle ) );
}

void HTMLDiffAlgoDisplay::end()
{
  mText.append( QLatin1String("</table></center>"
                "</body>"
                "</html>") );

  setHtml( mText );
}

void HTMLDiffAlgoDisplay::setLeftSourceTitle( const QString &title )
{
  mLeftTitle = title;
}

void HTMLDiffAlgoDisplay::setRightSourceTitle( const QString &title )
{
  mRightTitle = title;
}

void HTMLDiffAlgoDisplay::additionalLeftField( const QString &id, const QString &value )
{
  mText.append( QString::fromLatin1( "<tr><td align=\"right\"><b>%1:</b></td><td bgcolor=\"#9cff83\">%2</td><td></td><td></td></tr>" )
               .arg( id )
               .arg( textToHTML( value ) ) );
}

void HTMLDiffAlgoDisplay::additionalRightField( const QString &id, const QString &value )
{
  mText.append( QString::fromLatin1( "<tr><td align=\"right\"><b>%1:</b></td><td></td><td></td><td bgcolor=\"#9cff83\">%2</td></tr>" )
               .arg( id )
               .arg( textToHTML( value ) ) );
}

void HTMLDiffAlgoDisplay::conflictField( const QString &id, const QString &leftValue,
                                          const QString &rightValue )
{
  mText.append( QString::fromLatin1( "<tr><td align=\"right\"><b>%1:</b></td><td bgcolor=\"#ff8686\">%2</td><td></td><td bgcolor=\"#ff8686\">%3</td></tr>" )
               .arg( id )
               .arg( textToHTML( leftValue ) )
               .arg( textToHTML( rightValue ) ) );
}
