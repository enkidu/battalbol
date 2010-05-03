#include "irc.hpp"
Irc::Irc(QObject * parent) : QObject(parent)
{
    sent = 0;
    timer = new QTimer();
    timer->setInterval(60000);
    timer->start();
    limit = 10;
    connect(timer, SIGNAL(timeout()), this, SLOT(resetSent()));
    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(readyRead()),
            this, SLOT(parse()));
}

void Irc::connectTo(QString server, QString channel, QString nick)
{
    this->channel = channel.toAscii();
    this->nick = nick.toAscii();
    socket->connectToHost(server, 6667);
    QByteArray ba = QByteArray::number(socket->localPort());
    qDebug() << ba;
    socket->write("user "
                  + this->nick
                  + " enkidu "
                  + server.toAscii()
                  + " :battalbot\n");
    socket->write("nick " + this->nick + "\n");
}

void Irc::parse()
{
    QByteArray ba = socket->readLine();
    while (ba != "")
    {
        qDebug() << ba;
        if (ba.startsWith("PING"))
        {
            ba = ba.mid(4).prepend("PONG");
            socket->write(ba);
        }
        else if (ba.startsWith(":"))
        {
            ba = ba.mid(1);
            if (ba.startsWith(this->nick) && ba.contains("MODE " + nick) && ba.contains("+i"))
                socket->write("join " + this->channel + "\n");
            else if (ba.contains(":!wall "))
            {
                QString str(ba);
                QStringList strings = str.split(":!wall ");
                if (strings.count() == 2)
                {
                    QStringList parts = strings[0].split(" ");
                    if (parts.count() > 2)
                    {
                        QString sender = parts[0];
                        QString ident = strings[1];
                        emit wallRequest(sender, ident);
                    }
                }
            }
            else if (ba.contains(":!dmg "))
            {
                QString str(ba);
                QStringList strings = str.split(":!dmg ");
                if (strings.count() == 2)
                {
                    QStringList parts = strings[0].split(" ");
                    if (parts.count() > 2)
                    {
                        QString ident = strings[1];
                        if (sent <= limit)
                        {
                            emit playerRequest(ident);
                            sent++;
                        }
                    }
                }
            }
            else if (ba.contains(":!auth "))
            {
                QString str(ba);
                QStringList strings = str.split(":!auth ");
                qDebug() << strings[0];
                qDebug() << strings[1];
                if (strings.count() == 2)
                {
                    QStringList parts = strings[0].split(" ");
                    if (parts.count() > 2)
                    {
                        QString sender = parts[0];
                        parts = strings[1].split(" ");
                        if (parts.count() == 2)
                        {
                            QString nick = parts[0];
                            QString pass = parts[1];
                            emit authRequest(sender, nick, pass);
                        }
                    }
                }
            }
            else if (ba.contains(":!addauth"))
            {
                QString str(ba);
                QStringList strings = str.split(":!addauth ");
                qDebug() << strings[0];
                qDebug() << strings[1];
                if (strings.count() == 2)
                {
                    QStringList parts = strings[0].split(" ");
                    if (parts.count() > 2)
                    {
                        QString sender = parts[0];
                        parts = strings[1].split(" ");
                        if (parts.count() == 2)
                        {
                            QString nick = parts[0];
                            QString pass = parts[1];
                            emit authAdd(sender, nick, pass);
                        }
                    }
                }
            }
        }
        ba = socket->readLine();
    }
}

void Irc::write(QString text)
{
    socket->write("PRIVMSG " + this->channel + " :" + text.toAscii() + "\n");
}

void Irc::setTopic(QString topic)
{
    socket->write("TOPIC " + this->channel + " :" + topic.toAscii() + "\n");
}

void Irc::resetSent()
{
    sent = 0;
}
