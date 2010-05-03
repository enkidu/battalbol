#include "reader.hpp"
#include <QtDebug>
Reader::Reader(QObject * parent) : QObject(parent)
{
    manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(parse(QNetworkReply*)));
}

void Reader::read()
{
    manager->get(QNetworkRequest(QUrl("http://www.erepublik.com/en/battles/log/" + id)));
    qDebug() << "reading";
}

void Reader::readData()
{
    manager->get(QNetworkRequest(QUrl("http://www.erepublik.com/en/battles/show/" + id)));
}

void Reader::setID(int id)
{
    this->id = QString::number(id);
}

void Reader::getPlayer(QString id)
{
    manager->get(QNetworkRequest(QUrl("http://api.erepublik.com/v1/feeds/citizens/" + id)));
}

void Reader::parse(QNetworkReply *r)
{
    QLocale loc = QLocale(QLocale::C);
    QUrl url = r->url();
    QByteArray ba = r->readAll();
    if (url == QUrl("http://www.erepublik.com/en/battles/log/" + id))
    {
        bool finished = false;
        if (ba.size() < 90)
            finished = true;
        QList<QByteArray> list = ba.split('\n');
        for(int i = 0; i < list.size(); i++)
        {
          if (list[i].contains("current_dp:"))
          {
              qDebug() << list[i];
              QList<QByteArray> chunks = list[i].split(':');
              QByteArray num = chunks[1];
              num.chop(1);
              num = num.replace(" ", "");
          emit wall(num.toInt(), finished);
          }
        }
    }
    else if (url == QUrl("http://www.erepublik.com/en/battles/show/" + id))
    {
        QList<QByteArray> list = ba.split('\n');
        int wall;
        QDateTime critical;
        QDateTime server;
        for(int i = 0; i < list.size(); i++)
        {
          if (list[i].contains("original_dp"))
          {
              QList<QByteArray> chunks = list[i].split('=');
              QByteArray num = chunks[1];
              num.chop(1);
              num = num.replace(" ", "");
              wall =num.toInt();
          }
          else if(list[i].contains("battle_critical_at"))
          {
              QList<QByteArray> chunks = list[i].split('=');
              QByteArray num = chunks[1];
              num.chop(9);
              num = num.mid(16);
              critical = loc.toDateTime(QString::fromAscii(num), "d MMM yyyy HH:mm:ss");
          }
          else if(list[i].contains("server_time"))
          {
              QList<QByteArray> chunks = list[i].split('=');
              QByteArray num = chunks[1];
              num.chop(9);
              num = num.mid(16);
              server = loc.toDateTime(QString::fromAscii(num), "d MMM yyyy HH:mm:ss");
          }
       }
        QTime timeLeft = QTime(0,0,0,0);
        timeLeft = timeLeft.addSecs(server.secsTo(critical));
        qDebug() << timeLeft.toString();
        emit startCounting(wall, timeLeft);
        manager->get(QNetworkRequest(QUrl("http://api.erepublik.com/v1/feeds/battle_logs/" + id + "/0")));
    }
    else if (url ==QUrl("http://api.erepublik.com/v1/feeds/battle_logs/" + id + "/0"))
    {
        QDomDocument doc;
        doc.setContent(ba);
        QDomNode root = doc.documentElement();
        QDomNode battle = root.namedItem("battle-info");
        if (!battle.isNull())
        {
            QDomNode war = battle.namedItem("war");

            QDomNode att = battle.namedItem("attacker");
            if (!att.isNull())
                attacker = att.toElement().text();
            QDomNode def = battle.namedItem("defender");
            if (!def.isNull())
                defender = def.toElement().text();
            if (!war.isNull())
                {
                    war_id = war.toElement().text();
                    manager->get(QNetworkRequest(QUrl("http://api.erepublik.com/v1/feeds/war/" + war_id)));
                }
        }
    }
    else if (url == QUrl("http://api.erepublik.com/v1/feeds/war/" + war_id))
    {
        QDomDocument doc;
        doc.setContent(ba);
        QDomNode root = doc.documentElement();
        QDomNode battles = root.namedItem("battles");
        QDomNode battle = battles.firstChild();
        while (!battle.isNull())
        {
            if (battle.namedItem("battle-id").toElement().text() == id)
            {
                region = battle.namedItem("region").toElement().text();
                qDebug() << region;
                qDebug() << attacker;
                qDebug() << defender;
                if (attacker != defender)
                    emit battleInfo("Battle in region "
                                    + region
                                    + " between "
                                    + attacker
                                    + " and "
                                    + defender
                                    + " | http://www.erepublik.com/en/battles/show/" + id);
                else emit battleInfo("Independence war in region "
                                     + region
                                     + " against country "
                                     + defender
                                     + " | http://www.erepublik.com/en/battles/show/" + id);
            }
            battle = battle.nextSibling();
        }
    }
    else if (url.toString().contains("http://api.erepublik.com/v1/feeds/citizens/"))
    {
        QDomDocument doc;
        doc.setContent(ba);
        QDomNode root = doc.documentElement();
        double bonus = 1.2;
        QString rank = root.namedItem("military-rank").toElement().text();
        double wellness = root.namedItem("wellness").toElement().text().toDouble();
        double strength = root.namedItem("strength").toElement().text().toDouble();
        QString name = root.namedItem("name").toElement().text();
        QString pregion = doc.namedItem("region").toElement().text();
        qDebug() << rank << wellness << strength << name << pregion;
        if (rank == "Private")
            bonus = 1.2;
        else if (rank == "Corporal")
            bonus = 1.4;
        else if (rank == "Sergeant")
            bonus = 1.6;
        else if (rank == "Lieutenant")
            bonus = 1.8;
        else if (rank == "Captain")
            bonus = 2.0;
        else if (rank == "Colonel")
            bonus = 2.2;
        else if (rank == "General")
            bonus = 2.4;
        else if (rank == "Field Marshal")
            bonus = 2.6;
        bonus = bonus * strength;
        double q0 = 0.5 * bonus;
        double q1 = 1.2 * bonus;
        double q2 = 1.4 * bonus;
        double q3 = 1.6 * bonus;
        double q4 = 1.8 * bonus;
        double q5 = 2.0 * bonus;
        double wbmax = 1.75 * 2.0;
        double wb = (1.0 + (wellness - 25)/100.0) * 2.0;
        QString pid = "Player: " + name
                      + "; Region: " + pregion
                      + "; Well:" + QString::number(wellness, 'f', 2)
                      + "; Str: " + QString::number(strength, 'f', 2)
                      + "; Dmg: (Max): "
                      + QString::number(q0 * wbmax, 'f', 0) + " "
                      + QString::number(q1 * wbmax, 'f', 0) + " "
                      + QString::number(q2 * wbmax, 'f', 0) + " "
                      + QString::number(q3 * wbmax, 'f', 0) + " "
                      + QString::number(q4 * wbmax, 'f', 0) + " "
                      + QString::number(q5 * wbmax, 'f', 0)
                      + "; (Cur): "
                      + QString::number(q0 * wb, 'f', 0) + " "
                      + QString::number(q1 * wb, 'f', 0) + " "
                      + QString::number(q2 * wb, 'f', 0) + " "
                      + QString::number(q3 * wb, 'f', 0) + " "
                      + QString::number(q4 * wb, 'f', 0) + " "
                      + QString::number(q5 * wb, 'f', 0);
        emit playerInfo(pid);
    }
    r->deleteLater();
}
