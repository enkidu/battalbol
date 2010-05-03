#ifndef READER_HPP
#define READER_HPP

#include <QObject>
#include <QtNetwork>
#include <QtXml>

class Reader : public QObject
{
Q_OBJECT
public:
    Reader(QObject * parent = 0);
public slots:
    void read();
    void readData();
    void setID(int);
    void parse(QNetworkReply *);
    void getPlayer(QString);
signals:
    void wall(int, bool);
    void startCounting(int, QTime);
    void battleInfo(QString);
    void playerInfo(QString);
private:
    QNetworkAccessManager *manager;
    QString id;
    QString war_id;
    QString attacker;
    QString defender;
    QString region;
};

#endif // READER_HPP
