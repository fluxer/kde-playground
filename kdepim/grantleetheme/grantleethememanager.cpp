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

#include "grantleethememanager.h"
#include "globalsettings_base.h"

#include <KDirWatch>
#include <KConfigGroup>
#include <KConfig>
#include <KActionCollection>
#include <KToggleAction>
#include <KLocalizedString>
#include <KActionMenu>
#include <KStandardDirs>
#include <KDebug>

#include <QDir>
#include <QAction>
#include <QDirIterator>
#include <QActionGroup>

static const KCatalogLoader loader( QLatin1String("libgrantleetheme") );

using namespace GrantleeTheme;

class GrantleeThemeManager::Private
{
public:
    Private(GrantleeTheme::GrantleeThemeManager::Application type, const QString &desktopFileName, KActionCollection *ac, const QString &relativePath, GrantleeThemeManager *qq)
        : applicationType(type),
          defaultDesktopFileName(desktopFileName),
          actionGroup(0),
          menu(0),
          actionCollection(ac),
          q(qq)
    {
        watch = new KDirWatch( q );
        initThemesDirectories(relativePath);
        separatorAction = new QAction(q);
        separatorAction->setSeparator(true);

        q->connect( watch, SIGNAL(dirty(QString)), SLOT(directoryChanged()) );
        updateThemesPath();
    }

    ~Private()
    {
        Q_FOREACH ( KToggleAction *action, themesActionList ) {
            if (actionGroup)
                actionGroup->removeAction(action);
            if (actionCollection)
                actionCollection->removeAction( action );
            delete action;
        }
        themesActionList.clear();
        themes.clear();
    }

    void directoryChanged()
    {
        updateThemesPath();
        updateActionList();
        Q_EMIT q->updateThemes();
    }

    void updateThemesPath()
    {
        if ( !themesDirectories.isEmpty() ) {
            Q_FOREACH (const QString &directory, themesDirectories) {
                watch->removeDir( directory );
            }
        } else {
            return;
        }

        // clear all previous theme information
        themes.clear();

        Q_FOREACH (const QString &directory, themesDirectories) {
            QDirIterator dirIt( directory, QStringList(), QDir::AllDirs | QDir::NoDotAndDotDot );
            QStringList alreadyLoadedThemeName;
            while ( dirIt.hasNext() ) {
                dirIt.next();
                const QString dirName = dirIt.fileName();
                GrantleeTheme::Theme theme = q->loadTheme( dirIt.filePath(), dirName, defaultDesktopFileName );
                if (theme.isValid()) {
                    QString themeName = theme.name();
                    if (alreadyLoadedThemeName.contains(themeName)) {
                        int i = 2;
                        const QString originalName(theme.name());
                        while (alreadyLoadedThemeName.contains(themeName)) {
                            themeName = originalName + QString::fromLatin1(" (%1)").arg(i);
                            ++i;
                        }
                        theme.setName(themeName);
                    }
                    alreadyLoadedThemeName << themeName;
                    themes.insert( dirName, theme );
                    //kDebug()<<" theme.name()"<<theme.name();
                }
            }
            watch->addDir( directory );
        }

        Q_EMIT q->themesChanged();
        watch->startScan();
    }


    void updateActionList()
    {
        if (!actionGroup || !menu)
            return;
        QString themeActivated;
        Q_FOREACH ( KToggleAction *action, themesActionList ) {
            if (action->isChecked()) {
                themeActivated = action->data().toString();
            }
            actionGroup->removeAction(action);
            if (actionCollection)
                actionCollection->removeAction( action );
            menu->removeAction(action);
            delete action;
        }
        menu->removeAction(separatorAction);
        themesActionList.clear();


        bool themeActivatedFound = false;
        QMapIterator<QString, GrantleeTheme::Theme> i(themes);
        while (i.hasNext()) {
            i.next();
            GrantleeTheme::Theme theme = i.value();
            KToggleAction *act = new KToggleAction(theme.name(),q);
            act->setToolTip(theme.description());
            act->setData(theme.dirName());
            if (theme.dirName() == themeActivated) {
                act->setChecked(true);
                themeActivatedFound = true;
            }
            themesActionList.append(act);
            actionGroup->addAction(act);
            menu->addAction(act);
            q->connect(act, SIGNAL(triggered(bool)), q, SLOT(slotThemeSelected()));
        }
        if (!themeActivatedFound) {
            if (!themesActionList.isEmpty() && !themeActivated.isEmpty()) {
                //Activate first item if we removed theme.
                KToggleAction *act = themesActionList.at(0);
                act->setChecked(true);
                selectTheme(act);
            }
        }
        menu->addAction(separatorAction);
    }

    void selectTheme(KToggleAction *act)
    {
        if (act) {
            switch(applicationType) {
            case GrantleeThemeManager::Mail:
                GrantleeSettings::self()->setGrantleeMailThemeName( act->data().toString() );
                break;
            case GrantleeThemeManager::Addressbook:
                GrantleeSettings::self()->setGrantleeAddressBookThemeName( act->data().toString() );
                break;
            }
            GrantleeSettings::self()->writeConfig();
        }
    }

    void slotThemeSelected()
    {
        if (q->sender() ) {
            KToggleAction *act = dynamic_cast<KToggleAction *>(q->sender());
            selectTheme(act);
            Q_EMIT q->grantleeThemeSelected();
        }
    }

