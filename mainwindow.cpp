#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    if_ready(false),
    if_oppo_ready(false),
    lasting_withdraw(0),
    refresher(new QTimer)
{
    ui->setupUi(this);
    refresher->setInterval(100);
    connect(refresher,SIGNAL(timeout()),this,SLOT(refresh_state()));

    connect(ui->setupButton,SIGNAL(clicked(bool)),this,SLOT(setupcon()));
    connect(ui->connectpushButton_2,SIGNAL(clicked(bool)),this,SLOT(connectServer()));
    connect(ui->sendMsgButton,SIGNAL(clicked(bool)),this,SLOT(sendChat()));
    connect(ui->startButton,SIGNAL(clicked(bool)),this,SLOT(start()));
    connect(ui->gameArea,SIGNAL(stone_set(int,int,int)),this,SLOT(sendStoneMsg(int,int,int)));
    connect(ui->gameArea,SIGNAL(win_signal()),this,SLOT(sendWinMsg()));
    connect(ui->quitButton,SIGNAL(clicked(bool)),this,SLOT(quit()));
    connect(ui->gameArea,SIGNAL(timeout(QTime,QTime,QTime,QTime)),this,SLOT(refresh_time(QTime,QTime,QTime,QTime)));
    connect(ui->withdrawButton,SIGNAL(clicked(bool)),this,SLOT(withdraw()));
    connect(ui->saveButton,SIGNAL(clicked(bool)),this,SLOT(save()));
    connect(ui->loadButton,SIGNAL(clicked(bool)),this,SLOT(load()));
    ui->quitButton->hide();
    ui->withdrawButton->hide();
    ui->saveButton->hide();
    ui->remaintimeprogressBar->hide();

    ui->startButton->setEnabled(false);
    ui->loadButton->setEnabled(false);
    ui->sendMsgButton->setEnabled(false);


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupcon(){
    quint16 port;
    setupServerDialog dia;
    if (dia.exec()==QDialog::Accepted){
        port = dia.get_port();
        server = new QTcpServer;
        server->listen(QHostAddress::Any,port);
        connect(server,SIGNAL(newConnection()),this,SLOT(acceptConnection()));
        if (server->isListening()){
            ui->connectpushButton_2->setEnabled(false);
            ui->textBrowser->append(QString("[SystemInfo]Server listening is established, port: %1!").arg(port));
        }
    }
}

void MainWindow::acceptConnection(){
    socket = server->nextPendingConnection();
    this->refresher->start();
    if (socket->state()==QAbstractSocket::ConnectedState){
        ui->startButton->setEnabled(true);
        ui->sendMsgButton->setEnabled(true);
        ui->loadButton->setEnabled(true);
        ui->textBrowser->append(QString("[SystemInfo]New client is connected, IP:%1, port:%2, ready for game!").arg(socket->peerAddress().toString()).arg(socket->peerPort()));
        connect(socket,SIGNAL(readyRead()),this,SLOT(analyseinfo()));
    }
}

//解析报文
//约定报文内容：
//READY:发送方已准备好
//X,Y,P:发送方落子坐标X Y 玩家号P
//YOURTURN:发送方丧失操作权，接收方操作（用于超时情况）
//CHAT+:聊天内容
//WIN:发送方获胜

//QUIT:发送方申请退出
//QUITAGREE:发送方同意退出
//QUITNOAGREE:发送方拒绝退出

//WITHDRAW:发送方申请悔棋
//WITHDRAWAGREE:发送方同意悔棋
//NOWITHDRAW:发送方拒绝悔棋

//LOAD...:读取存档指令
//SAVE_YOUR_INFO:要求接收方提供lasting_withdraw信息以供存档
//SAVE_YOUR_INFO+INT:对方提供的lasting_withdraw信息
//PAUSE:暂停接收方时钟
//KEEPON:启动接收方时钟
void MainWindow::analyseinfo(){
    QString data;
    data = socket->readLine();
    while (!data.isEmpty())
    {
    data.remove('\n');
    //ui->textBrowser->append(data);//test
    if (QRegExp("^READY\+.*").exactMatch(data)){
        if_oppo_ready=true;
        int oppo_player=data.split(QString("+"))[1].toInt();
        if (oppo_player==0){
            ui->whiteradioButton_2->setChecked(false);
            ui->whiteradioButton_2->setCheckable(false);
            ui->textBrowser->append("[SystemInfo]Your opponent has chosen the white color!");
        }
        else if (oppo_player==1){
            ui->blackradioButton->setChecked(false);
            ui->blackradioButton->setCheckable(false);
            ui->textBrowser->append("[SystemInfo]Your opponent has chosen the black color!");
        }
        if (if_ready&&if_oppo_ready){
            ui->gameArea->reset();
            //ui->gameArea->able=true;
            //ui->gameArea->player=1;//先开始者先行，执黑
            if (ui->blackradioButton->isChecked()){
                ui->gameArea->player=1;
                ui->gameArea->able=true;
            }
            else if (ui->whiteradioButton_2->isChecked()){
                ui->gameArea->player=0;
                ui->gameArea->able=false;
            }
            ui->startButton->setEnabled(false);
            ui->loadButton->setEnabled(false);
            ui->quitButton->show();
            ui->withdrawButton->show();
            ui->saveButton->show();
            ui->remaintimeprogressBar->show();
            ui->gameArea->timer->start();
            if_ready=false;
            if_oppo_ready=false;
            ui->textBrowser->append("[SystemInfo]Game start!");
        }
    }
    else if (QRegExp("^[0-9]{1,2},[0-9]{1,2},[0-9]{1,2}$").exactMatch(data)){
        QStringList stoneinfo = data.split(QString(","));
        ui->gameArea->gamedata.set_stone(stoneinfo[0].toInt(),stoneinfo[1].toInt(),stoneinfo[2].toInt());
        ui->gameArea->update();
        ui->gameArea->able=true;
        ui->gameArea->remain_time.setHMS(0,0,MAX_TIME,0);
        ui->withdrawButton->setEnabled(true);
        ui->textBrowser->append(QString("[Your Opponent's Step](%1,%2)").arg(stoneinfo[0].toInt()).arg(stoneinfo[1].toInt()));
    }
    else if (data==QString("YOURTURN")){
        ui->gameArea->able=true;
        ui->gameArea->remain_time.setHMS(0,0,MAX_TIME,0);
    }
    else if (data==QString("WIN")){
        qDebug()<<"Lost";
        ui->gameArea->able=false;
        QMessageBox lose(QMessageBox::Information,QString("Sorry"),QString("You have lost!"),QMessageBox::Ok);
        ui->gameArea->able=false;
        ui->startButton->setEnabled(true);
        ui->loadButton->setEnabled(true);
        ui->blackradioButton->setCheckable(true);
        ui->whiteradioButton_2->setCheckable(true);
        if_ready=false;
        if_oppo_ready=false;
        ui->gameArea->timer->stop();
        ui->quitButton->hide();
        ui->withdrawButton->hide();
        //ui->saveButton->hide();
        lose.exec();
    }
    else if (QRegExp("^CHAT\+.*").exactMatch(data)){
        QString chatinfo = data.split(QString("+"))[1];
        ui->textBrowser->append(chatinfo);
    }
    else if (data==QString("QUIT")){
        ui->gameArea->timer->stop();
        tempstate=ui->gameArea->able;
        ui->gameArea->able=false;
        QMessageBox qb(QMessageBox::Warning,QString("Quit"),QString("Your opponent wants to quit, do you agree?"),QMessageBox::Yes|QMessageBox::No);
        if (qb.exec()==QMessageBox::Yes){
            sendMsg(QString("QUITAGREE"));
            ui->gameArea->able=false;
            QMessageBox quit(QMessageBox::Information,QString("Quit"),QString("You quit!"));
            ui->startButton->setEnabled(true);
            ui->loadButton->setEnabled(true);
            ui->blackradioButton->setCheckable(true);
            ui->whiteradioButton_2->setCheckable(true);
            if_ready=false;
            if_oppo_ready=false;
            ui->gameArea->timer->stop();
            ui->quitButton->hide();
            ui->withdrawButton->hide();
            //ui->saveButton->hide();
            quit.exec();
        }
        else{
            sendMsg(QString("QUITNOAGREE"));
            ui->gameArea->timer->start();
            ui->gameArea->able=tempstate;
        }
    }
    else if (data==QString("QUITAGREE")){
        ui->gameArea->able=false;
        QMessageBox quit(QMessageBox::Information,QString("Quit"),QString("You quit!"));
        ui->blackradioButton->setCheckable(true);
        ui->whiteradioButton_2->setCheckable(true);
        if_ready=false;
        if_oppo_ready=false;
        ui->gameArea->timer->stop();
        ui->quitButton->hide();
        ui->withdrawButton->hide();
        //ui->saveButton->hide();
        ui->startButton->setEnabled(true);
        ui->loadButton->setEnabled(true);
        quit.exec();
    }
    else if (data==QString("QUITNOAGREE")){
        QMessageBox quit(QMessageBox::Information,QString("Quit"),QString("You are not allowed to quit!"));
        ui->gameArea->timer->start();
        ui->gameArea->able=tempstate;
        quit.exec();
    }
    else if (data==QString("WITHDRAW")){
        ui->gameArea->timer->stop();
            bool temp=ui->gameArea->able;
            ui->gameArea->able=false;
            QMessageBox msb(QMessageBox::Information,QString("Do You Agree"),QString("Your opponent wants to withdraw!"),QMessageBox::Yes|QMessageBox::No);
            if (msb.exec()==QMessageBox::Yes){
                //悔棋...
                ui->gameArea->gamedata.rev_stone(1-ui->gameArea->player);
                ui->gameArea->update();
                ui->gameArea->able=false;
                ui->gameArea->remain_time.setHMS(0,0,MAX_TIME,0);
                sendMsg("WITHDRAWAGREE");
            }
            else{
                ui->gameArea->able=temp;
                sendMsg("NOWITHDRAW");
            }
            ui->gameArea->timer->start();
    }
    else if (data==QString("WITHDRAWAGREE")){
        ui->gameArea->timer->start();
        QMessageBox msb(QMessageBox::Information,QString("Agree"),QString("You can withdraw now!"),QMessageBox::Ok);
        //悔棋...
        ui->gameArea->gamedata.rev_stone(ui->gameArea->player);
        ui->gameArea->update();
        lasting_withdraw++;
        ui->gameArea->able=true;
        ui->gameArea->remain_time.setHMS(0,0,MAX_TIME,0);
        if (lasting_withdraw==2)
            ui->withdrawButton->setEnabled(false);
        msb.exec();
    }
    else if (data==QString("NOWITHDRAW")){
        QMessageBox wd(QMessageBox::Information,QString("Withdraw"),QString("You are not allowed to withdraw!"));
        ui->gameArea->timer->start();
        ui->gameArea->able=tempstate;
        wd.exec();
    }
    else if (QRegExp("^LOAD.*").exactMatch(data)){
        QStringList arglist=data.split("+");
        if (arglist[1]==QString("START")){
            ui->gameArea->reset();
        }
        else if (arglist[1]==QString("TOTALTIME")){
            ui->gameArea->total_time=QTime::fromString(arglist[2]);
        }
        else if (arglist[1]==QString("OPPOTOTALTIME")){
            ui->gameArea->oppo_total_time=QTime::fromString(arglist[2]);
        }
        else if (arglist[1]==QString("REMAINTIME")){
            ui->gameArea->remain_time=QTime::fromString(arglist[2]);
        }
        else if (arglist[1]==QString("MYTOTALTIME")){
            ui->gameArea->my_total_time=QTime::fromString(arglist[2]);
        }
        else if (arglist[1]==QString("PLAYER")){
            ui->gameArea->player=arglist[2].toInt();
        }
        else if (arglist[1]==QString("ABLE")){
            int ab=arglist[2].toInt();
            if (ab==0)
                ui->gameArea->able=false;
            else
                ui->gameArea->able=true;
        }
        else if (arglist[1]==QString("OPLW")){
            this->oppo_lasting_withdraw=arglist[2].toInt();
        }
        else if (arglist[1]==QString("LW")){
            this->lasting_withdraw=arglist[2].toInt();
        }
        else if (arglist[1]==QString("SETSTONE")){
            ui->gameArea->gamedata.set_stone(arglist[2].toInt(),arglist[3].toInt(),arglist[4].toInt());
        }
        else if (arglist[1]==QString("END")){
            ui->gameArea->timer->start();
            if (ui->gameArea->player==0){
                ui->whiteradioButton_2->setChecked(true);
                ui->blackradioButton->setChecked(false);
                ui->blackradioButton->setCheckable(false);
            }
            else{
                ui->blackradioButton->setChecked(true);
                ui->whiteradioButton_2->setChecked(false);
                ui->whiteradioButton_2->setCheckable(false);
            }
            ui->quitButton->show();
            ui->withdrawButton->show();
            ui->saveButton->show();
            ui->remaintimeprogressBar->show();
            ui->startButton->setEnabled(false);
            ui->loadButton->setEnabled(false);
        }
        else if (arglist[1]==QString("FILE")){
            QMessageBox lm(QMessageBox::Information,QString("Load"),QString("Do you want to load the file %1").arg(arglist[2]),QMessageBox::Yes|QMessageBox::No);
            if (lm.exec()==QMessageBox::Yes){
                ui->gameArea->reset();
                sendMsg(QString("LOAD+AGREE"));
            }
        }
        else if (arglist[1]==QString("AGREE")){
            sendMsg(QString("LOAD+START"));
            QFile loadf(load_file_name);
            loadf.open(QIODevice::Text|QIODevice::ReadOnly);
            QTextStream la(&loadf);
            ui->gameArea->reset();
            QString temps;
            la>>temps;
            ui->gameArea->total_time=QTime::fromString(temps);
            sendMsg(QString("LOAD+TOTALTIME+%1").arg(temps));
            temps.clear();
            la>>temps;
            ui->gameArea->my_total_time=QTime::fromString(temps);
            sendMsg(QString("LOAD+OPPOTOTALTIME+%1").arg(temps));
            temps.clear();
            la>>temps;
            ui->gameArea->remain_time=QTime::fromString(temps);
            sendMsg(QString("LOAD+REMAINTIME+%1").arg(temps));
            temps.clear();
            la>>temps;
            ui->gameArea->oppo_total_time=QTime::fromString(temps);
            sendMsg(QString("LOAD+MYTOTALTIME+%1").arg(temps));
            temps.clear();
            la>>ui->gameArea->player;
            sendMsg(QString("LOAD+PLAYER+%1").arg(1-ui->gameArea->player));
            int ab=0;
            la>>ab;
            if (ab==0)
                ui->gameArea->able=false;
            else
                ui->gameArea->able=true;
            sendMsg(QString("LOAD+ABLE+%1").arg(1-ab));
            la>>lasting_withdraw;
            sendMsg(QString("LOAD+OPLW+%1").arg(lasting_withdraw));
            la>>oppo_lasting_withdraw;
            sendMsg(QString("LOAD+LW+%1").arg(oppo_lasting_withdraw));
            int stepnum=0;
            la>>stepnum;
            std::stack<Step> tempsteps;
            for (int i=0;i<stepnum;++i){
                Step tempstep;
                la>>tempstep.x>>tempstep.y>>tempstep.player;
                tempsteps.push(tempstep);
            }
            while (!tempsteps.empty()){
                ui->gameArea->gamedata.set_stone(tempsteps.top().x,tempsteps.top().y,tempsteps.top().player);
                sendMsg(QString("LOAD+SETSTONE+%1+%2+%3").arg(tempsteps.top().x).arg(tempsteps.top().y).arg(tempsteps.top().player));
                tempsteps.pop();
            }
            sendMsg(QString("LOAD+END"));
            ui->gameArea->timer->start();
            if (ui->gameArea->player==0){
                ui->whiteradioButton_2->setChecked(true);
                ui->blackradioButton->setChecked(false);
                ui->blackradioButton->setCheckable(false);
            }
            else{
                ui->blackradioButton->setChecked(true);
                ui->whiteradioButton_2->setChecked(false);
                ui->whiteradioButton_2->setCheckable(false);
            }
            ui->quitButton->show();
            ui->withdrawButton->show();
            ui->saveButton->show();
            ui->remaintimeprogressBar->show();
            ui->startButton->setEnabled(false);
            ui->loadButton->setEnabled(false);
            QMessageBox subox(QMessageBox::Information,QString("Loaded"),QString("Successfully loaded file %1").arg(load_file_name),QMessageBox::Ok);
            subox.exec();
        }
    }
    else if (QRegExp("^SAVE_YOUR_INFO.*").exactMatch(data)){
        QStringList li=data.split(QString("+"));
        if (li.size()==1){
            sendMsg(QString("SAVE_YOUR_INFO+%1").arg(lasting_withdraw));
        }
        else{
            oppo_lasting_withdraw=li[1].toInt();
        }
    }
    else if (data==QString("PAUSE")){
        ui->gameArea->timer->stop();
    }
    else if (data==QString("KEEPON")){
        ui->gameArea->timer->start();
    }
    data.clear();
    data=socket->readLine();
    }
}

void MainWindow::sendChat(){
    if (socket->isValid())
    {
    QString chatdata = QString("CHAT+")+(QString("[Player%1]").arg(ui->gameArea->player))+ui->lineEdit->text();
    sendMsg(chatdata);
    ui->textBrowser->append(QString("[You]")+ui->lineEdit->text());
    ui->lineEdit->clear();
    }
}

//发送报文，结尾自动加\n以方便逐行读取
void MainWindow::sendMsg(QString text){
    QByteArray array;
    array.clear();
    array.append(text+QString("\n"));
    socket->write(array.data());
}

void MainWindow::connectServer(){
    setupServerDialog dia;
    if (dia.exec()==QDialog::Accepted){
        socket=new QTcpSocket;
        socket->connectToHost(dia.get_ip(),dia.get_port());
        this->refresher->start();
        connect(socket,SIGNAL(connected()),this,SLOT(after_connected()));
    }
}

void MainWindow::after_connected(){
    connect(socket,SIGNAL(readyRead()),this,SLOT(analyseinfo()));
    ui->setupButton->setEnabled(false);
    ui->startButton->setEnabled(true);
    ui->loadButton->setEnabled(true);
    ui->sendMsgButton->setEnabled(true);
    ui->textBrowser->append(QString("[SystemInfo]Connected to server at IP:%1, Port:%2, ready for game!").arg(socket->peerAddress().toString()).arg(socket->peerPort()));
}

void MainWindow::start(){
    if (ui->blackradioButton->isChecked()||ui->whiteradioButton_2->isChecked())
    {
    if_ready=true;
    if (ui->blackradioButton->isChecked()){
        sendMsg(QString("READY+1"));
        ui->textBrowser->append("[SystemInfo]You have chosen the black color!");
        ui->gameArea->player=1;
    }
    else if (ui->whiteradioButton_2->isChecked()){
        sendMsg(QString("READY+0"));
        ui->textBrowser->append("[SystemInfo]You have chosen the white color!");
        ui->gameArea->player=0;
    }
    if (if_ready&&if_oppo_ready){
        ui->gameArea->reset();
        //ui->gameArea->able=false;
        //ui->gameArea->player=0;//后开始者后行，执白
        if (ui->blackradioButton->isChecked()){
            ui->gameArea->player=1;
            ui->gameArea->able=true;
        }
        else if (ui->whiteradioButton_2->isChecked()){
            ui->gameArea->player=0;
            ui->gameArea->able=false;
        }
        ui->startButton->setEnabled(false);
        ui->loadButton->setEnabled(false);
        ui->quitButton->show();
        ui->withdrawButton->show();
        ui->remaintimeprogressBar->show();
        ui->saveButton->show();
        ui->gameArea->timer->start();
        if_ready=false;
        if_oppo_ready=false;
        ui->textBrowser->append("[SystemInfo]Game start!");
    }
    }
}

void MainWindow::sendStoneMsg(int x, int y, int p){
    sendMsg(QString("%1,%2,%3").arg(x).arg(y).arg(p));
    ui->textBrowser->append(QString("[Your Step](%1,%2)").arg(x).arg(y));
    lasting_withdraw=0;
    //校准时间
    sendMsg(QString("LOAD+TOTALTIME+%1").arg(ui->gameArea->total_time.toString()));
    sendMsg(QString("LOAD+OPPOTOTALTIME+%1").arg(ui->gameArea->my_total_time.toString()));
    //sendMsg(QString("LOAD+REMAINTIME+%1").arg(ui->gameArea->remain_time.toString()));
    sendMsg(QString("LOAD+MYTOTALTIME+%1").arg(ui->gameArea->oppo_total_time.toString()));
}

void MainWindow::sendWinMsg(){
    sendMsg(QString("WIN"));
    ui->gameArea->able=false;
    QMessageBox win(QMessageBox::Information,QString("Congratulations"),QString("You win!"),QMessageBox::Ok);
    ui->gameArea->able=false;
    ui->blackradioButton->setCheckable(true);
    ui->whiteradioButton_2->setCheckable(true);
    if_ready=false;
    if_oppo_ready=false;
    ui->gameArea->timer->stop();
    ui->quitButton->hide();
    ui->withdrawButton->hide();
    //ui->saveButton->hide();
    ui->startButton->setEnabled(true);
    ui->loadButton->setEnabled(true);
    win.exec();
}

void MainWindow::quit(){
    ui->gameArea->timer->stop();
    tempstate=ui->gameArea->able;
    ui->gameArea->able=false;
    sendMsg(QString("QUIT"));
}

void MainWindow::refresh_time(QTime t,QTime r,QTime m,QTime o){
    ui->Totaltimelabel->setText(QString("Total:")+t.toString(QString("HH:mm:ss")));
    ui->remaintime_label_3->setText(QString("Remaining:")+r.toString(QString("HH:mm:ss")));
    ui->remaintimeprogressBar->setValue(r.second());
    //qDebug()<<QString("Remaining:")+r.toString(QString("HH:mm:ss"));
    ui->my_total_timelabel_2->setText(QString("Your total:")+m.toString(QString("HH:mm:ss")));
    ui->oppotimelabel->setText(QString("The other total:")+o.toString(QString("HH:mm:ss")));
    if (ui->gameArea->able&&r==QTime(0,0,0,0)){
        sendMsg(QString("YOURTURN"));
        lasting_withdraw=0;
        ui->gameArea->able=false;
        ui->gameArea->remain_time.setHMS(0,0,MAX_TIME,0);
    }
}

void MainWindow::withdraw(){
    ui->gameArea->timer->stop();
    tempstate=ui->gameArea->able;
    sendMsg(QString("WITHDRAW"));
    ui->gameArea->able=false;
}

//存储当前棋局，文件结构：
//总时间
//我方总时间
//当前活动方剩余时间
//对方总时间
//本机玩家号（棋子颜色，0白1黑）
//本机是否处于活动状态
//本机本局已悔棋次数
//对方本局已悔棋次数
//落子记录栈
void MainWindow::save(){
    sendMsg(QString("SAVE_YOUR_INFO"));
    QString savedname((QDate::currentDate().toString(Qt::ISODate))+QString("_")+(QTime::currentTime().toString(Qt::ISODate)));
    savedname = savedname.replace(QString("-"),QString("_"));
    savedname = savedname.replace(QString(":"),QString("_"))+QString(".txt");
    qDebug()<<savedname;
    QFileDialog dia(this,QString("Save"),QDir::currentPath(),QString("*.txt"));
    dia.setAcceptMode(QFileDialog::AcceptSave);
    dia.selectFile(savedname);
    if (dia.exec()==QFileDialog::Accepted){
        QFile savef(dia.selectedFiles()[0]);
        savef.open(QIODevice::Text|QIODevice::ReadWrite);
        QTextStream sa(&savef);
        sa<<ui->gameArea->total_time.toString()<<endl;
        sa<<ui->gameArea->my_total_time.toString()<<endl;
        sa<<ui->gameArea->remain_time.toString()<<endl;
        sa<<ui->gameArea->oppo_total_time.toString()<<endl;
        sa<<ui->gameArea->player<<endl;
        sa<<ui->gameArea->able<<endl;
        sa<<lasting_withdraw<<endl;
        sa<<oppo_lasting_withdraw<<endl;
        std::stack<Step> savesteps = ui->gameArea->gamedata.get_steps();
        sa<<savesteps.size()<<endl;
        while (!savesteps.empty()){
            sa<<savesteps.top().x<<" "<<savesteps.top().y<<" "<<savesteps.top().player<<endl;
            savesteps.pop();
        }
    }
}

void MainWindow::load(){
    QFileDialog dia(this,QString("Load"),QDir::currentPath(),QString("*.txt"));
    if (dia.exec()==QFileDialog::Accepted){
        load_file_name=dia.selectedFiles()[0];
        sendMsg(QString("LOAD+FILE+%1").arg(dia.selectedFiles()[0]));
    }
}

void MainWindow::refresh_state(){
    if (ui->gameArea->able){
        ui->whoseturnlabel_2->setText(QString("IT IS YOUR TURN NOW!"));
    }
    else{
        ui->whoseturnlabel_2->clear();
    }
    switch (this->socket->state()){
    case QAbstractSocket::UnconnectedState:
        ui->connectionstatelabel->setText("Unconnected");
        break;
    case QAbstractSocket::ConnectingState:
        ui->connectionstatelabel->setText("Connecting");
        break;
    case QAbstractSocket::ConnectedState:
        ui->connectionstatelabel->setText("Connected");
        break;
    default:
        ui->connectionstatelabel->clear();
        break;
    }
}
