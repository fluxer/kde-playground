#define QT_NO_KEYWORDS
#include <QtCore>
#include <QtGui>
#include <KStyle>
#include <KGlobalSettings>
#include <KSharedConfig>
#include <KLocale>

#include <glib.h>
#include <lightdm-gobject-1/lightdm.h>

#include "ui_kgreeter.h"

QT_USE_NAMESPACE

// For the callbacks
static GMainLoop *glibloop = NULL;

class KGreeter : public QMainWindow
{
    Q_OBJECT
public:
    explicit KGreeter(QWidget *parent = 0);

    QByteArray getUser() const;
    QByteArray getPass() const;
    QByteArray getSession() const;

    LightDMGreeter* getGreater() const;

    static void show_prompt_cb(LightDMGreeter *ldmgreeter, const char *ldmtext, LightDMPromptType ldmtype, gpointer ldmptr);
    static void authentication_complete_cb(LightDMGreeter *ldmgreeter, gpointer ldmptr);
    static void show_message_cb(LightDMGreeter *ldmgreeter, const gchar *ldmtext, LightDMMessageType ldmtype, gpointer ldmptr);

protected:
    void paintEvent(QPaintEvent *event);

private Q_SLOTS:
    void slotSuspend();
    void slotHibernate();
    void slotPoweroff();
    void slotReboot();

    void slotLayout();

    void slotLogin();

private:
    Ui::KGreeter m_ui;
    LightDMGreeter *m_ldmgreeter;
};

KGreeter::KGreeter(QWidget *parent)
    : QMainWindow(parent),
    m_ldmgreeter(nullptr)
{
#if !defined(GLIB_VERSION_2_36)
    g_type_init();
#endif

    m_ui.setupUi(this);

    m_ldmgreeter = lightdm_greeter_new();

    g_signal_connect(m_ldmgreeter, LIGHTDM_GREETER_SIGNAL_SHOW_PROMPT, G_CALLBACK(KGreeter::show_prompt_cb), this);
    g_signal_connect(m_ldmgreeter, LIGHTDM_GREETER_SIGNAL_AUTHENTICATION_COMPLETE, G_CALLBACK(KGreeter::authentication_complete_cb), this);
    g_signal_connect(m_ldmgreeter, LIGHTDM_GREETER_SIGNAL_SHOW_MESSAGE, G_CALLBACK(KGreeter::show_message_cb), this);

    GList *ldmlayouts = lightdm_get_layouts();
    for (GList *ldmitem = ldmlayouts; ldmitem; ldmitem = ldmitem->next) {
        LightDMLayout *ldmlayout = static_cast<LightDMLayout*>(ldmitem->data);
        Q_ASSERT(ldmlayout);

        QAction* layoutaction = new QAction(m_ui.menuKeyboard);
        layoutaction->setText(QString::fromUtf8(lightdm_layout_get_description(ldmlayout)));
        layoutaction->setData(QVariant(QString::fromUtf8(lightdm_layout_get_name(ldmlayout))));
        connect(layoutaction, SIGNAL(triggered()), this, SLOT(slotLayout()));
        m_ui.menuKeyboard->addAction(layoutaction);
    }

    GList *ldmusers = lightdm_user_list_get_users(lightdm_user_list_get_instance());
    for (GList *ldmitem = ldmusers; ldmitem; ldmitem = ldmitem->next) {
        LightDMUser *ldmuser = static_cast<LightDMUser*>(ldmitem->data);
        Q_ASSERT(ldmuser);

        const QString ldmuserimage = QString::fromUtf8(lightdm_user_get_image(ldmuser));
        if (!ldmuserimage.isEmpty()) {
            m_ui.usersbox->addItem(QIcon(QPixmap(ldmuserimage)), QString::fromUtf8(lightdm_user_get_name(ldmuser)));
        } else {
            m_ui.usersbox->addItem(QString::fromUtf8(lightdm_user_get_name(ldmuser)));
        }
    }

    GList *ldmsessions = lightdm_get_sessions();
    for (GList* ldmitem = ldmsessions; ldmitem; ldmitem = ldmitem->next) {
       LightDMSession *ldmsession = static_cast<LightDMSession*>(ldmitem->data);
       Q_ASSERT(ldmsession);

       const QString ldmsessionname = QString::fromUtf8(lightdm_session_get_name(ldmsession));
       const QString ldmsessionkey = QString::fromUtf8(lightdm_session_get_key(ldmsession));
       m_ui.sessionsbox->addItem(ldmsessionname, QVariant(ldmsessionkey));
   }

    const QString ldmdefaultuser = QString::fromUtf8(lightdm_greeter_get_select_user_hint(m_ldmgreeter));
    if (!ldmdefaultuser.isEmpty()) {
        m_ui.usersbox->setEditText(ldmdefaultuser);
    }

    m_ui.groupbox->setTitle(QString::fromUtf8(lightdm_get_hostname()));

    m_ui.actionSuspend->setVisible(lightdm_get_can_suspend());
    m_ui.actionHibernate->setVisible(lightdm_get_can_hibernate());
    m_ui.actionPoweroff->setVisible(lightdm_get_can_shutdown());
    m_ui.actionReboot->setVisible(lightdm_get_can_restart());
    connect(m_ui.actionSuspend, SIGNAL(triggered()), this, SLOT(slotSuspend()));
    connect(m_ui.actionHibernate, SIGNAL(triggered()), this, SLOT(slotHibernate()));
    connect(m_ui.actionPoweroff, SIGNAL(triggered()), this, SLOT(slotPoweroff()));
    connect(m_ui.actionReboot, SIGNAL(triggered()), this, SLOT(slotReboot()));

    connect(m_ui.loginbutton, SIGNAL(pressed()), this, SLOT(slotLogin()));
}

