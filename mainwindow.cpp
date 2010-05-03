#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMenu>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    for (int i = 0; i < 6 ;i++)
    {
        levels.append(0);
    }
    ui->setupUi(this);
    settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "enkidu", "battalbot");
    ui->le_server->setText(settings->value("server", "").toString());
    ui->le_channel->setText(settings->value("channel", "").toString());
    ui->le_botname->setText(settings->value("botname", "").toString());
    QHash<QString, QVariant> tmp;
    auths = settings->value("auths", tmp).toHash();
    onFinish = false;
    added = 0;
    QLocale::setDefault(QLocale::English);
    trayIcon = new QSystemTrayIcon();
    QMenu *menu = new QMenu();
    QAction *actionQuit = new QAction("Quit", trayIcon);
    menu->addAction(actionQuit);
    trayIcon->setContextMenu(menu);
    trayIcon->setIcon(QIcon(":/icon.png"));
    trayIcon->show();
    this->setWindowIcon(QIcon(":/icon.png"));
    this->setWindowTitle("eRepublik wall-watching BOT");
    connect(ui->pb_connect, SIGNAL(clicked()), this, SLOT(connectToServer()));
    irc = new Irc(this);
    reader = new Reader;
    battle = new Battle;
    refreshTree();
    connect(battle, SIGNAL(getWall()), reader, SLOT(read()));
    connect(ui->pb_StartCount, SIGNAL(clicked()), this, SLOT(getInfo()));
    connect(reader, SIGNAL(startCounting(int,QTime)), battle, SLOT(startCounting(int, QTime)));
    connect(reader, SIGNAL(startCounting(int,QTime)), this, SLOT(setInitialWall(int,QTime)));
    connect(battle, SIGNAL(setTimeT(QTime)), this, SLOT(setTimeElapsed(QTime)));
    connect(reader, SIGNAL(wall(int,bool)), battle, SLOT(actWall(int,bool)));
    connect(reader, SIGNAL(wall(int,bool)), this, SLOT(setWall(int,bool)));
    connect(battle, SIGNAL(setText(QString)), irc, SLOT(write(QString)));
    connect(reader, SIGNAL(battleInfo(QString)), irc, SLOT(setTopic(QString)));

    connect(trayIcon,
            SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this,
            SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));

    connect(irc, SIGNAL(authRequest(QString,QString,QString)),
            this, SLOT(auth(QString,QString,QString)));

    connect(irc, SIGNAL(wallRequest(QString,QString)),
            this, SLOT(wall(QString,QString)));

    connect(irc, SIGNAL(playerRequest(QString)),
            reader, SLOT(getPlayer(QString)));

    connect(reader, SIGNAL(playerInfo(QString)),
            irc, SLOT(write(QString)));

    connect(ui->pb_authAdd, SIGNAL(clicked()),
            this, SLOT(addAuth()));

    connect(ui->pb_delAuth, SIGNAL(clicked()),
            this, SLOT(delAuth()));

    connect(actionQuit, SIGNAL(triggered()), this, SLOT(appQuit()));
    curve = new QwtPlotCurve();
    curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve->attach(ui->qwtPlot);
    for (int i = 0; i < 6; i++)
    {
        QwtPlotCurve *line = new QwtPlotCurve();
        if (i == 0 || i == 5)
        {
            line->setPen(QPen(QColor(0,0,255)));
        }
        else
        {
            line->setPen(QPen(QColor(0,200,0)));
        }
        line->attach(ui->qwtPlot);
        lines.append(line);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refreshTree()
{
    QHash<QString, QVariant>::const_iterator iterator = auths.constBegin();
    while (iterator != auths.constEnd())
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->tw_auths);
        item->setText(0, iterator.key());
        item->setText(1, iterator.value().toString());
        ui->tw_auths->addTopLevelItem(item);
        ++iterator;
    }
}

void MainWindow::connectToServer()
{
    irc->connectTo(ui->le_server->text(), ui->le_channel->text(), ui->le_botname->text());
    ui->pb_connect->setEnabled(false);
}

