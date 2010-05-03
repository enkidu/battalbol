#include "battle.hpp"
#include <QFile>
#include <QTextStream>
Battle::Battle(QObject * parent) : QObject (parent)
{
    timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer,
            SIGNAL(timeout()),
            this,
            SLOT(tick()));
}

void Battle::tick()
{
    if (!onFinish)
        time = time.addSecs(-1);
    else
    {
        if (backup == 0)
            backup = 29;
        else
            backup--;
    }
    if ((time.second() == 0) && (time.hour() == 0) && (time.minute() == 0))
    {
        onFinish = true;
    }
    if (!onFinish)
    {
        if (time.hour() > 2)
        {
            if((time.minute() % 5 == 0) && (time.second() == 0))
                emit getWall();
        }
        else if ((time.hour() < 3) && (time.hour() > 0))
        {
            if(time.second() == 0)
                emit getWall();
        }
        else
        {
            if(time.second() % 30 == 0)
                emit getWall();
        }
        emit setTimeT(time);
    }
    else
    {
        if (backup %30 == 0)
            emit getWall();
        emit setTimeT(QTime(0,0,backup,0));
    }
}

void Battle::setTime(QTime t)
{
    time = t;
    timer->start();
    onFinish = false;
    backup = 0;
}

void Battle::startCounting(int w, QTime t)
{
    startWall = w;
    time = t;
    timer->start();
    onFinish = false;
    backup = 0;
}

void Battle::actWall(int wall, bool ok)
{
    QLocale locale(QLocale::Polish, QLocale::Poland);
    QString text;
    if (ok && (time.hour() == 0) && (time.minute() == 0) && (time.second() == 0))
    {
        timer->stop();
        text = QString("\x03") + QString("12 Battle ID: ")
               + id
               + " Wall: "
               + locale.toString(wall)
               + " Finished";
    }
    if ( wall >= startWall)
    {
        text = QString("\x03") + QString("4 Battle ID: ")
                        + id
                        + " Wall: "
                        + locale.toString(wall)
                        + " (Secure) Time left: "
                        + time.toString("hh:mm:ss");
    }
    else if (wall >= 0)
    {
        int part = startWall / 5;
        int parts = wall / part;
        QString region;
        switch (parts)
        {
            case 0:
                region = "Administration Center";
                break;
            case 1:
                region = "City";
                break;
            case 2:
                region = "Suburbia";
                break;
            case 3:
                region = "Rural Area";
                break;
            case 4:
                region = "Border Area";
                break;
        }
        text = QString("\x03") + QString("12 Battle ID: ")
                + id
                + " Wall: "
                + locale.toString(wall)
                + " (" + region + ")"
                + " Time left: "
                + time.toString("hh:mm:ss");
    }
    else
    {
        text = QString("\x03") + QString("10 Battle ID: ")
                + id
                + " Wall: "
                + locale.toString(wall)
                + " (Underground) Time left: "
                + time.toString("hh:mm:ss");
    }
    emit setText(text);
    QFile file("/home/enkidu/battalbot/"+id+".txt");
    if(file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append))
    {
        QTextStream stream(&file);
        stream << "Time; "
                << time.toString("hh:mm:ss").toAscii()
                << "; Wall; "
                << QByteArray::number(wall)
                << "\n";
    }
    file.close();
}

void Battle::setStartWall(int wall)
{
    this->startWall = wall;
}

void Battle::setID(int id)
{
    this->id = QString::number(id);
}
