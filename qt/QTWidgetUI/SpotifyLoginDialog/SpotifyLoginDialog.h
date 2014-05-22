#ifndef SPOTIFYLOGINDIALOG_H
#define SPOTIFYLOGINDIALOG_H

#include <QDialog>
#include "LibSpotifyIf/ILibSpotifyLoginUI.h"

namespace Ui {
class SpotifyLoginDialog;
}

class SpotifyLoginDialog : public QDialog, public ILibSpotifyLoginUI
{
    Q_OBJECT

public:
    explicit SpotifyLoginDialog(QWidget *parent = 0);
    ~SpotifyLoginDialog();

    virtual LibSpotifyLoginParams getLoginParams( const std::string& message, const std::string& oldUsername, bool oldRememberMe );

private:
    Ui::SpotifyLoginDialog *ui;
};

#endif // SPOTIFYLOGINDIALOG_H