void KGreeter::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QImage image("/home/smil3y/katana/kde-workspace/plasma/wallpapers/data/Auros/contents/images/1280x800.png");
    painter.drawImage(rect(), image);

    QImage image2("/home/smil3y/katana/kde-workspace/kdm/kfrontend/themes/ariya/rectangle.png");
    QSize image2size(m_ui.groupbox->size());
    image2size.rwidth() = image2size.width() * 1.04;
    image2size.rheight() = image2size.height() * 1.6;
    painter.drawImage(m_ui.groupbox->pos(), image2.scaled(image2size));

    QMainWindow::paintEvent(event);
}

QByteArray KGreeter::getUser() const
{
    return m_ui.usersbox->currentText().toLocal8Bit();
}

QByteArray KGreeter::getPass() const
{
    return m_ui.passedit->text().toLocal8Bit();
}

QByteArray KGreeter::getSession() const
{
    return m_ui.sessionsbox->itemData(m_ui.sessionsbox->currentIndex()).toString().toUtf8();
}

LightDMGreeter * KGreeter::getGreater() const
{
    return m_ldmgreeter;
}

void KGreeter::show_prompt_cb(LightDMGreeter *ldmgreeter, const char *ldmtext, LightDMPromptType ldmtype, gpointer ldmptr)
{
    // qDebug() << Q_FUNC_INFO;

    KGreeter* kgreeter = static_cast<KGreeter*>(ldmptr);
    Q_ASSERT(kgreeter);

    if (ldmtype == LIGHTDM_PROMPT_TYPE_SECRET) {
        const QByteArray kgreeterpass = kgreeter->getPass();

        g_autoptr(GError) gliberror = NULL;
        lightdm_greeter_respond(ldmgreeter, kgreeterpass.constData(), &gliberror);
    }
}

void KGreeter::authentication_complete_cb(LightDMGreeter *ldmgreeter, gpointer ldmptr)
{
    // qDebug() << Q_FUNC_INFO;

    KGreeter* kgreeter = static_cast<KGreeter*>(ldmptr);
    Q_ASSERT(kgreeter);

    const QByteArray kgreetersession = kgreeter->getSession();

    // Start the session
    g_autoptr(GError) gliberror = NULL;
    if (!lightdm_greeter_get_is_authenticated(ldmgreeter) ||
        !lightdm_greeter_start_session_sync(ldmgreeter, kgreetersession.constData(), &gliberror))
    {
        kgreeter->statusBar()->showMessage(i18n("Failed to authenticate or start session"));
        g_main_loop_quit(glibloop);
    } else {
        g_main_loop_quit(glibloop);
        qApp->quit();
    }
}

