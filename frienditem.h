#ifndef FRIENDITEM_H
#define FRIENDITEM_H

#include <QWidget>
class QPixmap;

namespace Ui {
class FriendItem;
}

class FriendItem : public QWidget
{
    Q_OBJECT

public:
    explicit FriendItem(QWidget *parent = 0);
    ~FriendItem();

    void setMarkName(const QString markname);
    void setStatus(const QString status);
    void setFace(const QPixmap face);
    void setLnick(const QString lnick);

private:
    Ui::FriendItem *ui;

};

#endif // FRIENDITEM_H
