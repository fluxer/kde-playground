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


class KDirWatch : QObject
{
%TypeHeaderCode
#include <kdirwatch.h>
%End



public:
    enum WatchMode
    {
        WatchDirOnly,
        WatchFiles,
        WatchSubDirs
    };

    typedef QFlags<KDirWatch::WatchMode> WatchModes;

                            KDirWatch (QObject* parent /TransferThis/ = 0);
    void                    addDir (const QString& path, KDirWatch::WatchModes watchModes = KDirWatch::WatchDirOnly);
    void                    addFile (const QString& file);
    QDateTime               ctime (const QString& path) const;
    void                    removeDir (const QString& path);
    void                    removeFile (const QString& file);
    bool                    stopDirScan (const QString& path);
    bool                    restartDirScan (const QString& path);
    void                    startScan (bool notify = 0, bool skippedToo = 0);
    void                    stopScan ();
    bool                    isStopped ();
    bool                    contains (const QString& path) const;
    static void             statistics ();
    void                    setCreated (const QString& path);
    void                    setDirty (const QString& path);
    void                    setDeleted (const QString& path);

    enum Method
    {
        FAM,
        INotify,
        DNotify,
        Stat
    };

    KDirWatch::Method       internalMethod ();
    static KDirWatch*       self ();
    static bool             exists ();

signals:
    void                    dirty (const QString& path);
    void                    created (const QString& path);
    void                    deleted (const QString& path);
public:
    ~KDirWatch ();
};
// KDirWatch


