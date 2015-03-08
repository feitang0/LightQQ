#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = 0);
    ~LoginWindow();

signals:
    void sendMsg(QString, QString);
    void qrLoginMsg();

public slots:
    void recvLoginFailed();
    void recvQrPic(QPixmap qrPic);

private slots:
    void on_loginButton_clicked();
    void on_qrLoginButton_clicked();

private:
    Ui::LoginWindow *ui;
};

#endif // LOGINWINDOW_H
