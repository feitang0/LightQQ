#include <QCryptographicHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QPixmap>
#include <QMap>
#include <QHash>
#include <QTimer>
#include <QEventLoop>
#include <QDebug>

#include "smartqq.h"

SmartQQ::SmartQQ()
{
    network = new QNetworkAccessManager();
    cookieJar = new NetworkCookieJar();
    network->setCookieJar(cookieJar);

    login_sig = "DGNb7b9A-Whu7oYleDJwbgDaImPDVbeL5UapjBFAWKiENo18Hdl3PcoF2V8-cXx6";  // 安全参数，可以伪造
    clientid = "53999199";       // 必要参数，不过可以伪造，值在一定范围
    msg_id = 8210001;    // 发送消息时的必要参数，可以伪造，每次发消息时自增1
}

SmartQQ::SmartQQ(QByteArray _uin, QByteArray _pwd)
{
    network = new QNetworkAccessManager();
    cookieJar = new NetworkCookieJar();
    network->setCookieJar(cookieJar);

    uin = _uin;
    pwd = _pwd;
    login_sig = "DGNb7b9A-Whu7oYleDJwbgDaImPDVbeL5UapjBFAWKiENo18Hdl3PcoF2V8-cXx6";  // 安全参数，可以伪造
    clientid = "53999199";       // 必要参数，不过可以伪造，值在一定范围
    msg_id = 8210001;    // 发送消息时的必要参数，可以伪造，每次发消息时自增1
    init();
}

SmartQQ::~SmartQQ()
{
    delete cookieJar;
    delete network;
}

QString SmartQQ::getUin()
{
    return uin;
}

bool SmartQQ::getLoginStat()
{
    if (loginSuccess)
    {
        return true;
    }
    else
        return false;
}

// 登陆步骤，每步都是必须
void SmartQQ::init()
{
    bool ok = false;
    ok = checkVC(uin, login_sig, verifyCode, ptvfsession);
    if (!ok)
    {
        qDebug() << "登陆失败";
        loginSuccess = false;
        return;
    }
    p = getEncryptPasswd(uin, pwd, verifyCode);
    ok = login(uin, p, verifyCode, ptvfsession, login_sig, check_sig);
    if (!ok)
    {
        qDebug() << "登陆失败";
        loginSuccess = false;
        return;
    }
    getPtwebqq(cookieJar->getAllCookies(), ptwebqq);
    qDebug() << "得到的ptwebqq: " << ptwebqq;
    checkSig(check_sig);
    ok = login2(ptwebqq, vfwebqq, psessionid);
    if (!ok)
    {
        qDebug() << "登陆失败";
        loginSuccess = false;
        return;
    }
    hash = hashValue(uin, ptwebqq);
    loginSuccess = true;
}

QByteArray SmartQQ::getEncryptPasswd(const QByteArray &uin, const QByteArray &passwd, const QByteArray &verifyCode)
{
    QByteArray puin = getPuin(uin);
    QByteArray hash1 = QCryptographicHash::hash(passwd, QCryptographicHash::Md5).toHex().toUpper();
    hash1 = hexchar2bin(hash1);
    QByteArray hash2 = QCryptographicHash::hash(hash1 + hexchar2bin(puin), QCryptographicHash::Md5).toHex().toUpper();
    QByteArray hash3 = QCryptographicHash::hash(hash2 + verifyCode.toUpper(), QCryptographicHash::Md5).toHex().toUpper();
    return hash3;
}

QByteArray SmartQQ::getPuin(const QByteArray &uin)
{
    QByteArray puin;
    bool ok;
    unsigned long long uinULL = uin.toULongLong(&ok, 10);
    puin.setNum(uinULL, 16);
    int length = puin.length();
    for (int i = 0; i < 16 - length; i++)
    {
        puin = "0" + puin;
    }
    return puin;
}

QByteArray SmartQQ::hexchar2bin(const QByteArray &str)
{
    QByteArray result;
    for ( int i = 0; i < str.length(); i += 2 )
    {
        bool ok;
        result.append((uchar)str.mid(i, 2).toULong(&ok, 16));
    }
    return result;
}

QByteArray SmartQQ::hashValue2(const QByteArray &x, const QByteArray &K)
{
    QByteArray T = "";
    QByteArray N = "";
    for (N = K + "password error"; ; )
    {
        if (T.length() <= N.length())
        {
            T += x;
            if (T.length() == N.length())
                break;
        }
        else
        {
            T = T.mid(0, N.length());
            break;
        }
    }
    QList<int> V;
    for (int i = 0; i < T.length(); i++)
        V.append( QChar(T.at(i)).unicode() ^ QChar(N.at(i)).unicode() );
//    for (int i = 0; i < V.length(); i++)
//        qDebug() << V.at(i);
    QByteArray M [] = {
        "0", "1", "2", "3", "4", "5", "6", "7",
        "8", "9", "A", "B", "C", "D", "E", "F"
    };
    T = "";
    for (int i = 0; i < V.length(); i++)
    {
        T += M[V[i] >> 4 & 15];
        T += M[V[i] & 15];
    }
    return T;
}

