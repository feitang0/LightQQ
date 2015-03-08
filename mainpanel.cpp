#include "mainpanel.h"
#include "ui_mainpanel.h"

#include <QDebug>
#include <QThread>
#include <QListWidget>
#include <QListWidgetItem>

MainPanel::MainPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainPanel)
{
    ui->setupUi(this);

    groupName = new QHash<int, QString>();
    connect(ui->friendListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(on_itemClicked(QListWidgetItem*)));
    connect(this, SIGNAL(addFriend()), this, SLOT(on_addFriend()));
    connect(this, SIGNAL(lnickChanged(QString)), this, SLOT(on_lnickChanged(QString)));
}

MainPanel::~MainPanel()
{
    delete ui;
}

FriendItem *MainPanel::getItemWidget(QString uin)
{
    QListWidgetItem *item = friendItemHash.value(uin);
    FriendItem* widgetItem = (FriendItem *)ui->friendListWidget->itemWidget(item);
    return widgetItem;
}

void MainPanel::recvMsg(QString _uin, QString _pwd)
{
    qq = new SmartQQ(_uin.toLatin1(), _pwd.toLatin1());
    if (!qq->getLoginStat())
    {
        emit loginFailed();
        return;
    }
    emit loginSuccess();
    this->show();
    init();
}

void MainPanel::recvQrLoginMsg()
{
    qDebug() << "收到扫描二维码的登陆信号";
    qDebug() << "实例化QQ对象...";
    qq = new SmartQQ();
    qDebug() << "开始二维码登陆...";
    QPixmap qrPic = qq->getQRPic();
    qDebug() << "得到二维码图像信息...";
    emit sendQrPic(qrPic);
    qq->qrLogin();
    if (!qq->getLoginStat())
    {
        emit loginFailed();
        return;
    }
    emit loginSuccess();
    this->show();
    init();
}

void MainPanel::on_itemClicked(QListWidgetItem *item)
{
    if (isGroup.value(item))
    {
        int groupIndex = groupItem.key(item);
        bool& hidden = isHidden[groupIndex];
        if (isHidden.value(groupIndex))
        {
            item->setIcon(QIcon(":icon/icon/open_arrow_fire.png"));
            hidden = false;
        }
        else
        {
            item->setIcon(QIcon(":icon/icon/open_arrow.png"));
            hidden = true;
        }
        QList<QListWidgetItem *> memberItems = groupMemberItem.values(groupIndex);
        for (int i = 0; i < memberItems.size(); i++)
        {
            QListWidgetItem *member = memberItems.at(i);
            member->setHidden(!member->isHidden());
        }
    }
}

void MainPanel::on_addFriend()
{
    QList<QString> friendsUin = friendsHash.keys();
    for (int i = 0; i < friendsUin.size(); i++)
    {
        QString uin = friendsUin.at(i);
        QPixmap face = qq->getFace(uin);
        QListWidgetItem *friendItem = friendItemHash.value(uin);
        FriendItem *item = (FriendItem *)ui->friendListWidget->itemWidget(friendItem);
        item->setFace(face);
    }
}

void MainPanel::on_lnickChanged(QString uin)
{
    QListWidgetItem *item = friendItemHash.value(uin);
    FriendItem *friendWidget = (FriendItem *)ui->friendListWidget->itemWidget(item);
    friendWidget->setLnick(friendsHash.value(uin).lnick);
}

