#ifndef ACCOUNTDIALOG_H
#define ACCOUNTDIALOG_H

#include <QDialog>

namespace Ui {
class AccountDialog;
}

class AccountDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AccountDialog(QWidget *parent = 0);
    ~AccountDialog();
    
private slots:
    void on_downloadButton_clicked();

private:
    Ui::AccountDialog *ui;
};

#endif // ACCOUNTDIALOG_H
