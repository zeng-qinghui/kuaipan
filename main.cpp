#include "logindialog.h"
#include "accountdialog.h"
#include <QApplication>
#include "kuaipanoauth.h"

Kuaipan::OAuth oauth("xcfRF6CCbvYcjjU2","Bwd3lTTT13vCmwcK");

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AccountDialog *m;
    LoginDialog *w;
    qsrand(time(NULL));
    if(oauth.readOAuthToken()){
        m = new AccountDialog();
        m->show();
    }else{
        w = new LoginDialog();
        w->show();
    }
    return a.exec();;
}
