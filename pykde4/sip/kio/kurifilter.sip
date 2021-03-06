//
//     Copyright 2008 Jim Bublitz <jbublitz@nwinternet.com>
//     Earlier copyrights 1998 - 2007 Jim Bublitz also apply

//                 Generated by twine

// This file is part of PyKDE4.

// PyKDE4 is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of
// the License, or (at your option) any later version.

// PyKDE4 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.



class KUriFilterData
{
%TypeHeaderCode
#include <kurifilter.h>
%End



public:
    enum UriTypes
    {
        NetProtocol,
        LocalFile,
        LocalDir,
        Executable,
        Help,
        Shell,
        Blocked,
        Error,
        Unknown
    };

                            KUriFilterData ();
    explicit                KUriFilterData (const KUrl& url);
    explicit                KUriFilterData (const QString& url);
//ig                            KUriFilterData (const KUriFilterData& data);
    KUrl                    uri () const;
    QString                 errorMsg () const;
    KUriFilterData::UriTypes  uriType () const;
    void                    setData (const KUrl& url);
    void                    setData (const QString& url);
    bool                    setAbsolutePath (const QString& abs_path);
    QString                 absolutePath () const;
    bool                    hasAbsolutePath () const;
    QString                 argsAndOptions () const;
    bool                    hasArgsAndOptions () const;
    QString                 iconName ();
    void                    setCheckForExecutables (bool check);
    bool                    checkForExecutables () const;
    QString                 typedString () const;

private:
//force
                            KUriFilterData (const KUriFilterData& other);
//end
public:
    ~KUriFilterData ();
    QString                 searchTerm () const;
    QChar                   searchTermSeparator () const;
    QString                 searchProvider () const;
    QStringList             preferredSearchProviders () const;
    QString                 queryForPreferredSearchProvider (const QString& provider) const;
    QString                 iconNameForPreferredSearchProvider (const QString& provider) const;
    QStringList             alternateSearchProviders () const;
    QString                 alternateDefaultSearchProvider () const;
    void                    setAlternateSearchProviders (const QStringList& providers);
    void                    setAlternateDefaultSearchProvider (const QString& provider);
    enum SearchFilterOption
    {
        SearchFilterOptionNone,
        RetrieveSearchProvidersOnly,
        RetrievePreferredSearchProvidersOnly,
        RetrieveAvailableSearchProvidersOnly
    };
    typedef QFlags<KUriFilterData::SearchFilterOption> SearchFilterOptions;
    KUriFilterSearchProvider  queryForSearchProvider (const QString& provider) const;
    QStringList             allQueriesForSearchProvider (const QString& provider) const;
    QString                 defaultUrlScheme () const;
    KUriFilterData::SearchFilterOptions  searchFilteringOptions () const;
    void                    setDefaultUrlScheme (const QString&);
    void                    setSearchFilteringOptions (KUriFilterData::SearchFilterOptions options);
};
// KUriFilterData


class KUriFilterPlugin : QObject
{
%TypeHeaderCode
#include <kurifilter.h>
%End


public:
    explicit                KUriFilterPlugin (const QString& name, QObject* parent /TransferThis/ = 0);
    virtual bool            filterUri (KUriFilterData& data) const=0;
    virtual KCModule*       configModule (QWidget*, const char*) const;
    virtual QString         configName () const;

protected:
    void                    setFilteredUri (KUriFilterData& data, const KUrl& uri) const;
    void                    setErrorMsg (KUriFilterData& data, const QString& errmsg) const;
    void                    setUriType (KUriFilterData& data, KUriFilterData::UriTypes type) const;
    void                    setArguments (KUriFilterData& data, const QString& args) const;
    void                    setSearchProvider (KUriFilterData& data, const QString& provider, const QString& term, const QChar& separator) const;
//ig    void                    setPreferredSearchProviders (KUriFilterData& data, const KUriFilterPlugin::ProviderInfoList& providers) const;
    QString                 iconNameFor (const KUrl& url, KUriFilterData::UriTypes type) const;
    void                    setSearchProviders (KUriFilterData& data, const QList<KUriFilterSearchProvider*>& providers) const;
    QHostInfo               resolveName (const QString& hostname, unsigned long timeout) const;
};
// KUriFilterPlugin


class KUriFilter
{
%TypeHeaderCode
#include <kurifilter.h>
%End


public:
    static KUriFilter*      self ();
    bool                    filterUri (KUriFilterData& data, const QStringList& filters = QStringList());
    bool                    filterUri (KUrl& uri, const QStringList& filters = QStringList());
    bool                    filterUri (QString& uri, const QStringList& filters = QStringList());
    KUrl                    filteredUri (const KUrl& uri, const QStringList& filters = QStringList());
    QString                 filteredUri (const QString& uri, const QStringList& filters = QStringList());
    QStringList             pluginNames () const;

protected:
                            KUriFilter ();
    void                    loadPlugins ();

private:
//force
                            KUriFilter (const KUriFilter& other);
//end
public:
    ~KUriFilter ();
    enum SearchFilterType
    {
        NormalTextFilter,
        WebShortcutFilter
    };
    typedef QFlags<KUriFilter::SearchFilterType> SearchFilterTypes;
    bool                    filterSearchUri (KUriFilterData& data, KUriFilter::SearchFilterTypes types);
};
// KUriFilter


class KUriFilterSearchProvider
{
%TypeHeaderCode
#include <kurifilter.h>
%End

public:
                            KUriFilterSearchProvider ();
                            KUriFilterSearchProvider (const KUriFilterSearchProvider&);
    virtual ~KUriFilterSearchProvider ();
    QString                 desktopEntryName () const;
    QString                 name () const;
    QString                 iconName () const;
    QStringList             keys () const;
    QString                 defaultKey () const;
protected:
    void                    setDesktopEntryName (const QString&);
    void                    setIconName (const QString&);
    void                    setKeys (const QStringList&);
    void                    setName (const QString&);
};