QByteArray SmartQQ::hashValue(const QByteArray &b, const QByteArray &j)
{
    //QByteArray a = "0000";
    int a[4] = { 0, 0, 0, 0 };
    for (int i = 0; i < j.length(); i++)
        a[i % 4] ^= QChar(j.at(i)).unicode();

    QByteArray W[2] = { "EC", "OK" };
    //QByteArray d = "0000";
    int d[4];
    d[0] = b.toLongLong() >> 24 & 255 ^ QChar(W[0].at(0)).unicode();
    d[1] = b.toLongLong() >> 16 & 255 ^ QChar(W[0].at(1)).unicode();
    d[2] = b.toLongLong() >> 8 & 255 ^ QChar(W[1].at(0)).unicode();
    d[3] = b.toLongLong() & 255 ^ QChar(W[1].at(1)).unicode();

    //QByteArray WW = "00000000";
    int WW[8];
    for (int i = 0; i < 8; i++)
        WW[i] = i % 2 == 0 ? a[i >> 1] : d[i >> 1];

    QByteArray aa [] = {
        "0", "1", "2", "3", "4", "5", "6", "7",
        "8", "9", "A", "B", "C", "D", "E", "F"
    };
    QByteArray dd = "";
    for (int i = 0; i < 8; i++)
    {
        dd += aa[WW[i] >> 4 & 15];
        dd += aa[WW[i] & 15];
    }
    return dd;
}


void SmartQQ::setHeaderBase(QNetworkRequest &request)
{
    request.setRawHeader("User-Agent", "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:31.0) Gecko/20100101 Firefox/31.0");
    request.setRawHeader("Accept", "*/*");
    request.setRawHeader("Accept-Language", "en-US,en;q=0.5");
    request.setRawHeader("Accept-Encoding", "gzip,deflate");
    request.setRawHeader("DNT", "1");
    request.setRawHeader("Connection", "keep-alive");
}

QString SmartQQ::get_t()
{
    int secs = QTime::currentTime().secsTo(QTime(1970, 1, 1));
    return QString::number(secs, 10);
}

