#ifndef MAINPANEL_H
#define MAINPANEL_H

#include <QWidget>
#include <QMultiHash>
#include <QHash>
#include "frienditem.h"
#include "SmartQQ/smartqq.h"
#include "getface.h"

class QListWidgetItem;
class QThread;

namespace Ui {
class MainPanel;
}

class MainPanel : public QWidget
{
    Q_OBJECT
//    QThread getFaceThread;
public:
    explicit MainPanel(QWidget *parent = 0);
    ~MainPanel();

    friend class GetFace;

    FriendItem* getItemWidget(QString uin);

signals:
    void loginFailed();
    void loginSuccess();
    void addFriend();
    void lnickChanged(QString);
    void sendQrPic(QPixmap qrPic);

public slots:
    void recvMsg(QString _uin, QString _pwd);
    void recvQrLoginMsg();

private slots:
    void on_itemClicked(QListWidgetItem *item);
    void on_addFriend();
    void on_lnickChanged(QString uin);

private:
    void init();

    Ui::MainPanel *ui;

    SmartQQ *qq;
    QHash<int, QString> *groupName;                 // key:分组编号 value:分组名称
    QHash<QString, Friend> friendsHash;             // key:好友uin value:好友对象; 需要优化
    QHash<int, QListWidgetItem*> groupItem;         // key:分组编号 value:分组widget
    QHash<QString, QListWidgetItem*> friendItemHash;    // key:好友uin value:好友widget
    QHash<QListWidgetItem*, bool> isGroup;          // key:widget value:是分组还是好友项
    QHash<int, bool> isHidden;                      // key:分组编号 value:是否隐藏
    QMultiHash<int, QListWidgetItem*> groupMemberItem;  // key:分组编号 value:分组成员widget
};

#endif // MAINPANEL_H
