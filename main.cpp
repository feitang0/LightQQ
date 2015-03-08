#include "loginwindow.h"
#include "frienditem.h"
#include "mainpanel.h"
#include "SmartQQ/friend.h"
#include "SmartQQ/smartqq.h"
#include <QDebug>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LoginWindow w;
    MainPanel mainPanel;
    w.show();
    QObject::connect(&w, SIGNAL(sendMsg(QString,QString)), &mainPanel, SLOT(recvMsg(QString,QString)));
    QObject::connect(&w, SIGNAL(qrLoginMsg()), &mainPanel, SLOT(recvQrLoginMsg()));
    QObject::connect(&mainPanel, SIGNAL(sendQrPic(QPixmap)), &w, SLOT(recvQrPic(QPixmap)));
    QObject::connect(&mainPanel, SIGNAL(loginFailed()), &w, SLOT(recvLoginFailed()));
    QObject::connect(&mainPanel, SIGNAL(loginSuccess()), &w, SLOT(hide()));

    return a.exec();
}
