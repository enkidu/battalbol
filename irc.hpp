#ifndef IRC_HPP
#define IRC_HPP

#include <QObject>
#include <QtNetwork>

class Irc : public QObject
{
Q_OBJECT
public:
    Irc(QObject * parent = 0);
private:
    QByteArray channel;
    QByteArray nick;
    QTcpSocket *socket;
    QTimer *timer;
    int sent;
    int limit;
public slots:
    void connectTo(QString, QString, QString);
    void setTopic(QString);
    void write(QString);
private slots:
    void parse();
    void resetSent();
signals:
    void authRequest(QString, QString, QString);
    void wallRequest(QString, QString);
    void authAdd(QString, QString, QString);
    void playerRequest(QString);
};

#endif // IRC_HPP
