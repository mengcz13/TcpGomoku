#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QString>
#include <QStringList>
#include <QtEndian>
#include <QByteArray>
#include <QRegExp>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include "gamearea.h"

#include "setupserverdialog.h"

#include <unistd.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QTcpServer* server;
    QTcpSocket* socket;

    bool if_ready;//是否已经准备好开始/退出/悔棋
    bool if_oppo_ready;//对方是否准备好
    int lasting_withdraw;//连续悔棋次数
    int oppo_lasting_withdraw;//对方连续悔棋次数
    bool tempstate;//提出悔棋时的状态(able)

    QString load_file_name;//存档文件名

    QTimer* refresher;//每0.1s刷新一次连接状态和活动状态

public slots:
    //Server部分
    void setupcon();//Server建立连接
    void acceptConnection();//分配socket

    //共用部分
    void analyseinfo();//解析报文
    void sendChat();//聊天
    void sendMsg(QString);//发送报文

    void start();//开始游戏
    void sendStoneMsg(int,int,int);//发送落子信息
    void sendWinMsg();//发送获胜信息
    void quit();//退出比赛
    void refresh_time(QTime,QTime,QTime,QTime);//刷新时间信息
    void withdraw();//提出悔棋

    void save();//保存当前棋局
    void load();//读取

    void refresh_state();

    //Client部分
    void connectServer();
    void after_connected();
};

#endif // MAINWINDOW_H
