#include "accountdialog.h"
#include "ui_accountdialog.h"
#include "kuaipanoauth.h"

extern Kuaipan::OAuth oauth;

AccountDialog::AccountDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AccountDialog)
{
    ui->setupUi(this);
    QMap<QString,QVariant> user = oauth.account_info();
    QTextBrowser *profile = this->findChild<QTextBrowser*>("profile");
    QString html = "<ul style='margin-left:-30px;'>";
    html += "<li>" + user["user_name"].toString() + "</li>";
    html += "<li>Used:" + QString::number(user["quota_used"].toLongLong()/1024/1024) + "MB</li>";
    html += "<li>Total:" + QString::number(user["quota_total"].toLongLong()/1024/1024) + "MB</li>";
    html += "</ul>";
    profile->setText(html);
    //oauth.SyncFiles("");
}

AccountDialog::~AccountDialog()
{
    delete ui;
}

void AccountDialog::on_downloadButton_clicked()
{
    QPushButton *btn = this->findChild<QPushButton*>("downloadButton");
    btn->setDisabled(true);
    oauth.SyncFiles("");
    btn->setDisabled(false);
}
