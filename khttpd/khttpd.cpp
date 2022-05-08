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

#include <kdnssd.h>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KApplication>
#include <KUrl>
#include <KIcon>
#include <KMimeType>
#include <KDebug>
#include <khttp.h>
#include <QBuffer>
#include <QDir>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostInfo>

static QByteArray contentForDirectory(const QString &path, const QString &basedir)
{
    QByteArray data;
    data.append("<html>");
    data.append("<table>");
    data.append("  <tr>");
    data.append("    <th></th>"); // icon
    data.append("    <th>Filename</th>");
    data.append("    <th>MIME</th>");
    data.append("    <th>Size</th>");
    data.append("  </tr>");
    QDir::Filters dirfilters = (QDir::Files | QDir::AllDirs | QDir::NoDot);
    if (QDir::cleanPath(path) == QDir::cleanPath(basedir)) {
        dirfilters = (QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot);
    }
    const QDir::SortFlags dirsortflags = (QDir::Name | QDir::DirsFirst);
    QDir dir(path);
    foreach (const QFileInfo &fileinfo, dir.entryInfoList(dirfilters, dirsortflags)) {
        const QString fullpath = path.toLocal8Bit() + QLatin1Char('/') + fileinfo.fileName();
        // chromium does weird stuff if the link starts with two slashes - removes, the host and
        // port part of the link (or rather does not prepend them) and converts the first directory
        // to lower-case
        const QString cleanpath = QDir::cleanPath(fullpath.mid(basedir.size()));

        data.append("  <tr>");

        const bool isdotdot = (fileinfo.fileName() == QLatin1String(".."));
        if (isdotdot) {
            const QString fileicon = QString::fromLatin1("<img src=\"/khttpd_icons/go-previous\" width=\"20\" height=\"20\">");
            data.append("<td>");
            data.append(fileicon.toAscii());
            data.append("</td>");
        } else {
            const QString fileicon = QString::fromLatin1("<img src=\"/khttpd_icons/%1\" width=\"20\" height=\"20\">").arg(KMimeType::iconNameForUrl(KUrl(fullpath)));
            data.append("<td>");
            data.append(fileicon.toAscii());
            data.append("</td>");
        }

        // qDebug() << Q_FUNC_INFO << fullpath << basedir << cleanpath;
        data.append("<td><a href=\"");
        data.append(cleanpath.toLocal8Bit());
        data.append("\">");
        data.append(fileinfo.fileName().toLocal8Bit());
        data.append("</a><br></td>");

        data.append("<td>");
        if (!isdotdot) {
            const QString filemime = KMimeType::findByPath(fullpath)->name();
            data.append(filemime.toAscii());
        }
        data.append("</td>");

        data.append("<td>");
        if (fileinfo.isFile()) {
            const QString filesize = KGlobal::locale()->formatByteSize(fileinfo.size(), 1);
            data.append(filesize.toAscii());
        }
        data.append("</td>");

        data.append("  </tr>");
    }
    data.append("</table>");
    data.append("</html>");
    return data;
}

class HttpServer : public KHTTP
{
    Q_OBJECT
public:
    HttpServer(QObject *parent = nullptr);

    QString directory;
protected:
    void respond(const QByteArray &url, QByteArray *outdata, ushort *httpstatus, KHTTPHeaders *outheaders) final;
};

HttpServer::HttpServer(QObject *parent)
    : KHTTP(parent)
{
    directory = QDir::currentPath();
}

