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

%ModuleHeaderCode
#include <ksocketfactory.h>
%End

namespace KSocketFactory
{
QTcpSocket*             connectToHost (const QString& protocol, const QString& host, quint16 port, QObject* parent /Transfer/ = 0);
QTcpSocket*             connectToHost (const QUrl& url, QObject* parent /Transfer/ = 0);
void                    connectToHost (QTcpSocket* socket, const QString& protocol, const QString& host, quint16 port);
void                    connectToHost (QTcpSocket* socket, const QUrl& url);
QTcpSocket*             synchronousConnectToHost (const QString& protocol, const QString& host, quint16 port, int msecs = 30000, QObject* parent /Transfer/ = 0);
QTcpSocket*             synchronousConnectToHost (const QUrl& url, int msecs = 30000, QObject* parent /Transfer/ = 0);
void                    synchronousConnectToHost (QTcpSocket* socket, const QString& protocol, const QString& host, quint16 port, int msecs = 30000);
void                    synchronousConnectToHost (QTcpSocket* socket, const QUrl& url, int msecs = 30000);
QTcpServer*             listen (const QString& protocol, const QHostAddress& address = QHostAddress::Any, quint16 port = 0, QObject* parent /Transfer/ = 0);
QUdpSocket*             datagramSocket (const QString& protocol, const QString& host, QObject* parent /Transfer/ = 0);
QNetworkProxy           proxyForConnection (const QString& protocol, const QString& host);
QNetworkProxy           proxyForListening (const QString& protocol);
QNetworkProxy           proxyForDatagram (const QString& protocol, const QString& host);
};
// KSocketFactory



%ModuleHeaderCode
#include <ksocketfactory.h>
%End

