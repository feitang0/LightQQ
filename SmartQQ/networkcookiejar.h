#ifndef NETWORKCOOKIEJAR_H
#define NETWORKCOOKIEJAR_H

#include <QNetworkCookieJar>
#include <QNetworkCookie>

// 设置的一个存放网络cookies的类，非常重要，如果不设置cookies，服务器会拒绝通信

class NetworkCookieJar : public QNetworkCookieJar
{
public:
    NetworkCookieJar();
    ~NetworkCookieJar();

    QList<QNetworkCookie> getAllCookies();  // 获取全部 cookie
    void setCookies(const QList<QNetworkCookie> & cookieList);  // 设置 cookie
};

#endif // NETWORKCOOKIEJAR_H
