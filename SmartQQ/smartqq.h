#ifndef SMARTQQ_H
#define SMARTQQ_H

#include <QHash>
#include "friend.h"
#include "networkcookiejar.h"

class QString;
class QByteArray;
class QNetworkAccessManager;
class QNetworkRequest;

class SmartQQ
{
public:
    SmartQQ();
    SmartQQ(QByteArray _uin, QByteArray _pwd);
    ~SmartQQ();

    bool qrLogin();
    QString getUin();
    bool getLoginStat();
    void init();
    void getUserFriends2(QHash<QString, Friend> &friendsMap, QHash<int, QString> *qCategories); // 获取好友列表
    void poll2();   // 发送心跳包
    void send_buddy_msg2(const QString to_uin, const QString content);
    void getOnlinebuddies2(QHash<QString, Friend> &friendsHash);
    void getSingleLongNick2(const QString uin, QHash<QString, Friend> &friendsHash);
    void visitQzone(const QString account);
    QString getFriendUin2(const QString uin);
    QPixmap getFace(const QString uin);
    QPixmap getQRPic();

private:

    // 下面几个方法用于加密处理
    QByteArray getEncryptPasswd(const QByteArray &uin, const QByteArray &passwd, const QByteArray &verifyCode);
    QByteArray getPuin(const QByteArray &uin);
    QByteArray hexchar2bin(const QByteArray &str);
    QByteArray hashValue(const QByteArray &x, const QByteArray &K);
    QByteArray hashValue2(const QByteArray &x, const QByteArray &K);

    void setHeaderBase(QNetworkRequest &request);   // 设置基本的Http request头部
    QString get_t();

    // 下面几步用于登陆
    // 检查是否需要验证码，同时获取验证码和 ptvfsession 参数
    bool checkVC(const QByteArray &uin, const QByteArray &login_sig, QByteArray &verifyCode, QByteArray &ptvfsession);
    // 第一次登陆
    bool login(const QByteArray &uin, const QByteArray &p, const QByteArray &verifyCode, const QByteArray &ptvfsession, const QByteArray &login_sig, QByteArray &check_sig);
    // 获取 ptwebqq 参数，从cookie中提取
    void getPtwebqq(const QList<QNetworkCookie> &cookies, QByteArray &ptwebqq);
    // 检查安全参数
    void checkSig(const QByteArray &check_sig);
    // 第二次登陆，获取 psessionid 参数
    bool login2(const QByteArray &ptwebqq, QByteArray &vfwebqq, QByteArray &psessionid);

    QNetworkAccessManager *network;         // 负责网络请求、接收
    NetworkCookieJar *cookieJar;            // 存放 cookies
    void showCookies(const QList<QNetworkCookie> &cookies); // 查看cookie

    // 下面是一些必要的参数
    QByteArray uin;                         // QQ号
    QByteArray pwd;                         // 密码
    QByteArray verifyCode;                  // 验证码
    QByteArray p;                           // 加密后的密码
    QByteArray login_sig;
    QByteArray ptvfsession;            // 必要参数
    QByteArray ptwebqq;                // 必要参数
    QByteArray check_sig;              // 必要参数
    QByteArray vfwebqq;                // 必要参数
    QByteArray psessionid;             // 必要参数
    QByteArray clientid;
    QByteArray hash;
    unsigned long long msg_id;
    bool loginSuccess;
};

#endif // SMARTQQ_H
