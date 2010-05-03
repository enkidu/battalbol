#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include "irc.hpp"
#include "reader.hpp"
#include "battle.hpp"
#include "qwt_plot_curve.h"
#include <QSystemTrayIcon>

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    void setTimeElapsed(QTime);
    void setWall(int, bool);
private:
    Ui::MainWindow *ui;
    Irc *irc;
    Reader *reader;
    Battle *battle;
    QwtPlotCurve *curve;
    QList<QwtPlotCurve *> lines;
    QList<double> levels;
    QTime elapsed;
    bool onFinish;
    int added;
    QVector<double> xdata;
    QVector<double> ydata;
    QSystemTrayIcon *trayIcon;
    QHash<QString,QVariant> auths;
    QHash<QString,QString> authed;
    QSettings *settings;
    void refreshTree();
private slots:
    void connectToServer();
    void startPolling();
    void getInfo();
    void trayIconActivated(QSystemTrayIcon::ActivationReason);
    void appQuit();
    void setInitialWall(int, QTime);
    void auth(QString, QString, QString);
    void wall(QString, QString);
    void addAuth();
    void delAuth();
};

#endif // MAINWINDOW_H
