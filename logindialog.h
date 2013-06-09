#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QtNetwork>
#include "kuaipanoauth.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit LoginDialog(QWidget *parent = 0);
    ~LoginDialog();

private slots:
    void on_loginButton_clicked();

    void on_getVerifier_clicked();

private:
    Ui::LoginDialog *ui;
};

#endif // LOGINDIALOG_H
