#ifndef FRIEND_H
#define FRIEND_H

#include <QString>

// 定义一个 Friend 类，用来表示QQ好友
class Friend
{
public:
    Friend();
    ~Friend();

public:
    QString uin;        // 好友的唯一性标示，测试发现每次登陆QQ可能会发生变化
    QString account;    // 好友的QQ号
    QString markname;   // 备注名
    QString nick;       // 昵称
    QString status;     // 状态
    QString client_type;// 客户端类型
    QString lnick;      // 好友签名
    int category;       // 分组
    int type;           // 目前不清楚
    int vip_level;      // vip等级
    int face;           // 目前不清楚，可能和获取头像信息有关
    int flag;           // 目前不清楚
    bool is_vip;        // 是否是VIP，即QQ会员
};

#endif // FRIEND_H
