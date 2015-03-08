#include "frienditem.h"
#include "ui_frienditem.h"

FriendItem::FriendItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FriendItem)
{
    ui->setupUi(this);
}

FriendItem::~FriendItem()
{
    delete ui;
}

void FriendItem::setMarkName(const QString markname)
{
    ui->marknameLabel->setText(markname);
}

void FriendItem::setStatus(const QString status)
{
    ui->statusLabel->setText(status);
}

void FriendItem::setFace(const QPixmap face)
{
    ui->faceLabel->setPixmap(face);
}

void FriendItem::setLnick(const QString lnick)
{
    ui->signatureLabel->setText(lnick);
}
