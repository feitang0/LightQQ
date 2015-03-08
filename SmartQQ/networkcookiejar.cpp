#include "networkcookiejar.h"

NetworkCookieJar::NetworkCookieJar()
{

}

NetworkCookieJar::~NetworkCookieJar()
{

}

QList<QNetworkCookie> NetworkCookieJar::getAllCookies()
{
    return allCookies();
}

void NetworkCookieJar::setCookies(const QList<QNetworkCookie> &cookieList)
{
    if (this == NULL)
        return;
    this->setAllCookies(cookieList);
}