    KToggleAction *actionForTheme()
    {
        QString themeName;
        switch(applicationType) {
        case GrantleeThemeManager::Mail:
            themeName = GrantleeSettings::self()->grantleeMailThemeName();
            break;
        case GrantleeThemeManager::Addressbook:
            themeName = GrantleeSettings::self()->grantleeAddressBookThemeName();
            break;
        }
        if (themeName.isEmpty())
            return 0;
        Q_FOREACH(KToggleAction *act, themesActionList) {
            if (act->data().toString() == themeName) {
                return static_cast<KToggleAction*>(act);
            }
        }
        return 0;
    }

    void initThemesDirectories(const QString &themesRelativePath)
    {
        if (!themesRelativePath.isEmpty()) {
            themesDirectories = KGlobal::dirs()->findDirs("data", themesRelativePath);
            if (themesDirectories.count() < 2) {
                //Make sure to add local directory
                const QString localDirectory = KStandardDirs::locateLocal("data", themesRelativePath);
                if (!themesDirectories.contains(localDirectory)) {
                    themesDirectories.append(localDirectory);
                }
            }
        }
    }
    GrantleeThemeManager::Application applicationType;
    QString defaultDesktopFileName;
    QString downloadConfigFileName;
    QStringList themesDirectories;
    QMap<QString, GrantleeTheme::Theme> themes;
    QList<KToggleAction*> themesActionList;
    KDirWatch *watch;
    QActionGroup *actionGroup;
    KActionMenu *menu;
    KActionCollection *actionCollection;
    QAction *separatorAction;

    GrantleeThemeManager *q;
};

GrantleeThemeManager::GrantleeThemeManager(GrantleeTheme::GrantleeThemeManager::Application applicationType, const QString &defaultDesktopFileName, KActionCollection *actionCollection, const QString &path, QObject *parent)
    : QObject(parent), d(new Private(applicationType, defaultDesktopFileName, actionCollection, path,this))
{
}

GrantleeThemeManager::~GrantleeThemeManager()
{
    delete d;
}

QMap<QString, GrantleeTheme::Theme> GrantleeThemeManager::themes() const
{
    return d->themes;
}

void GrantleeThemeManager::setActionGroup( QActionGroup *actionGroup )
{
    if (d->actionGroup != actionGroup) {
        d->actionGroup = actionGroup;
        d->updateActionList();
    }
}

KToggleAction *GrantleeThemeManager::actionForTheme()
{
    return d->actionForTheme();
}

void GrantleeThemeManager::setThemeMenu(KActionMenu *menu)
{
    if (d->menu != menu) {
        d->menu = menu;
        d->updateActionList();
    }
}

QStringList GrantleeThemeManager::displayExtraVariables(const QString &themename) const
{
    QMapIterator<QString, GrantleeTheme::Theme> i(d->themes);
    while (i.hasNext()) {
        i.next();
        if (i.value().dirName() == themename) {
            return i.value().displayExtraVariables();
        }
    }
    return QStringList();
}

GrantleeTheme::Theme GrantleeThemeManager::theme(const QString &themeName)
{
    if (d->themes.contains(themeName)) {
        return d->themes.value(themeName);
    }
    return GrantleeTheme::Theme();
}

void GrantleeThemeManager::setDownloadNewStuffConfigFile(const QString &configFileName)
{
    d->downloadConfigFileName = configFileName;
}

QString GrantleeThemeManager::pathFromThemes(const QString &themesRelativePath, const QString &themeName, const QString &defaultDesktopFileName)
{
    QStringList themesDirectories;
    if (!themesRelativePath.isEmpty()) {
        themesDirectories = KGlobal::dirs()->findDirs("data", themesRelativePath);
        if (themesDirectories.count() < 2) {
            //Make sure to add local directory
            const QString localDirectory = KStandardDirs::locateLocal("data", themesRelativePath);
            if (!themesDirectories.contains(localDirectory)) {
                themesDirectories.append(localDirectory);
            }
        }
        Q_FOREACH (const QString &directory, themesDirectories) {
            QDirIterator dirIt( directory, QStringList(), QDir::AllDirs | QDir::NoDotAndDotDot );
            while ( dirIt.hasNext() ) {
                dirIt.next();
                const QString dirName = dirIt.fileName();
                GrantleeTheme::Theme theme = loadTheme( dirIt.filePath(), dirName, defaultDesktopFileName );
                if (theme.isValid()) {
                    if (dirName == themeName) {
                        return theme.absolutePath();
                    }
                }
            }
        }
    }
    return QString();
}

GrantleeTheme::Theme GrantleeThemeManager::loadTheme(const QString &themePath, const QString &dirName, const QString &defaultDesktopFileName )
{
    const QString themeInfoFile = themePath + QDir::separator() + defaultDesktopFileName;
    KConfig config( themeInfoFile );
    KConfigGroup group( &config, QLatin1String( "Desktop Entry" ) );

    GrantleeTheme::Theme theme;
    theme.setDirName(dirName);
    theme.setName( group.readEntry( "Name", QString() ) );
    theme.setDescription( group.readEntry( "Description", QString() ) );
    theme.setFilename( group.readEntry( "FileName" , QString() ) );
    theme.setDisplayExtraVariables( group.readEntry( "DisplayExtraVariables", QStringList() ) );
    theme.setAbsolutePath(themePath);
    return theme;
}


#include "moc_grantleethememanager.cpp"
