#include "loginwindow.h"
#include "ui_loginwindow.h"
#include <QDebug>
#include <QMessageBox>

LoginWindow::LoginWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::recvLoginFailed()
{
    QMessageBox messageBox;
    messageBox.setText(tr("登陆失败，请检查你的账户或密码!"));
    messageBox.setIcon(QMessageBox::Critical);
    messageBox.exec();
}

void LoginWindow::recvQrPic(QPixmap qrPic)
{
    ui->iconLabel->setPixmap(qrPic);
}

void LoginWindow::on_loginButton_clicked()
{
    QString uin = ui->qqEdit->text();
    QString pwd = ui->pwdEdit->text();
    qDebug() << "uin is " << uin;
    emit sendMsg(uin, pwd);
}

void LoginWindow::on_qrLoginButton_clicked()
{
    emit qrLoginMsg();
}
