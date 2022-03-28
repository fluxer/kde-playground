/*  This file is part of KDesktopSpy
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
#include <QTcpServer>
#include <QTcpSocket>
#include <QBuffer>
#include <QDesktopWidget>


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


class KDesktopSpy : public QObject
{
    Q_OBJECT
public:
    KDesktopSpy(QObject *parent = nullptr);
    ~KDesktopSpy();

    bool start(const QString &host, int port);
    QString errorString() const;

public Q_SLOTS:
    void handleRequest();

private:
    QTcpServer m_tcpserver;
};

KDesktopSpy::KDesktopSpy(QObject *parent)
    : QObject(parent)
{
    connect(&m_tcpserver, SIGNAL(newConnection()), this, SLOT(handleRequest()));
}

KDesktopSpy::~KDesktopSpy()
{
}

bool KDesktopSpy::start(const QString &host, int port)
{
    QHostAddress address;
    address.setAddress(host);
    return m_tcpserver.listen(address, port);
}


QString KDesktopSpy::errorString() const
{
    return m_tcpserver.errorString();
}

void KDesktopSpy::handleRequest()
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

    QDesktopWidget* desktopwidget = qApp->desktop();

    QByteArray block;
    if (desktopwidget && headerparser.path() == "/") {
        block.append("HTTP/1.1 200 OK\r\n");
        block.append(QString("Date: %1 GMT\r\n").arg(QDateTime(QDateTime::currentDateTime())
                                                    .toString("ddd, dd MMM yyyy hh:mm:ss")).toAscii());
        block.append("Server: KDestopSpy\r\n");

        QByteArray data;
        data.append(QString("<html><img src=\"image.png\" width=\"%1\" height=\"%2\"><meta http-equiv=\"refresh\" content=\"0.5\"></html>").arg(desktopwidget->width()).arg(desktopwidget->height()).toAscii());

        block.append("Content-Type: text/html; charset=UTF-8\r\n");
        block.append(QString("Content-Length: %1\r\n\r\n").arg(data.length()).toAscii());
        block.append(data);
    } else if (desktopwidget && headerparser.path() == "/image.png") {
        block.append("HTTP/1.1 200 OK\r\n");
        block.append(QString("Date: %1 GMT\r\n").arg(QDateTime(QDateTime::currentDateTime())
                                                    .toString("ddd, dd MMM yyyy hh:mm:ss")).toAscii());
        block.append("Server: KDestopSpy\r\n");

        // FIXME: grabbing QDesktopWidget does not work?
        // const QPixmap desktoppixmap = QPixmap::grabWidget((QWidget*)desktopwidget);
        const QPixmap desktoppixmap = QPixmap::fromX11Pixmap(QX11Info::appRootWindow());
        QBuffer desktopbuffer;
        desktopbuffer.open(QBuffer::ReadWrite);
        // desktoppixmap.save("/tmp/image.png", "PNG");
        if (!desktoppixmap.save(&desktopbuffer, "PNG")) {
            kWarning() << "could not save image";
        }
        const QByteArray data = desktopbuffer.data();

        block.append("Content-Type: image/png\r\n");
        block.append(QString("Content-Length: %1\r\n\r\n").arg(data.length()).toAscii());
        block.append(data);
    } else {
        block.append("HTTP/1.1 404 Not Found\r\n");
        block.append(QString("Date: %1 GMT\r\n").arg(QDateTime(QDateTime::currentDateTime())
                                                    .toString("ddd, dd MMM yyyy hh:mm:ss")).toAscii());
        block.append("Server: KDestopSpy\r\n");

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
    KAboutData aboutData("kdesktopspy", 0, ki18n("KDesktopSpy"),
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
    KCmdLineArgs::addCmdLineOptions(option);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KApplication *kdestopspyapp = new KApplication();

    KDesktopSpy kdesktopspy(kdestopspyapp);
    if (!kdesktopspy.start(args->getOption("address"), args->getOption("port").toInt())) {
        kWarning() << kdesktopspy.errorString();
        return 1;
    }
    qDebug() << "Server running on" << args->getOption("address") << args->getOption("port");

    return kdestopspyapp->exec();
}

#include "kdesktopspy.moc"
