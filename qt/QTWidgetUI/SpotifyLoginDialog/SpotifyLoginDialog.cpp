#include "SpotifyLoginDialog.h"
#include "ui_SpotifyLoginDialog.h"

SpotifyLoginDialog::SpotifyLoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SpotifyLoginDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    ui->verticalLayout->setAlignment(ui->logo, Qt::AlignHCenter);
}

SpotifyLoginDialog::~SpotifyLoginDialog()
{
    delete ui;
}

LibSpotifyLoginParams SpotifyLoginDialog::getLoginParams( const std::string& message, const std::string& oldUsername, bool oldRememberMe )
{
    LibSpotifyLoginParams res;
    res.username = oldUsername;
    res.password = "";
    res.rememberMe = oldRememberMe;

    ui->messageLabel->setText(QString::fromStdString(message));
    ui->username->setText(QString::fromStdString(oldUsername));
    ui->rememberMe->setChecked(oldRememberMe);
    QMetaObject::invokeMethod( this, "exec", Qt::BlockingQueuedConnection );
    int accepted = result();
    if ( accepted )
    {
        res.username = ui->username->text().toStdString();
        res.password = ui->password->text().toStdString();
        ui->password->setText("");
        res.rememberMe = ui->rememberMe->isChecked();
    }
    else
    {
        QMetaObject::invokeMethod( QCoreApplication::instance(), "quit", Qt::BlockingQueuedConnection);
    }
    return res;
}