void HttpServer::respond(const QByteArray &url, QByteArray *outdata, ushort *outhttpstatus, KHTTPHeaders *outheaders)
{
    qDebug() << Q_FUNC_INFO << url;

    const QString normalizedpath = QUrl::fromPercentEncoding(url);
    QFileInfo pathinfo(directory + QLatin1Char('/') + normalizedpath);
    // qDebug() << Q_FUNC_INFO << normalizedpath << pathinfo.filePath();
    const bool isdirectory = pathinfo.isDir();
    const bool isfile = pathinfo.isFile();
    QByteArray block;
    if (normalizedpath.startsWith(QLatin1String("/khttpd_icons/"))) {
        const QPixmap iconpixmap = KIcon(normalizedpath.mid(14)).pixmap(20);
        QBuffer iconbuffer;
        iconbuffer.open(QBuffer::ReadWrite);
        if (!iconpixmap.save(&iconbuffer, "PNG")) {
            kWarning() << "could not save image";
        }
        const QByteArray data = iconbuffer.data();

        *outhttpstatus = 200;
        outheaders->insert("Server", "KHTTPD");
        outheaders->insert("Content-Type", "image/png");

        block.append(data);
    } else if (isdirectory) {
        const QByteArray data = contentForDirectory(pathinfo.filePath(), directory);

        *outhttpstatus = 200;
        outheaders->insert("Server", "KHTTPD");
        outheaders->insert("Content-Type", "text/html; charset=UTF-8");

        block.append(data);
    } else if (isfile) {
        QFile datafile(pathinfo.filePath());
        datafile.open(QFile::ReadOnly);
        const QByteArray data = datafile.readAll();

        const QString filemime = KMimeType::findByPath(pathinfo.filePath())->name();
        *outhttpstatus = 200;
        outheaders->insert("Server", "KHTTPD");
        outheaders->insert("Content-Type", QString::fromLatin1("%1; charset=UTF-8").arg(filemime).toAscii());

        block.append(data);
    } else {
        const QByteArray data("<html>404 Not Found</html>");

        *outhttpstatus = 404;
        outheaders->insert("Server", "KHTTPD");
        outheaders->insert("Content-Type", "text/html; charset=UTF-8");
        block.append(data);
    }

    outdata->clear();
    outdata->append(block);
}


class KHTTPD : public QObject
{
    Q_OBJECT
public:
    KHTTPD(QObject *parent = nullptr);
    ~KHTTPD();

    bool start(const QString &host, int port, const QString &directory);
    QString errorString() const;

private:
    HttpServer m_httpserver;
    KDNSSD m_kdnssd;
};

KHTTPD::KHTTPD(QObject *parent)
    : QObject(parent)
{
}

KHTTPD::~KHTTPD()
{
    m_kdnssd.unpublishService();
}

bool KHTTPD::start(const QString &host, int port, const QString &directory)
{
#if 1
    QFile keyfile("/home/smil3y/katana/kdelibs/kutils/khttp/example.com+5-key.pem");
    // QFile keyfile("/etc/ssl/private/ssl-cert-snakeoil.key");
    if (!keyfile.open(QFile::ReadOnly)) {
        kWarning() << "Could not open key file";
        return false;
    }
    const QByteArray keydata = keyfile.readAll();
    QFile certfile("/home/smil3y/katana/kdelibs/kutils/khttp/example.com+5.pem");
    // QFile certfile("/etc/ssl/certs/ssl-cert-snakeoil.pem");
    if (!certfile.open(QFile::ReadOnly)) {
        kWarning() << "Could not open cert file";
        return false;
    }
    const QByteArray certdata = certfile.readAll();
    if (!m_httpserver.setCertificate(keydata, certdata)) {
        kWarning() << "Could not set certificate";
        return false;
    }
#endif
#if 0
    m_httpserver.setAuthenticate("asd", "123", "Not authorized");
#endif

    m_kdnssd.publishService(
        "_http._tcp", port,
        i18n("KHTTPD@%1", QHostInfo::localHostName())
    );
    m_httpserver.directory = directory;
    QHostAddress address;
    address.setAddress(host);
    return m_httpserver.start(address, port);
}


QString KHTTPD::errorString() const
{
    return m_httpserver.errorString();
}

int main(int argc, char** argv)
{
    KAboutData aboutData(
        "khttpd", 0, ki18n("KHTTPD"),
        "1.0.0", ki18n("Serve directory and publish it on service discovery."),
        KAboutData::License_GPL_V2,
        ki18n("(c) 2022 Ivailo Monev"),
        KLocalizedString(),
        "http://github.com/fluxer/katana"
    );

    aboutData.addAuthor(ki18n("Ivailo Monev"), ki18n("Maintainer"), "xakepa10@gmail.com");
    aboutData.setProgramIconName(QLatin1String("network-server"));

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