void MainWindow::getInfo()
{
    reader->setID(ui->sb_battleID->value());
    battle->setID(ui->sb_battleID->value());
    reader->readData();
    xdata.clear();
    ydata.clear();
    curve->setData(xdata, ydata);
    for (int i =0; i < 6; i++)
    {
        QVector<double> x;
        QVector<double> y;
        lines[i]->setData(x, y);
    }
    onFinish = false;
    added = 0;
    ui->qwtPlot->replot();
}

void MainWindow::startPolling()
{
    reader->setID(ui->sb_battleID->value());
    battle->setID(ui->sb_battleID->value());
    //battle->setStartWall(ui->sb_startWall->value());
    //battle->setTime(ui->te_starttime->time());
}

void MainWindow::setTimeElapsed(QTime t)
{
    elapsed=t;
    if (onFinish)
        added++;
    if (t.hour() == 0 && t.minute() == 0 && t.second() == 0 && onFinish == false)
    {
        added = 0;
        onFinish = true;
    }
    ui->ac_currentTime->setTime(t);
    if (t.hour() > 2)
    {
        ui->pb_poll->setMaximum(300);
        ui->pb_poll->setValue((t.minute() % 5) * 60 + t.second());
    }
    else if ((t.hour() < 3) && (t.hour() > 0))
    {
        ui->pb_poll->setMaximum(60);
        ui->pb_poll->setValue(t.second());
    }
    else
    {
        ui->pb_poll->setMaximum(30);
        ui->pb_poll->setValue(t.second() % 30);
    }
}

void MainWindow::setWall(int w, bool)
{
    ui->sb_currentWall->setValue(w);
    int e = 0;
    if (onFinish)
        e = added;
    else
        e = elapsed.secsTo(QTime(0,0,0,0));
    xdata.append((double)e);
    ydata.append((double)w);
    curve->setData(xdata, ydata);
    for (int i =0; i < 6; i++)
    {
        QVector<double> x;
        x.append(xdata.first());
        x.append(xdata.last());
        QVector<double> y;
        y.append(levels[i]);
        y.append(levels[i]);
        lines[i]->setData(x, y);
    }
    ui->qwtPlot->replot();
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason r)
{
    if (r == QSystemTrayIcon::Trigger)
    {
        this->setVisible(!this->isVisible());
    }
}

void MainWindow::appQuit()
{
    settings->setValue("server", ui->le_server->text());
    settings->setValue("channel", ui->le_channel->text());
    settings->setValue("botname", ui->le_botname->text());
    settings->setValue("auths", auths);
    settings->sync();
    QCoreApplication::exit(0);
}

void MainWindow::setInitialWall(int w,QTime)
{
    int s = w / 5;
    for (int i= 0; i < 6; i++)
    {
        levels[i] = (double) (w - i*s);
    }
}

void MainWindow::auth(QString id, QString nick, QString pass)
{
    qDebug() << id << nick << pass << "";
    if(auths.contains(nick))
    {
        qDebug() << "ok" << auths.value(nick, "").toString() << "";
        if(auths.value(nick, "").toString() == pass.trimmed())
        {
            qDebug() << "authed";
            authed.insert(id, nick);
            irc->write("user " + nick + " auth OK");
        }
    }
}

void MainWindow::wall(QString id, QString num)
{
    if (authed.contains(id))
    {
        qDebug() << "wall" << num.trimmed() << "";
    ui->sb_battleID->setValue(num.trimmed().toInt());
    getInfo();
    }
}

void MainWindow::addAuth()
{
    if(!ui->le_pass->text().isEmpty() && !ui->le_user->text().isEmpty())
        if(!auths.contains(ui->le_user->text()))
        {
            auths.insert(ui->le_user->text(), ui->le_pass->text());
            refreshTree();
        }
}

void MainWindow::delAuth()
{
    QTreeWidgetItem * item = ui->tw_auths->currentItem();
    if(item)
        if(auths.contains(item->text(0)))
        {
            auths.remove(item->text(0));
            refreshTree();
        }
}
