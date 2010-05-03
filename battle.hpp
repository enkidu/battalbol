#ifndef BATTLE_HPP
#define BATTLE_HPP

#include <QObject>
#include <QTimer>
#include <QTime>

class Battle : public QObject
{
Q_OBJECT
public:
    Battle(QObject *parent = 0);
public slots:
    void setTime(QTime);
    void actWall(int, bool);
    void setStartWall(int);
    void setID(int);
    void startCounting(int, QTime);
signals:
    void getWall();
    void setText(QString);
    void setTimeT(QTime);
private:
    int startWall;
    QTime time;
    QTimer *timer;
    QString id;
    bool onFinish;
    int backup;
private slots:
    void tick();
};

#endif // BATTLE_HPP
