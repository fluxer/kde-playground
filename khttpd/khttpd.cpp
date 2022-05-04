/*  This file is part of KHTTPD
    Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#include <KAboutData>
#include <KCmdLineArgs>
#include <KApplication>
#include <KUrl>
#include <KDebug>
#include <QDir>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostInfo>
#include <DNSSD/PublicService>

static QByteArray bodyForPath(const QString &path, const QString &basedir)
{
    QByteArray data;
    data.append("<html>");
    QDir dir(path);
    foreach (const QFileInfo &fileinfo, dir.entryInfoList(QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot)) {
        const QString fullpath = path.toLocal8Bit() + QLatin1Char('/') + fileinfo.fileName();
        // chromium does weird stuff if the link starts with two slashes - removes, the host and
        // port part of the link and converts the first path to lower-case
        const QString cleanpath = QDir::cleanPath(fullpath.mid(basedir.size()));
        // qDebug() << Q_FUNC_INFO << fullpath << basedir << cleanpath;
        data.append("<p><a href=\"");
        data.append(cleanpath.toLocal8Bit());
        data.append("\">");
        data.append(fileinfo.fileName().toLocal8Bit());
        data.append("</a></p>");
    }
    data.append("</html>");
    return data;
}

class HttpHeaderParser
{
public:
    void parseHeader(const QByteArray &header);

    QString path() const { return m_path; }

private:
    QString m_path;
};

void HttpHeaderParser::parseHeader(const QByteArray &header)
{
    bool firstline = true;
    foreach (const QByteArray &line, header.split('\n')) {
        if (line.isEmpty()) {
            firstline = false;
            continue;
        }
        if (firstline) {
            const QList<QByteArray> splitline = line.split(' ');
            if (splitline.size() == 3) {
                m_path = splitline.at(1).trimmed();
            }
        }
        firstline = false;
    }
    // qDebug() << Q_FUNC_INFO << m_path;
}


class KHTTPD : public QObject
{
    Q_OBJECT
public:
    KHTTPD(QObject *parent = nullptr);
    ~KHTTPD();

    bool start(const QString &host, int port, const QString &directory);
    QString errorString() const;

public Q_SLOTS:
    void handleRequest();

private:
    QTcpServer m_tcpserver;
    QString m_directory;
    DNSSD::PublicService *m_publicservice;
};

KHTTPD::KHTTPD(QObject *parent)
    : QObject(parent),
    m_publicservice(nullptr)
{
    connect(&m_tcpserver, SIGNAL(newConnection()), this, SLOT(handleRequest()));
}

KHTTPD::~KHTTPD()
{
    if (m_publicservice) {
        m_publicservice->stop();
        delete m_publicservice;
    }
}

bool KHTTPD::start(const QString &host, int port, const QString &directory)
{
    m_publicservice = new DNSSD::PublicService(
        i18n("KHTTPD@%1", QHostInfo::localHostName()),
        "_http._tcp", port
    );
    m_publicservice->publishAsync();
    m_directory = directory;
    QHostAddress address;
    address.setAddress(host);
    return m_tcpserver.listen(address, port);
}


QString KHTTPD::errorString() const
{
    return m_tcpserver.errorString();
}

void KHTTPD::handleRequest()
{
    QTcpSocket *clientConnection = m_tcpserver.nextPendingConnection();

    connect(clientConnection, SIGNAL(disconnected()),
            clientConnection, SLOT(deleteLater()));

    if (!clientConnection->waitForReadyRead()) {
        clientConnection->disconnectFromHost();
    }

    const QByteArray request(clientConnection->readAll());
    // qDebug() << Q_FUNC_INFO << "request" << request;

    HttpHeaderParser headerparser;
    headerparser.parseHeader(request);

    QFileInfo pathinfo(m_directory + QLatin1Char('/') + headerparser.path());
    // qDebug() << Q_FUNC_INFO << headerparser.path() << pathinfo.filePath();
    const bool isdirectory = pathinfo.isDir();
    const bool isfile = pathinfo.isFile();
    QByteArray block;
    if (headerparser.path() == "/") {
        block.append("HTTP/1.1 200 OK\r\n");
        block.append(QString("Date: %1 GMT\r\n").arg(QDateTime(QDateTime::currentDateTime())
                                                    .toString("ddd, dd MMM yyyy hh:mm:ss")).toAscii());
        block.append("Server: KHTTPD\r\n");

        QByteArray data = bodyForPath(m_directory, m_directory);

        block.append("Content-Type: text/html; charset=UTF-8\r\n");
        block.append(QString("Content-Length: %1\r\n\r\n").arg(data.length()).toAscii());
        block.append(data);
    } else if (isdirectory) {
        block.append("HTTP/1.1 200 OK\r\n");
        block.append(QString("Date: %1 GMT\r\n").arg(QDateTime(QDateTime::currentDateTime())
                                                    .toString("ddd, dd MMM yyyy hh:mm:ss")).toAscii());
        block.append("Server: KHTTPD\r\n");

        QByteArray data = bodyForPath(pathinfo.filePath(), m_directory);

        block.append("Content-Type: text/html; charset=UTF-8\r\n");
        block.append(QString("Content-Length: %1\r\n\r\n").arg(data.length()).toAscii());
        block.append(data);
    } else if (isfile) {
        block.append("HTTP/1.1 200 OK\r\n");
        block.append(QString("Date: %1 GMT\r\n").arg(QDateTime(QDateTime::currentDateTime())
                                                    .toString("ddd, dd MMM yyyy hh:mm:ss")).toAscii());
        block.append("Server: KHTTPD\r\n");

        QFile datafile(pathinfo.filePath());
        datafile.open(QFile::ReadOnly);
        const QByteArray data = datafile.readAll();

        block.append("Content-Type: text/plain; charset=UTF-8\r\n");
        block.append(QString("Content-Length: %1\r\n\r\n").arg(data.length()).toAscii());
        block.append(data);
    } else {
        block.append("HTTP/1.1 404 Not Found\r\n");
        block.append(QString("Date: %1 GMT\r\n").arg(QDateTime(QDateTime::currentDateTime())
                                                    .toString("ddd, dd MMM yyyy hh:mm:ss")).toAscii());
        block.append("Server: KHTTPD\r\n");

        const QByteArray data("<html>404 Not Found</html>");

        block.append("Content-Type: text/html; charset=UTF-8\r\n");
        block.append(QString("Content-Length: %1\r\n\r\n").arg(data.length()).toAscii());
        block.append(data);
    }

    clientConnection->write(block);
    clientConnection->disconnectFromHost();

}

int main(int argc, char** argv)
{
    KAboutData aboutData("khttpd", 0, ki18n("KHTTPD"),
                         "1.0.0", ki18n("Spy desktop over network."),
                         KAboutData::License_GPL_V2,
                         ki18n("(c) 2022 Ivailo Monev"),
                         KLocalizedString(),
                        "http://github.com/fluxer/katana"
                        );

    aboutData.addAuthor(ki18n("Ivailo Monev"),
                        ki18n("Maintainer"),
                        "xakepa10@gmail.com");
    aboutData.setProgramIconName(QLatin1String("krfb"));

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineOptions option;
    option.add("address <address>", ki18n("Address to use, default value"), QHostAddress(QHostAddress::Any).toString().toAscii());
    option.add("port <port>", ki18n("Port to use, default value"), "8080");
    option.add("directory <directory>", ki18n("Directory to use, default value"), QDir::currentPath().toAscii());
    KCmdLineArgs::addCmdLineOptions(option);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KApplication *khttpdapp = new KApplication();

    KHTTPD khttpd(khttpdapp);
    if (!khttpd.start(args->getOption("address"), args->getOption("port").toInt(), args->getOption("directory"))) {
        kWarning() << khttpd.errorString();
        return 1;
    }
    qDebug() << "Server running on" << args->getOption("address") << args->getOption("port") << args->getOption("directory");

    return khttpdapp->exec();
}

#include "khttpd.moc"