bool SmartQQ::qrLogin()
{
    QString url = "https://ssl.ptlogin2.qq.com/ptqrlogin?webqq_type=10&remember_uin=1&login2qq=1&aid=501004106&u1=http%3A%2F%2Fw.qq.com%2Fproxy.html%3Flogin2qq%3D1%26webqq_type%3D10&ptredirect=0&ptlang=2052&daid=164&from_ui=1&pttype=1&dumy=&fp=loginerroralert&action=0-0-851467&mibao_css=m_webqq&t=undefined&g=1&js_type=0&js_ver=10107&pt_randsalt=0";
    url += "&login_sig=" + login_sig;
    qDebug() << "qr url is " << url;

    while (true)
    {
        QNetworkRequest request;
        request.setRawHeader("Host", "ssl.ptlogin2.qq.com");
        request.setRawHeader("Referer", "https://ui.ptlogin2.qq.com/cgi-bin/login?daid=164&target=self&style=16&mibao_css=m_webqq&appid=501004106&enable_qlogin=0&no_verifyimg=1&s_url=http%3A%2F%2Fw.qq.com%2Fproxy.html&f_url=loginerroralert&strong_login=1&login_state=10&t=20131024001");
        request.setUrl(url);

        QNetworkReply *reply = network->get(request);
        QEventLoop loop;
        QObject::connect(network, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
        loop.exec();
        QByteArray page;
        if (reply->error() == QNetworkReply::NoError)
        {
            page = reply->readAll();
            qDebug() << "page is " << page;
        }
        else
        {
            qDebug() << reply->errorString();
            qDebug() << "Network error!";
        }

        QList<QByteArray> ptuiCB;
        for (int i = 0; i < page.length(); i++)
        {
            if (page.at(i) == '\'')
            {
                QByteArray temp;
                int j;
                for (j = i + 1; page.at(j) != '\''; j++)
                    temp.append(page.at(j));
                ptuiCB.append(temp);
                i = j;
                continue;
            }
        }

        // 输出第一次登陆获得的参数
        qDebug() << "QR login...";
        qDebug() << "----------------";
        for (int i = 0; i < ptuiCB.size(); i++)
            qDebug() << "ptui " << i << " " << ptuiCB.at(i);
        qDebug() << "----------------\n";
        if (ptuiCB.at(0) == "0")
        {
            check_sig = ptuiCB.at(2);
            qDebug() << "check_sig is " << check_sig;
            break;
        }
        else if (ptuiCB.at(0) == "67")
        {
            qDebug() << "正在认证中...";
        }
    }

    bool ok;
    getPtwebqq(cookieJar->getAllCookies(), ptwebqq);
    qDebug() << "得到的ptwebqq: " << ptwebqq;
    checkSig(check_sig);
    ok = login2(ptwebqq, vfwebqq, psessionid);
    if (!ok)
    {
        qDebug() << "登陆失败";
        loginSuccess = false;
        return false;
    }
    hash = hashValue(uin, ptwebqq);
    loginSuccess = true;
    qDebug() << "登陆成功!";
    return true;
}

bool SmartQQ::login(const QByteArray &uin, const QByteArray &p, const QByteArray &verifyCode, const QByteArray &ptvfsession, const QByteArray &login_sig, QByteArray &check_sig)
{
    QNetworkRequest request;
    setHeaderBase(request);
    request.setRawHeader("Host", "ssl.ptlogin2.qq.com");
    request.setRawHeader("Referer", "https://ui.ptlogin2.qq.com/cgi-bin/login?daid=164&target=self&style=16&mibao_css=m_webqq&appid=501004106&enable_qlogin=0&no_verifyimg=1&s_url=http%3A%2F%2Fw.qq.com%2Fproxy.html&f_url=loginerroralert&strong_login=1&login_state=10&t=20131024001");
    QString url = "https://ssl.ptlogin2.qq.com/login?";
    QString queryString = "webqq_type=10&remember_uin=1&login2qq=1&aid=501004106&u1=http%3A%2F%2Fw.qq.com%2Fproxy.html%3Flogin2qq%3D1%26webqq_type%3D10&h=1&ptredirect=0&ptlang=2052&daid=164&from_ui=1&pttype=1&dumy=&fp=loginerroralert&action=0-20-23951&mibao_css=m_webqq&t=1&g=1&js_type=0&js_ver=10105";
    queryString += "&u=" + uin;
    queryString += "&p=" + p;
    queryString += "&verifycode=" + verifyCode;
    queryString += "&login_sig=" + login_sig;
    queryString += "&pt_verifysession_v1=" + ptvfsession;
    url += queryString;
    request.setUrl(url);

    QNetworkReply *reply = network->get(request);
    QEventLoop loop;
    QObject::connect(network, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    loop.exec();

    QByteArray page;
    if (reply->error() == QNetworkReply::NoError)
    {
        page = reply->readAll();
        reply->deleteLater();
        qDebug() << "第一次登陆，返回的信息如下";
        qDebug() << page;
        qDebug() << "----------------";
    }
    else
    {
        qDebug() << reply->errorString();
        qDebug() << "Network Error!";
    }

    // 获取返回信息
//    QByteArrayList ptuiCB;
    QList<QByteArray> ptuiCB;
    for (int i = 0; i < page.length(); i++)
    {
        if (page.at(i) == '\'')
        {
            QByteArray temp;
            int j;
            for (j = i + 1; page.at(j) != '\''; j++)
                temp.append(page.at(j));
            ptuiCB.append(temp);
            i = j;
            continue;
        }
    }

    // 输出第一次登陆获得的参数
    qDebug() << "first login...";
    qDebug() << "----------------";
    for (int i = 0; i < ptuiCB.size(); i++)
        qDebug() << "ptui " << i << " " << ptuiCB.at(i);
    qDebug() << "----------------\n";
    check_sig = ptuiCB.at(2);

    if (ptuiCB.at(4) == "登录成功！")
        return true;
    return false;
}

void SmartQQ::getPtwebqq(const QList<QNetworkCookie> &cookies, QByteArray &ptwebqq)
{
    for (int i = 0; i < cookies.size(); i++)
    {
        if (cookies.at(i).name() == "ptwebqq")
            ptwebqq = cookies.at(i).value();
    }
}

// 检查安全参数，服务器会设置cookie，很必要
void SmartQQ::checkSig(const QByteArray &check_sig)
{
    QNetworkRequest request;
    setHeaderBase(request);
    request.setRawHeader("Host", "ptlogin4.web2.qq.com");
    qDebug() << "check_sig is " << check_sig;
    request.setUrl(QString(check_sig));
    QNetworkReply *reply = network->get(request);
    QEventLoop loop;
    QObject::connect(network, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    loop.exec();
    QByteArray page;
    if (reply->error() == QNetworkReply::NoError)
    {
        page = reply->readAll();
        reply->deleteLater();
        qDebug() << "check_sig page is " << page;
    }
    else
    {
        qDebug() << reply->errorString();
        qDebug() << "Network Error!";
    }
}

bool SmartQQ::login2(const QByteArray &ptwebqq, QByteArray &vfwebqq, QByteArray &psessionid)
{
    QNetworkRequest request;
    setHeaderBase(request);
    request.setRawHeader("Host", "d.web2.qq.com");
    request.setRawHeader("Accept", "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8");
    request.setRawHeader("Referer", "http://d.web2.qq.com/proxy.html?v=20130916001&callback=1&id=2");
    request.setRawHeader("Pragma", "no-cache");
    request.setRawHeader("Cache-Control", "no-cache");
    request.setUrl(QString("http://d.web2.qq.com/channel/login2"));
//    QByteArray postData = "r={\"ptwebqq\":\"" + ptwebqq + "\",\"clientid\":53999199,\"psessionid\":\"\",\"status\":\"online\"}";
//    r=%7B%22ptwebqq%22%3A%223e20dcd36dc637123b8422326c0266d137c82511b9c35bb47114fc0c3de63874%22%2C%22clientid%22%3A53999199%2C%22psessionid%22%3A%22%22%2C%22status%22%3A%22online%22%7D
    QByteArray postData = "r=%7B%22ptwebqq%22%3A%22" + ptwebqq + "%22%2C%22clientid%22%3A53999199%2C%22psessionid%22%3A%22%22%2C%22status%22%3A%22online%22%7D";
//    postData = postData.toPercentEncoding();
//    postData = QString((QByteArray(postData)).toPercentEncoding());
    qDebug() << "发送的数据" <<  postData;

    QNetworkReply *reply = network->post(request, postData);
    QEventLoop loop;
    QObject::connect(network, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    loop.exec();

    QByteArray page;
    if (reply->error() == QNetworkReply::NoError)
    {
        page = reply->readAll();
        reply->deleteLater();
        qDebug() << "第二次登陆得到的回复";
        qDebug() << page;
        qDebug() << "-----------------";

        QJsonParseError parseError;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(page, &parseError);
        if (parseError.error == QJsonParseError::NoError)
        {
            QVariantMap result = jsonDocument.toVariant().toMap();
            if (result["retcode"].toString() != "0")
            {
                qDebug() << "login failed...";
                return false;
            }
            QVariantMap data = result["result"].toMap();
            uin = data["uin"].toByteArray();
            vfwebqq = data["vfwebqq"].toByteArray();
            psessionid = data["psessionid"].toByteArray();
            qDebug() << "得到的参数:";
            qDebug() << "vfwebqq : " << vfwebqq;
            qDebug() << "psessionid : " << psessionid;
        }
    }
    else
    {
        qDebug() << reply->errorString();
        qDebug() << "Network Error!";
    }
    return true;
}

void SmartQQ::getUserFriends2(QHash<QString, Friend> &friendsHash, QHash<int, QString> *qCategories)
{
    QNetworkRequest request;
    setHeaderBase(request);
    request.setRawHeader("Host", "s.web2.qq.com");
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8");
    request.setRawHeader("Referer", "http://s.web2.qq.com/proxy.html?v=20130916001&callback=1&id=1");
    request.setRawHeader("Pragma", "no-cache");
    request.setRawHeader("Cache-Control", "no-cache");
    request.setUrl(QUrl("http://s.web2.qq.com/api/get_user_friends2"));
    QByteArray postData = "r=%7B%22vfwebqq%22%3A%22" + vfwebqq + "%22%2C%22hash%22%3A%22" + hash + "%22%7D";
    qDebug() << "获取好友列表发送的数据：" << postData;
    QNetworkReply *reply = network->post(request, postData);
    QEventLoop loop;
    QObject::connect(network, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    loop.exec();
    QByteArray page;
    if (reply->error() == QNetworkReply::NoError)
    {
        page = reply->readAll();
        qDebug() << "page is " << page << endl;
        reply->deleteLater();
        int retcode;
        QJsonParseError error;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(page, &error);
        if (error.error == QJsonParseError::NoError)
        {
            QVariantMap json = jsonDocument.toVariant().toMap();
            retcode = json["retcode"].toInt();
            if (retcode != 0)
            {
                qDebug() << "Login Failed...";
                exit(1);
            }
            QVariantMap result = json["result"].toMap();
            QVariantList friends = result["friends"].toList();
            foreach (QVariant qFriend, friends)
            {
                Friend newFriend;
                QVariantMap qqFriend = qFriend.toMap();
                newFriend.uin = qqFriend["uin"].toString();
                newFriend.flag = qqFriend["flag"].toInt();
                newFriend.category = qqFriend["categories"].toInt();
                friendsHash.insert(newFriend.uin, newFriend);
            }

            QVariantList marknames = result["marknames"].toList();
            foreach (QVariant markname, marknames)
            {
                QVariantMap qMarkname = markname.toMap();
                Friend& qFriend = friendsHash[qMarkname["uin"].toString()];
                qFriend.markname = qMarkname["markname"].toString();
                qFriend.type = qMarkname["type"].toInt();
            }

            QVariantList categories = result["categories"].toList();
            foreach (QVariant category, categories)
            {
                QVariantMap qCategory = category.toMap();
                qCategories->insert(qCategory["index"].toInt(), qCategory["name"].toString());
            }

            if (!qCategories->keys().contains(0))
            {
                qCategories->insert(0, "我的好友");
            }

            QVariantList vipinfo = result["vipinfo"].toList();
            foreach (QVariant vipInfo, vipinfo)
            {
                QVariantMap qVipInfo = vipInfo.toMap();
                Friend& qFriend = friendsHash[qVipInfo["u"].toString()];
                qFriend.vip_level = qVipInfo["vip_level"].toInt();
                qFriend.is_vip = qVipInfo["is_vip"].toBool();
            }

            QVariantList info = result["info"].toList();
            foreach (QVariant qInfo, info)
            {
                QVariantMap qqInfo = qInfo.toMap();
                Friend& qFriend = friendsHash[qqInfo["uin"].toString()];
                qFriend.face = qqInfo["face"].toInt();
                qFriend.flag = qqInfo["flag"].toInt();
                qFriend.nick = qqInfo["nick"].toString();
            }
        }
    }
    else
    {
        qDebug() << reply->errorString();
        qDebug() << "Network Error!";
    }
}

void SmartQQ::poll2()
{
    // 设置请求的相关信息
    QNetworkRequest request;
    setHeaderBase(request);
    request.setRawHeader("Host", "d.web2.qq.com");
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8");
    request.setRawHeader("Referer", "http://d.web2.qq.com/proxy.html?v=20130916001&callback=1&id=2");

    // 参数: ptwebqq, clientid, psessionid, key(为空)
    QByteArray postData = "r=%7B%22ptwebqq%22%3A%22" + ptwebqq + "%22%2C%22clientid%22%3A" + clientid + "%2C%22psessionid%22%3A%22" +  psessionid + "%22%2C%22key%22%3A%22%22%7D";

    qDebug() << "发送心跳包数据：" << postData;

    request.setUrl(QString("http://d.web2.qq.com/channel/poll2"));
    QTimer timer;   // 计时器
    timer.setSingleShot(true);
    QEventLoop loop;    // 设置时间循环
    QNetworkReply *reply = network->post(request, postData);
    QObject::connect(network, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    timer.start(5000);  // 设置超时时间，并开始计时
    loop.exec();    // 事件开始

    QByteArray page;
    if (timer.isActive())
    {
        // 如果计时器没有结束
        timer.stop();
        if (reply->error() == QNetworkReply::NoError)
        {
            // 返回JSON数据，未详细处理
            page = reply->readAll();
            reply->deleteLater();
            qDebug() << "----------------";
            qDebug() << "page is " << page;
            qDebug() << "----------------";
            QJsonParseError error;
            QJsonDocument jsonDocument = QJsonDocument::fromJson(page, &error);
            if (error.error == QJsonParseError::NoError)
            {
                QVariantMap response = jsonDocument.toVariant().toMap();
                QString retcode = response["retcode"].toString();
                if (retcode != "0")
                {
                    qDebug() << "有异常...";
                    qDebug() << "错误代码：" << retcode;
                }
                QVariantList result = response["result"].toList();
                for (int i = 0; i < result.size(); i++)
                {
                    QVariantMap msg = result.at(i).toMap();
                    QString poll_type = msg["poll_type"].toString();
                    if (poll_type == "message")
                    {
                        QVariantMap msgContent = msg["value"].toMap();
                        QString msg_id = msgContent["msg_id"].toString();
                        QString from_uin = msgContent["from_uin"].toString();
                        QString to_uin = msgContent["to_uin"].toString();
                        QString msg_id2 = msgContent["msg_id2"].toString();
                        QString msg_type = msgContent["msg_type"].toString();
                        QString reply_ip = msgContent["reply_ip"].toString();
                        QString time = msgContent["time"].toString();
                        QVariantList content = msgContent["content"].toList();
                        qDebug() << "content is " << content;
                        qDebug() << "from : " + from_uin << ", msg: " << content.at(1).toString();
                    }
                    else
                    {
                        qDebug() << "接收到其他消息...";
                        qDebug() << msg["value"];
                        qDebug() << "-----------------";
                    }
                }
            }
        }
        else
        {
            qDebug() << reply->errorString();
            qDebug() << "Network Error!";
        }
    }
    else
    {
        // 计时器结束，网络超时
        QObject::disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        reply->abort();
    }
}

void SmartQQ::send_buddy_msg2(const QString to_uin, const QString content)
{
    // 参数
//    "{"to":2142446475,"content":"[\"和人他\",[\"font\",{\"name\":\"宋体\",\"size\":10,\"style\":[0,0,0],\"color\":\"000000\"}]]","face":594,"clientid":53999199,"msg_id":2450001,"psessionid":"8368046764001d636f6e6e7365727665725f77656271714031302e3133332e34312e383400000398000004f7036e0400b7a560646d0000000a4065377851353549506c6d000000284115ad43fdf68968af6a76de278cf7abb2f925c8048a28076f6913763f013efff286daec0f4a128b"}"
//    to:发送对象
//    content:发送内容
//    msg_id:聊天序列号，但是具体细节未知
    QNetworkRequest request;
    setHeaderBase(request);
    request.setRawHeader("Host", "d.web2.qq.com");
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8");
    request.setRawHeader("Referer", "http://d.web2.qq.com/proxy.html?v=20130916001&callback=1&id=2");
    request.setUrl(QString("http://d.web2.qq.com/channel/send_buddy_msg2"));

    // 格式化参数，共5个参数，依次是 发送对象，消息内容，clientid, msg_id, psessionid
    QString msg = QString("{\"to\":%1,\"content\":\"[\\\"%2\\\",[\\\"font\\\",{\\\"name\\\":\\\"宋体\\\",\\\"size\\\":10,\\\"style\\\":[0,0,0],\\\"color\\\":\\\"000000\\\"}]]\",\"face\":594,\"clientid\":%3,\"msg_id\":%4,\"psessionid\":\"%5\"}").arg(to_uin).arg(content).arg(QString(clientid)).arg(msg_id).arg(QString(psessionid));
    msg = msg.toUtf8();
    msg_id += 1;
    qDebug() << "msg is " << msg;
//    QByteArray msgData = msg.toLocal8Bit();
    QByteArray msgData = msg.toUtf8();
    qDebug() << "msgData is " << msgData;
    msgData = msgData.toPercentEncoding();
    msgData = "r=" + msgData;
    qDebug() << "发送消息发送的数据：" << msgData;

    QEventLoop loop;
    QObject::connect(network, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    QNetworkReply *reply = network->post(request, msgData);
    loop.exec();

    QByteArray page;
    if (reply->error() == QNetworkReply::NoError)
    {
        page = reply->readAll();
        qDebug() << "发送消息返回的数据：" << page;
    }
    else
    {
        qDebug() << reply->errorString();
        qDebug() << "Network Error!";
    }
}

void SmartQQ::getOnlinebuddies2(QHash<QString, Friend> &friendsHash)
{
    QNetworkRequest request;
    setHeaderBase(request);
    request.setRawHeader("Host", "d.web2.qq.com");
    request.setRawHeader("Referer", "http://d.web2.qq.com/proxy.html?v=20130916001&callback=1&id=2");
    QString url = "http://d.web2.qq.com/channel/get_online_buddies2?";
    QString queryString = "vfwebqq=" + vfwebqq + "&clientid=" + clientid + "&psessionid=" + psessionid + "&t=" + get_t();
    request.setUrl(url + queryString);
    QEventLoop loop;
    QNetworkReply *reply = network->get(request);
    QObject::connect(network, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    loop.exec();

    QByteArray page;
    if (reply->error() == QNetworkReply::NoError)
    {
        page = reply->readAll();
        QJsonParseError error;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(page, &error);
        if (error.error == QJsonParseError::NoError)
        {
            QVariantMap response = jsonDocument.toVariant().toMap();
            QString retcode = response["retcode"].toString();
            if (retcode != "0")
            {
                qDebug() << "获取在线好友失败";
                return;
            }
            QVariantList result = response["result"].toList();
            foreach (QVariant singleMsg, result)
            {
                QVariantMap msg = singleMsg.toMap();
                QString uin = msg["uin"].toString();
                QString status = msg["status"].toString();
                QString client_type = msg["client_type"].toString();
                friendsHash[uin].status = status;
                friendsHash[uin].client_type = client_type;

                qDebug() << uin << " " << status;
            }
        }
    }
    else
    {
        qDebug() << reply->errorString();
        qDebug() << "NetworkError...";
    }
    reply->deleteLater();
}

void SmartQQ::getSingleLongNick2(const QString uin, QHash<QString, Friend> &friendsHash)
{
    Friend& singleFriend = friendsHash[uin];
    QNetworkRequest request;
    setHeaderBase(request);
    request.setRawHeader("Host", "s.web2.qq.com");
    request.setRawHeader("Referer", "http://s.web2.qq.com/proxy.html?v=20130916001&callback=1&id=1");
    QString url = "http://s.web2.qq.com/api/get_single_long_nick2?";
    QString queryString = "tuin=" + uin + "&vfwebqq=" + vfwebqq + "&t=" + get_t();
    request.setUrl(url + queryString);
    QNetworkReply *reply = network->get(request);
    QEventLoop loop;
    QObject::connect(network, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    loop.exec();
    QByteArray page;
    if (reply->error() == QNetworkReply::NoError)
    {
        page = reply->readAll();
        qDebug() << "获取好友签名->" << page;
        QJsonParseError error;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(page, &error);
        if (error.error == QJsonParseError::NoError)
        {
            QVariantMap response = jsonDocument.toVariant().toMap();
            QString retcode = response["retcode"].toString();
            if (retcode != "0")
            {
                qDebug() << "获取好友个性签名失败...";
                return;
            }
            QVariantList result = response["result"].toList();
            QVariantMap msg = result.at(0).toMap();
            QString lnick = msg["lnick"].toString();
            qDebug() << uin << " " << lnick;
            singleFriend.lnick = lnick;
        }
    }
    reply->deleteLater();
}

// 不能访问，未实现功能
void SmartQQ::visitQzone(const QString account)
{
    QNetworkRequest request;
    setHeaderBase(request);
    request.setRawHeader("Host", "user.qzone.qq.com");
    QString url = "http://user.qzone.qq.com/" + account;
    request.setUrl(url);
    QNetworkReply *reply = network->get(request);
    QEventLoop loop;
    QObject::connect(network, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    loop.exec();
    QByteArray page;
    if (reply->error() == QNetworkReply::NoError)
    {
        page = reply->readAll();
        qDebug() << page;
    }
    reply->deleteLater();
}

QString SmartQQ::getFriendUin2(const QString uin)
{
    QNetworkRequest request;
    setHeaderBase(request);
    request.setRawHeader("Host", "s.web2.qq.com");
    request.setRawHeader("Referer", "http://s.web2.qq.com/proxy.html?v=20130916001&callback=1&id=1");
    QString url = "http://s.web2.qq.com/api/get_friend_uin2?";
    QString queryString = "tuin=" + uin + "&type=1&vfwebqq=" + vfwebqq + "&t=" + get_t();
    request.setUrl(url + queryString);
    QNetworkReply *reply = network->get(request);
    QEventLoop loop;
    QObject::connect(network, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    loop.exec();
    QByteArray page;
    if (reply->error() == QNetworkReply::NoError)
    {
        page = reply->readAll();
        qDebug() << "获取账户数据->" << page;
        QJsonParseError error;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(page, &error);
        if (error.error == QJsonParseError::NoError)
        {
            QVariantMap response = jsonDocument.toVariant().toMap();
            QString retcode = response["retcode"].toString();
            if (retcode != "0")
            {
                qDebug() << "获取好友账户数据失败...";
                return "error";
            }
            QVariantMap result = response["result"].toMap();
            QString account = result["account"].toString();
            qDebug() << "uin is " << uin << " account is " << account;
            return account;
        }
        else
        {
            qDebug() << "转换Json失败...";
        }
    }
    return "error";
}

QPixmap SmartQQ::getFace(const QString uin)
{
    // 头像请求比较复杂，先向一个站点发送请求，可能会得到图片
    // 若不能得到图片，服务器会返回302状态
    // 此时QNetworkReply缓冲区没有携带任何数据，使用readAll()读取不到数据
    // 重定向的地址信息在返回的Http头部
    // 在QNetworkReply::rawHeaderPairs()里存放
    QNetworkRequest request;
    setHeaderBase(request);
    request.setRawHeader("Host", "face9.web.qq.com");
    request.setRawHeader("Referer", "http://w.qq.com/");
    QString queryString = "getface?cache=0&type=1&f=40&uin=" + uin + "&t=" + get_t() + "&vfwebqq=" + vfwebqq;
    QString url = "http://face9.web.qq.com/cgi/svr/face/" + queryString;
//    QString url = "http://q4.qlogo.cn/g?b=qq&k=zyTslBYpMmtGJWSRmdjHOA&s=41&t=1403751049";
    request.setUrl(url);
    QEventLoop loop;
    QNetworkReply *reply = network->get(request);
    QObject::connect(network, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    loop.exec();
    QByteArray page;
    if (reply->error() == QNetworkReply::NoError)
    {
        page = reply->readAll();
        reply->deleteLater();
        if (page.length() > 0)
        {
            QPixmap face;
            face.loadFromData(page);
            return face;
        }
    }
    else
    {
        qDebug() << reply->errorString();
        return QPixmap(":/face/qqface.jpg");
    }
    QString Reurl = reply->rawHeaderPairs().at(4).second;
    qDebug() << "302 url is " << Reurl;
    request.setUrl(Reurl);
    QNetworkReply *reply2 = network->get(request);
    QObject::connect(reply2, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    if (reply2->error() == QNetworkReply::NoError)
    {
        page = reply2->readAll();
        QPixmap face;
        face.loadFromData(page);
        return face;
    }
    else
    {
        qDebug() << reply->errorString();
        return QPixmap(":/face/face/qqface.jpg");
    }
    return QPixmap(":/face/face/qqface.jpg");
}

QPixmap SmartQQ::getQRPic()
{
    QNetworkRequest request;
    request.setRawHeader("Host", "ssl.ptlogin2.qq.com");
    request.setRawHeader("Referer", "https://ui.ptlogin2.qq.com/cgi-bin/login?daid=164&target=self&style=16&mibao_css=m_webqq&appid=501004106&enable_qlogin=0&no_verifyimg=1&s_url=http%3A%2F%2Fw.qq.com%2Fproxy.html&f_url=loginerroralert&strong_login=1&login_state=10&t=20131024001");
    request.setUrl(QString("https://ssl.ptlogin2.qq.com/ptqrshow?appid=501004106&e=0&l=M&s=5&d=72&v=4&t=0.7005189801688673"));

    qDebug() << "开始请求二维码图像...";
    QEventLoop loop;
    QNetworkReply *reply = network->get(request);
    qDebug() << "已发出请求...";
    QObject::connect(network, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    loop.exec();
    if (reply->error() == QNetworkReply::NoError)
    {
                QByteArray bytes = reply->readAll();
                QPixmap qrPic;
                qrPic.loadFromData(bytes);
                return qrPic;
    }
    else
    {
        qDebug() << reply->errorString();
        qDebug() << "getQRPic: network error!";
    }
    return QPixmap(":/face/face/qqicon.jpg");
}

bool SmartQQ::checkVC(const QByteArray &uin, const QByteArray &login_sig, QByteArray &verifyCode, QByteArray &ptvfsession)
{
    QNetworkRequest request;
//    request.setRawHeader("User-Agent", "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:31.0) Gecko/20100101 Firefox/31.0");
//    request.setRawHeader("Accept", "*/*");
//    request.setRawHeader("Accept-Language", "en-US,en;q=0.5");
//    request.setRawHeader("Accept-Encoding", "gzip,deflate");
//    request.setRawHeader("DNT", "1");
//    request.setRawHeader("Connection", "keep-alive");
    setHeaderBase(request);
    request.setRawHeader("Referer", "https://ui.ptlogin2.qq.com/cgi-bin/login?daid=164&target=self&style=16&mibao_css=m_webqq&appid=501004106&enable_qlogin=0&no_verifyimg=1&s_url=http%3A%2F%2Fw.qq.com%2Fproxy.html&f_url=loginerroralert&strong_login=1&login_state=10&t=20131024001");

    QList<QByteArray> requestHeader = request.rawHeaderList();
    for (int i = 0; i < requestHeader.size(); i++)
    {
        qDebug() << requestHeader.at(i);
    }

    /*
    pt_tea=1
    uin=1684055479
    appid=501004106
    js_ver=10105
    js_type=0
    login_sig=DGNb7b9A-Whu7oYleDJwbgDaImPDVbeL5UapjBFAWKiENo18Hdl3PcoF2V8-cXx6
    u1=http://w.qq.com/proxy.html
    r=0.07290777055609232
    */

    QByteArray queryString = "pt_tea=1&appid=501004106&js_ver=10105&js_type=0&login_sig=" + login_sig + "&u1=http%3A%2F%2Fw.qq.com%2Fproxy.html&r=0.07290777055609232";
    queryString = queryString + "&uin=" + uin;
    //queryString = queryString.toPercentEncoding();

    QByteArray url = "https://ssl.ptlogin2.qq.com/check?" + queryString;

//    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    request.setUrl(QString(url));
//    request.setUrl(QString("https://ui.ptlogin2.qq.com/cgi-bin/login?daid=164&target=self&style=16&mibao_css=m_webqq&appid=501004106&enable_qlogin=0&no_verifyimg=1&s_url=http%3A%2F%2Fw.qq.com%2Fproxy.html&f_url=loginerroralert&strong_login=1&login_state=10&t=20131024001"));
//    request.setUrl(QString("http://www.baidu.com"));
//    request.setUrl(QString("http://w.qq.com"));
//    QSslConfiguration config;
//    request.setSslConfiguration(config);


    QNetworkReply *reply = network->get(request);
    QEventLoop loop;
    QObject::connect(network, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    loop.exec();

    QByteArray page;
    if (reply->error() == QNetworkReply::NoError)
    {
        page =  reply->readAll();
        qDebug() << "检查是否需要验证码，返回信息如下：";
        qDebug() << page;
        qDebug() << "----------------";
    }
    else
    {
        qDebug() << reply->errorString();
        qDebug() << "Network Error!";
    }

//    QByteArrayList ptui;
    QList<QByteArray> ptui;
    for (int i = 0; i < page.length(); i++)
    {
        if (page.at(i) == '\'')
        {
            QByteArray temp;
            int j;
            for (j = i + 1; page.at(j) != '\''; j++)
                temp.append(page.at(j));
            ptui.append(temp);
            i = j;
            continue;
        }
    }
    for (int i = 0; i < ptui.size(); i++)
        qDebug() << "ptui " << i << " " << ptui.at(i);

    verifyCode = ptui.at(1);
    ptvfsession = ptui.at(3);
    if (ptui.at(0) == "0")
       return true;
    else
    {
        qDebug() << "You have to input VerifyCode to login...";
        return false;
    }

    return false;
}
