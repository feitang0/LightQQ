#include <QPixmap>
#include <QDebug>
#include "getface.h"

GetFace::GetFace(MainPanel *_mainPanel, QObject *parent) : QObject(parent)
{
    mainPanel = _mainPanel;
}

GetFace::~GetFace()
{

}

void GetFace::setFace()
{
    for (int i = 0; i < 10; i++)
        qDebug() << "开始设置头像";
    QList<Friend> friends = mainPanel->friendsHash.values();
    qDebug() << "好友人数：" << friends.length();
    for (int i = 0; i < friends.size(); i++)
    {
//        QString uin = friends.at(i).uin;
          mainPanel->qq->getFace(friends.at(i).uin);
//        mainPanel->getItemWidget(friends.at(i).uin)->setFace(mainPanel->qq->getFace(friends.at(i).uin));
//        friendItem->setFace(face);
    }
    qDebug() << "头像设置完成";
}

