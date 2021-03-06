//
//     Copyright 2008 Simon Edwards <simon@simonzone.com>

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



%ModuleHeaderCode
//ctscc
#include <dnssd/domainbrowser.h>
#include <dnssd/domainmodel.h>
#include <dnssd/publicservice.h>
#include <qobject.h>
#include <dnssd/remoteservice.h>
#include <dnssd/servicebrowser.h>
#include <dnssd/servicemodel.h>
#include <dnssd/servicetypebrowser.h>
%End

namespace DNSSD
{

class DomainBrowser : QObject
{
%TypeHeaderCode
#include <dnssd/domainbrowser.h>
%End



public:
    enum DomainType
    {
        Browsing,
        Publishing
    };

    explicit                DomainBrowser (DNSSD::DomainBrowser::DomainType type, QObject* parent /TransferThis/ = 0);
    QStringList             domains () const;
    void                    startBrowse ();
    bool                    isRunning () const;

signals:
    void                    domainRemoved (const QString& domain);
    void                    domainAdded (const QString& domain);


//  Subclasses of QObject

%ConvertToSubClassCode
    // CTSCC for subclasses of 'QObject'
    sipType = NULL;

    if (dynamic_cast<DNSSD::DomainBrowser*>(sipCpp))
        sipType = sipType_DNSSD_DomainBrowser;
    else if (dynamic_cast<DNSSD::PublicService*>(sipCpp))
        sipType = sipType_DNSSD_PublicService;
    else if (dynamic_cast<DNSSD::RemoteService*>(sipCpp))
        sipType = sipType_DNSSD_RemoteService;
    else if (dynamic_cast<DNSSD::ServiceBrowser*>(sipCpp))
        sipType = sipType_DNSSD_ServiceBrowser;
    else if (dynamic_cast<DNSSD::ServiceTypeBrowser*>(sipCpp))
        sipType = sipType_DNSSD_ServiceTypeBrowser;
    else if (dynamic_cast<DNSSD::DomainModel*>(sipCpp))
        sipType = sipType_DNSSD_DomainModel;
    else if (dynamic_cast<DNSSD::ServiceModel*>(sipCpp))
        sipType = sipType_DNSSD_ServiceModel;
%End

public:
    ~DomainBrowser ();
};
// DomainBrowser

};
// DNSSD