void MainPanel::init()
{
    ui->faceLabel->setPixmap(qq->getFace(qq->getUin()));
    qq->getUserFriends2(friendsHash, groupName);
    qq->getOnlinebuddies2(friendsHash);

    QList<int> groupIndexList = groupName->keys();
    qSort(groupIndexList);

    for (int i = 0; i < groupIndexList.size(); i++)
    {
        qDebug() << groupIndexList.at(i) << " " <<groupName->value(groupIndexList.at(i));
    }

//    QHash<int, QString>::iterator i;
//    for (i = groupName->begin(); i != groupName->end(); i++)
//    {
//        qDebug() << i.key() << " " << i.value();
//    }

    QList<QString> friendsUin = friendsHash.keys();
    for (int i = 0; i < friendsUin.size(); i++)
    {
        qDebug() << friendsUin.at(i) << friendsHash.value(friendsUin.at(i)).markname;
    }

//    QList<int> groupIndex = groupName->keys();
//    groupName->insert(0, "我的好友");

//    for (int i = 0; i < groupIndex.size(); i++)
//    {
//        QListWidgetItem *newGroupItem = new QListWidgetItem();
//        newGroupItem->setSizeHint(QSize(0, 33));
//        newGroupItem->setIcon(QIcon(":/icon/icon/open_arrow.png"));
//        newGroupItem->setText(groupName->value(i));
//        ui->friendListWidget->addItem(newGroupItem);
//        groupItem.insert(groupIndex.at(i), newGroupItem);
//        isGroup.insert(newGroupItem, true);
//        isHidden.insert(groupIndex.at(i), true);
//    }

    for (int i = 0; i < groupIndexList.size(); i++)
    {
        QListWidgetItem *newGroupItem = new QListWidgetItem();
        newGroupItem->setSizeHint(QSize(0, 33));
        newGroupItem->setIcon(QIcon(":/icon/icon/open_arrow.png"));
        newGroupItem->setText(groupName->value(groupIndexList.at(i)));
        ui->friendListWidget->addItem(newGroupItem);
        groupItem.insert(groupIndexList.at(i), newGroupItem);
        isGroup.insert(newGroupItem, true);
        isHidden.insert(groupIndexList.at(i), true);
    }

    QList<Friend> friends = friendsHash.values();
    for (int i = 0; i < friends.size(); i++)
    {
        Friend qFriend = friends.at(i);
        QListWidgetItem *newItem = new QListWidgetItem();
        friendItemHash.insert(qFriend.uin, newItem);
        newItem->setSizeHint(QSize(0, 58));
        FriendItem *friendItem = new FriendItem();
        if (!qFriend.markname.compare(""))
        {
            friendItem->setMarkName(qFriend.nick);
        }
        else
        {
            friendItem->setMarkName(qFriend.markname);
        }
        if (qFriend.status == "online")
        {
            friendItem->setStatus("[在线]");
        }
        friendItem->setFace(QPixmap(":/face/face/qqface.jpg"));
        int groupIndex = qFriend.category;
        QListWidgetItem* groupPlace = groupItem.value(groupIndex);
        ui->friendListWidget->insertItem(ui->friendListWidget->row(groupPlace) + groupMemberItem.values(groupIndex).size() + 1, newItem);
        groupMemberItem.insert(groupIndex, newItem);
        if (isHidden.value(groupIndex))
        {
            newItem->setHidden(true);
        }
        else
        {
            newItem->setHidden(false);
        }
        ui->friendListWidget->setItemWidget(newItem, friendItem);
    }
    qDebug() << "列表加载完毕";
    emit addFriend();
    qDebug() << "信号释放完毕";

    /***使用线程加载好友头像，有bug
    QThread *getFaceThread = new QThread;
    GetFace *setFace = new GetFace(this);
    setFace->moveToThread(getFaceThread);
    connect(getFaceThread, SIGNAL(started()), setFace, SLOT(setFace()));
    connect(getFaceThread, SIGNAL(finished()), setFace, SLOT(deleteLater()));
    connect(getFaceThread, SIGNAL(finished()), getFaceThread, SLOT(deleteLater()));
    getFaceThread->start();
    ***/

    for (int i = 0; i < friendsUin.size(); i++)
    {
        qq->getSingleLongNick2(friendsUin.at(i), friendsHash);
        emit lnickChanged(friendsUin.at(i));
    }
    for (int i = 0; i < friendsUin.size(); i++)
    {
        QString account = qq->getFriendUin2(friendsUin.at(i));
        friendsHash[friendsUin.at(i)].account = account;
    }
    qDebug() << "获取好友QQ号完毕...";
    while (true)
        qq->poll2();
}
