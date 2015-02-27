
#include "view.h"

#include "formatter.h"
#include "history.h"

#include <KAction>
#include <KActionCollection>
#include <KApplication>
#include <KDebug>
#include <KLocale>
#include <KMenu>
#include <KStandardDirs>
#include <KToolBarPopupAction>
#include <KGlobal>
#include <KWebPage>

#include <QFileInfo>
#include <QClipboard>
#include <QTextStream>
#include <QKeyEvent>
#include <QEvent>
#include <QScrollBar>

using namespace KHC;

View::View( QWidget *parentWidget, QObject *parent, KActionCollection *col )
    : KWebView( parentWidget, parent ), mState( Docu ), mActionCollection(col)
{
    mFormatter = new Formatter;
    if ( !mFormatter->readTemplates() ) {
      kDebug() << "Unable to read Formatter templates.";
    }

    m_fontScaleStepping = 10;

    connect( this, SIGNAL( setWindowCaption( const QString & ) ),
             this, SLOT( setTitle( const QString & ) ) );
    connect( this, SIGNAL( popupMenu( const QString &, const QPoint& ) ),
             this, SLOT( showMenu( const QString &, const QPoint& ) ) );

    KWebView::installEventFilter( this );
}

View::~View()
{
  delete mFormatter;
}

void View::copySelectedText()
{
  kapp->clipboard()->setText( selectedText() );
}

bool View::openUrl( const KUrl &url )
{
    mState = Docu;
    KWebView::setUrl( url );
    return true;
}

void View::saveState( QDataStream &stream )
{
    stream << mState;
}

void View::restoreState( QDataStream &stream )
{
    stream >> mState;
}

QString View::langLookup( const QString &fname )
{
    QStringList search;

    // assemble the local search paths
    const QStringList localDoc = KGlobal::dirs()->resourceDirs("html");

    // look up the different languages
    for (int id=localDoc.count()-1; id >= 0; --id)
    {
        QStringList langs = KGlobal::locale()->languageList();
        langs.replaceInStrings("en_US", "en");
        langs.append("en");
        QStringList::ConstIterator lang;
        for (lang = langs.constBegin(); lang != langs.constEnd(); ++lang)
            search.append(QString("%1%2/%3").arg(localDoc[id]).arg(*lang).arg(fname));
    }

    // try to locate the file
    QStringList::Iterator it;
    for (it = search.begin(); it != search.end(); ++it)
    {
        QFileInfo info(*it);
        if (info.exists() && info.isFile() && info.isReadable())
            return *it;

		QString file = (*it).left((*it).lastIndexOf('/')) + "/index.docbook";
		info.setFile(file);
		if (info.exists() && info.isFile() && info.isReadable())
			return *it;
    }

    return QString();
}

void View::setTitle( const QString &title )
{
    mTitle = title;
}

void View::beginSearchResult()
{
  mState = Search;

  mSearchResult = "";
}

void View::writeSearchResult( const QString &str )
{
  mSearchResult += str;
}

void View::endSearchResult()
{
  if ( !mSearchResult.isEmpty() ) emit searchResultCacheAvailable();
}

void View::beginInternal( const KUrl &url )
{
  mInternalUrl = url;
}

KUrl View::internalUrl() const
{
  return mInternalUrl;
}

void View::lastSearch()
{
  if ( mSearchResult.isEmpty() ) return;

  mState = Search;
}

void View::slotIncFontSizes()
{
}

void View::slotDecFontSizes()
{
}

void View::showMenu( const QString& url, const QPoint& pos)
{
}

void View::slotCopyLink()
{
  QApplication::clipboard()->setText(mCopyURL);
}

bool View::prevPage(bool checkOnly)
{
  return true;
}

bool View::nextPage(bool checkOnly)
{
  return true;
}

bool View::eventFilter( QObject *o, QEvent *e )
{
  return false;
}

void View::slotReload( const KUrl &url )
{
  openUrl( url );
}

#include "moc_view.cpp"
// vim:ts=2:sw=2:et
