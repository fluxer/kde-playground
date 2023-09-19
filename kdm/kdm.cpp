/*  This file is part of the KDE project
    Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

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

#include <QDir>
#include <QSettings>
#include <QGraphicsView>
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsWidget>
#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>
#include <Plasma/Theme>
#include <Plasma/Frame>
#include <Plasma/ScrollWidget>
#include <Plasma/LineEdit>
#include <Plasma/PushButton>
#include <Plasma/IconWidget>
#include <Plasma/FlashingLabel>
#include <Plasma/ComboBox>
#include <KUser>
#include <KLineEdit>
#include <KComboBox>
#include <KAuthorization>
#include <KLocale>
#include <KDebug>

static const int s_minimumuid = 1000;
static const QSizeF s_usericonsize = QSizeF(70, 70);

class KDMBackground : public QGraphicsWidget
{
public:
    KDMBackground(QGraphicsItem *parent = nullptr, Qt::WindowFlags flags = 0);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) final;

private:
    QImage m_background;
    QImage m_backgroundscaled;
};

KDMBackground::KDMBackground(QGraphicsItem *parent, Qt::WindowFlags flags)
    : QGraphicsWidget(parent, flags)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_background = QImage(Plasma::Theme::defaultTheme()->wallpaperPath());
}

void KDMBackground::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    const QSize windowsize = size().toSize();
    if (m_backgroundscaled.isNull() || windowsize != m_backgroundscaled.size()) {
        m_backgroundscaled = m_background.scaled(windowsize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    painter->drawImage(QPoint(0, 0), m_backgroundscaled);
}

class KDMWindow : public QGraphicsView
{
    Q_OBJECT
public:
    KDMWindow(QWidget *parent = nullptr);
    ~KDMWindow();

private Q_SLOTS:
    void slotUserClicked();
    void slotPasswordChanged(const QString &password);
    void slotLoginReleased();

private:
    QGraphicsScene* m_scene;
    QGraphicsWidget* m_widget;
    QGraphicsLinearLayout* m_layout;
    Plasma::Frame* m_frame;
    QGraphicsGridLayout* m_framelayout;
    QGraphicsWidget* m_userswidget;
    QGraphicsLinearLayout* m_userslayout;
    Plasma::LineEdit* m_lineedit;
    Plasma::PushButton* m_pushbutton;
    Plasma::FlashingLabel* m_flashinglabel;
    QGraphicsWidget* m_spacer;
    Plasma::ComboBox* m_combobox;
    QString m_loginname;
};

KDMWindow::KDMWindow(QWidget *parent)
    : QGraphicsView(parent)
{
    m_scene = new QGraphicsScene(this);

    m_widget = new QGraphicsWidget();
    m_layout = new QGraphicsLinearLayout(Qt::Vertical, m_widget);
    m_widget->setLayout(m_layout);

    m_frame = new Plasma::Frame(m_widget);
    m_frame->setFrameShadow(Plasma::Frame::Sunken);
    m_framelayout = new QGraphicsGridLayout(m_frame);
    m_userswidget = new QGraphicsWidget(m_frame);
    m_userslayout = new QGraphicsLinearLayout(Qt::Horizontal, m_userswidget);
    foreach (const KUser &user, KUser::allUsers()) {
        if (user.uid() < s_minimumuid) {
            continue;
        }
        Plasma::IconWidget* userwidget = new Plasma::IconWidget(m_userswidget);
        QString usericon = user.faceIconPath();
        if (usericon.isEmpty()) {
            usericon = QString::fromLatin1("user-identity");
        }
        userwidget->setMinimumIconSize(s_usericonsize);
        userwidget->setMaximumIconSize(s_usericonsize);
        userwidget->setIcon(usericon);
        const QString userloginname = user.loginName();
        QString username = user.property(KUser::FullName);
        if (username.isEmpty()) {
            username = userloginname;
        }
        userwidget->setText(username);
        userwidget->setProperty("_k_loginname", userloginname);
        connect(
            userwidget, SIGNAL(clicked()),
            this, SLOT(slotUserClicked())
        );
        m_userslayout->addItem(userwidget);
    }
    m_userswidget->setLayout(m_userslayout);
    m_framelayout->addItem(m_userswidget, 0, 0);
    m_lineedit = new Plasma::LineEdit(m_frame);
    m_lineedit->setClickMessage(i18n("Enter password..."));
    m_lineedit->nativeWidget()->setPasswordMode(true);
    connect(
        m_lineedit, SIGNAL(textChanged(QString)),
        this, SLOT(slotPasswordChanged(QString))
    );
    m_framelayout->addItem(m_lineedit, 1, 0);
    m_pushbutton = new Plasma::PushButton(m_frame);
    m_pushbutton->setText(i18n("Login"));
    m_pushbutton->setIcon(KIcon("user-online"));
    m_pushbutton->setEnabled(false);
    connect(
        m_pushbutton, SIGNAL(released()),
        this, SLOT(slotLoginReleased())
    );
    m_framelayout->addItem(m_pushbutton, 2, 0);
    m_frame->setLayout(m_framelayout);
    m_layout->addItem(m_frame);

    m_flashinglabel = new Plasma::FlashingLabel(m_widget);
    m_flashinglabel->setAutohide(true);
    m_layout->addItem(m_flashinglabel);

    m_spacer = new QGraphicsWidget(m_widget);
    m_spacer->setMinimumSize(50, 50);
    m_layout->addItem(m_spacer);
    m_combobox = new Plasma::ComboBox(m_widget);
    QDir dir("/usr/share/xsessions");
    foreach (const QString &entry, dir.entryList(QDir::Files)) {
        if (entry.endsWith(".desktop")) {
            QSettings desktopsettings("/usr/share/xsessions/" + entry);
            const QString sessionname = desktopsettings.string("Desktop Entry/Name");
            const QString sessionexec = desktopsettings.string("Desktop Entry/Exec");
            if (!sessionname.isEmpty() && !sessionexec.isEmpty()) {
                const QString sessionicon = desktopsettings.string("Desktop Entry/Icon");
                m_combobox->nativeWidget()->addItem(QIcon::fromTheme(sessionicon), sessionname, sessionexec);
            }
        }
    }
    m_layout->addItem(m_combobox);

    m_scene->addItem(m_widget);
    m_scene->setBackgroundBrush(QBrush(QImage(Plasma::Theme::defaultTheme()->wallpaperPath())));

    setScene(m_scene);
}

KDMWindow::~KDMWindow()
{
}

void KDMWindow::slotUserClicked()
{
    Plasma::IconWidget* userwidget = qobject_cast<Plasma::IconWidget*>(sender());
    m_loginname = userwidget->property("_k_loginname").toString();
}

void KDMWindow::slotPasswordChanged(const QString &password)
{
    m_pushbutton->setEnabled(!password.isEmpty());
}

void KDMWindow::slotLoginReleased()
{
    if (m_loginname.isEmpty()) {
        m_flashinglabel->flash(i18n("Select user first!"));
        kWarning() << "Select user first!";
        return;
    }

    if (!KAuthorization::isAuthorized("org.kde.kdm.helper")) {
        m_flashinglabel->flash(i18n("Not authorized!"));
        kWarning() << "Not authorized!";
        return;
    }

    QVariantMap kdmarguments;
    kdmarguments.insert("user", m_loginname);
    kdmarguments.insert("password", m_lineedit->text());
    int kdmreply = KAuthorization::execute(
        "org.kde.kdm.helper", "check", kdmarguments
    );
    if (kdmreply != 0) {
        m_flashinglabel->flash(i18n("Invalid password!"));
        kWarning() << "Invalid password!" << kdmreply;
        m_lineedit->setText(QString());
        return;
    }

    qDebug() << Q_FUNC_INFO << "password matches";
    kdmarguments.insert("session", m_loginname);
    kdmreply = KAuthorization::execute(
        "org.kde.kdm.helper", "login", kdmarguments
    );
    if (kdmreply != 0) {
        m_flashinglabel->flash(i18n("Could not login!"));
        kWarning() << "Could not login!" << kdmreply;
        m_lineedit->setText(QString());
    }
}


int main(int argc, char**argv)
{
    QApplication app(argc, argv);
    KDMWindow kdm;
    kdm.showMaximized();
    // kdm.show();
    return app.exec();
}

#include "kdm.moc"
