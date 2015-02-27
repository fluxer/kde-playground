
#ifndef KHC_MAINWINDOW_H
#define KHC_MAINWINDOW_H

#include <KXmlGuiWindow>
#include <KUrl>

#include <kio/job.h>
#include <kparts/browserextension.h>

#include "navigator.h"
#include "glossary.h"

class QSplitter;

class LogDialog;

namespace KHC {

class View;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.kde.khelpcenter.khelpcenter")
  public:
    MainWindow();
    ~MainWindow();

  public Q_SLOTS:
    Q_SCRIPTABLE void showHome();
    Q_SCRIPTABLE void lastSearch();

  public Q_SLOTS:
    void print();
    void statusBarMessage(const QString &m);
    void slotShowHome();
    void slotLastSearch();
    void showSearchStderr();
    /**
      Show document corresponding to given URL in viewer part.
    */
    void viewUrl( const QString & );

  protected:
    void setupActions();
    /**
      Show document corresponding to given URL in viewer part.
    */
    void viewUrl( const KUrl &url,
                  const KParts::OpenUrlArguments &args = KParts::OpenUrlArguments(),
                  const KParts::BrowserArguments &browserArgs = KParts::BrowserArguments() );

    virtual void saveProperties( KConfigGroup &config );
    virtual void readProperties( const KConfigGroup &config );

    void readConfig();
    void writeConfig();

  protected Q_SLOTS:
    void enableLastSearchAction();
    void enableCopyTextAction();

  private:
    void stop();

  private Q_SLOTS:
    void slotGlossSelected(const GlossaryEntry &entry);
    void slotStarted(KIO::Job *job);
    void slotInfoMessage(KJob *, const QString &);
    void documentCompleted();
    void slotIncFontSizes();
    void slotDecFontSizes();
    void slotConfigureFonts();
    void slotCopySelectedText();

private:
    void updateFontScaleActions();

    QSplitter *mSplitter;
    View *mDoc;
    Navigator *mNavigator;

    QAction *mLastSearchAction;
    QAction *mCopyText;
    LogDialog *mLogDialog;
};

}

#endif //KHC_MAINWINDOW_H
// vim:ts=2:sw=2:et