void KGreeter::show_message_cb(LightDMGreeter *ldmgreeter, const gchar *ldmtext, LightDMMessageType ldmtype, gpointer ldmptr)
{
    // qDebug() << Q_FUNC_INFO;

    KGreeter* kgreeter = static_cast<KGreeter*>(ldmptr);
    Q_ASSERT(kgreeter);

    if (ldmtype == LIGHTDM_MESSAGE_TYPE_INFO) {
        kgreeter->statusBar()->showMessage(QString::fromUtf8(ldmtext));
    } else {
        kgreeter->statusBar()->showMessage(QString::fromUtf8(ldmtext));
    }
}

void KGreeter::slotSuspend()
{
    g_autoptr(GError) gliberror = NULL;
    if (!lightdm_suspend(&gliberror)) {
        statusBar()->showMessage(i18n("Could not suspend"));
    }
}

void KGreeter::slotHibernate()
{
    g_autoptr(GError) gliberror = NULL;
    if (!lightdm_hibernate(&gliberror)) {
        statusBar()->showMessage(i18n("Could not hibernate"));
    }
}

void KGreeter::slotPoweroff()
{
    g_autoptr(GError) gliberror = NULL;
    if (!lightdm_shutdown(&gliberror)) {
        statusBar()->showMessage(i18n("Could not poweroff"));
    }
}

void KGreeter::slotReboot()
{
    g_autoptr(GError) gliberror = NULL;
    if (!lightdm_restart(&gliberror)) {
        statusBar()->showMessage(i18n("Could not reboot"));
    }
}

void KGreeter::slotLayout()
{
    const QAction* layoutaction = qobject_cast<QAction*>(sender());
    const QString layoutname = layoutaction->data().toString();
    GList *ldmlayouts = lightdm_get_layouts();
    for (GList *ldmitem = ldmlayouts; ldmitem; ldmitem = ldmitem->next) {
        LightDMLayout *ldmlayout = static_cast<LightDMLayout*>(ldmitem->data);
        Q_ASSERT(ldmlayout);

        if (layoutname == QString::fromUtf8(lightdm_layout_get_name(ldmlayout))) {
            lightdm_set_layout(ldmlayout);
            break;
        }
    }
}

void KGreeter::slotLogin()
{
    const QByteArray kgreeterusername = getUser();

    g_autoptr(GError) gliberror = NULL;
    lightdm_greeter_authenticate(m_ldmgreeter, kgreeterusername.constData(), &gliberror);
    g_main_loop_run(glibloop);
}

int main(int argc, char**argv)
{
    QApplication app(argc, argv);

    app.setStyle(KStyle::defaultStyle());

    QString kcolorscheme = "ObsidianCoast";
    if (kcolorscheme.isEmpty()) {
        app.setPalette(KGlobalSettings::createApplicationPalette());
    } else {
        KSharedConfigPtr kcolorschemeconfig = KSharedConfig::openConfig(QString::fromLatin1("color-schemes/%1.colors").arg(kcolorscheme), KConfig::FullConfig, "data");
        app.setPalette(KGlobalSettings::createApplicationPalette(kcolorschemeconfig));
    }

    glibloop = g_main_loop_new(NULL, false);

    KGreeter kgreeter;
    kgreeter.showMaximized();

    LightDMGreeter *ldmgreeter = kgreeter.getGreater();

    g_autoptr(GError) gliberror = NULL;
    if (!lightdm_greeter_connect_to_daemon_sync(ldmgreeter, &gliberror)) {
        fprintf(stderr, "%s\n", "Could not connect to daemon");
        return EXIT_FAILURE;
    }

    return app.exec();
}

#include "kgreeter.moc"
