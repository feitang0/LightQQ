#ifndef GETFACE_H
#define GETFACE_H

#include <QObject>
#include <QHash>
#include "SmartQQ/friend.h"
#include "mainpanel.h"

class MainPanel;

class GetFace : public QObject
{
    Q_OBJECT
public:
//    explicit GetFace(QObject *parent = 0);
    explicit GetFace(MainPanel * _mainPanel, QObject *parent = 0);
    ~GetFace();

signals:

public slots:
    void setFace();

private:
    MainPanel *mainPanel;
};

#endif // GETFACE_H
