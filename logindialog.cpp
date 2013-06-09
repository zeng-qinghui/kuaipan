#include "logindialog.h"
#include "ui_logindialog.h"
#include <QMessageBox>
#include "kuaipanoauth.h"
#include "accountdialog.h"

using namespace Kuaipan;
extern Kuaipan::OAuth oauth;

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::on_loginButton_clicked()
{
    QLineEdit * verifierCode = this->findChild<QLineEdit *>("verifierCode");
    AccountDialog *m;
    oauth.accessToken(verifierCode->text());
    oauth.saveOAuthToken();
    this->hide();
    m = new AccountDialog();
    m->show();
}

void LoginDialog::on_getVerifier_clicked()
{
    QString link;
    link = oauth.authorize();
    QString cmd = "chromium-browser \""+link+"\" &";
    system(cmd.toStdString().c_str());
}
