#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    if_ready(false),
    if_oppo_ready(false),
    lasting_withdraw(0)
{
    ui->setupUi(this);
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
    ui->startButton->setEnabled(false);
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
        if (server->isListening())
            ui->connectpushButton_2->setEnabled(false);
    }
}

void MainWindow::acceptConnection(){
    socket = server->nextPendingConnection();
    if (socket->isValid()){
        ui->startButton->setEnabled(true);
        ui->sendMsgButton->setEnabled(true);
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
//QUIT:发送方申请/同意退出
//WITHDRAW:发送方申请/同意悔棋
//NOWITHDRAW:发送方拒绝悔棋
//LOAD...:读取存档指令（待续）
void MainWindow::analyseinfo(){
    QString data;
    data = socket->readAll();
    ui->textBrowser->append(data);//test
    if (QRegExp("^READY\+.*").exactMatch(data)){
        if_oppo_ready=true;
        int oppo_player=data.split(QString("+"))[1].toInt();
        if (oppo_player==0){
            ui->whiteradioButton_2->setChecked(false);
            ui->whiteradioButton_2->setCheckable(false);
        }
        else if (oppo_player==1){
            ui->blackradioButton->setChecked(false);
            ui->blackradioButton->setCheckable(false);
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
            ui->quitButton->show();
            ui->gameArea->timer->start();
            if_ready=false;
            if_oppo_ready=false;
        }
    }
    else if (QRegExp("^[0-9]{1,2},[0-9]{1,2},[0-9]{1,2}$").exactMatch(data)){
        QStringList stoneinfo = data.split(QString(","));
        ui->gameArea->gamedata.set_stone(stoneinfo[0].toInt(),stoneinfo[1].toInt(),stoneinfo[2].toInt());
        ui->gameArea->update();
        ui->gameArea->able=true;
        ui->gameArea->remain_time.setHMS(0,0,MAX_TIME,0);
        ui->withdrawButton->setEnabled(true);
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
        ui->quitButton->hide();
        ui->blackradioButton->setCheckable(true);
        ui->whiteradioButton_2->setCheckable(true);
        if_ready=false;
        if_oppo_ready=false;
        ui->gameArea->timer->stop();
        lose.exec();
    }
    else if (QRegExp("^CHAT\+.*").exactMatch(data)){
        QString chatinfo = data.split(QString("+"))[1];
        ui->textBrowser->append(chatinfo);
    }
    else if (data==QString("QUIT")){
        if_oppo_ready=true;
        if (if_ready&&if_oppo_ready){
            //存盘...
            ui->gameArea->able=false;
            QMessageBox quit(QMessageBox::Information,QString("Quit"),QString("You quit!"));
            ui->startButton->setEnabled(true);
            ui->quitButton->hide();
            ui->blackradioButton->setCheckable(true);
            ui->whiteradioButton_2->setCheckable(true);
            if_ready=false;
            if_oppo_ready=false;
            ui->gameArea->timer->stop();
            quit.exec();
        }
    }
    else if (data==QString("WITHDRAW")){
        if (if_ready){
            QMessageBox msb(QMessageBox::Information,QString("Agree"),QString("You can withdraw now!"),QMessageBox::Ok);
            msb.exec();
            //悔棋...
            ui->gameArea->gamedata.rev_stone(ui->gameArea->player);
            ui->gameArea->update();
            lasting_withdraw++;
            ui->gameArea->able=true;
            ui->gameArea->remain_time.setHMS(0,0,MAX_TIME,0);
            if_ready=false;
            if (lasting_withdraw==2)
                ui->withdrawButton->setEnabled(false);
        }
        else{
            bool temp=ui->gameArea->able;
            ui->gameArea->able=false;
            QMessageBox msb(QMessageBox::Information,QString("Do You Agree"),QString("Your opponent wants to withdraw!"),QMessageBox::Yes|QMessageBox::No);
            if (msb.exec()==QMessageBox::Yes){
                //悔棋...
                ui->gameArea->gamedata.rev_stone(1-ui->gameArea->player);
                ui->gameArea->update();
                ui->gameArea->able=false;
                sendMsg("WITHDRAW");
            }
            else{
                ui->gameArea->able=temp;
                sendMsg("NOWITHDRAW");
            }
        }
    }
    else if (data==QString("NOWITHDRAW")){
        ui->gameArea->able=this->tempstate;
    }
}

void MainWindow::sendChat(){
    if (socket->isValid())
    {
    QString chatdata = QString("CHAT+")+ui->lineEdit->text();
    sendMsg(chatdata);
    ui->textBrowser->append(ui->lineEdit->text());
    ui->lineEdit->clear();
    }
}

void MainWindow::sendMsg(QString text){
    QByteArray array;
    array.clear();
    array.append(text);
    socket->write(array.data());
}

void MainWindow::connectServer(){
    setupServerDialog dia;
    if (dia.exec()==QDialog::Accepted){
        socket=new QTcpSocket;
        socket->connectToHost(dia.get_ip(),dia.get_port());
        if (socket->isValid()){
            connect(socket,SIGNAL(readyRead()),this,SLOT(analyseinfo()));
            ui->setupButton->setEnabled(false);
            ui->startButton->setEnabled(true);
            ui->sendMsgButton->setEnabled(true);
        }
    }
}

void MainWindow::start(){
    if (ui->blackradioButton->isChecked()||ui->whiteradioButton_2->isChecked())
    {
    if_ready=true;
    if (ui->blackradioButton->isChecked()){
        sendMsg(QString("READY+1"));
        ui->gameArea->player=1;
    }
    else if (ui->whiteradioButton_2->isChecked()){
        sendMsg(QString("READY+0"));
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
        ui->quitButton->show();
        ui->gameArea->timer->start();
        if_ready=false;
        if_oppo_ready=false;
    }
    }
}

void MainWindow::sendStoneMsg(int x, int y, int p){
    sendMsg(QString("%1,%2,%3").arg(x).arg(y).arg(p));
    lasting_withdraw=0;
}

void MainWindow::sendWinMsg(){
    sendMsg(QString("WIN"));
    ui->gameArea->able=false;
    QMessageBox win(QMessageBox::Information,QString("Congratulations"),QString("You win!"),QMessageBox::Ok);
    ui->gameArea->able=false;
    ui->startButton->setEnabled(true);
    ui->quitButton->hide();
    ui->blackradioButton->setCheckable(true);
    ui->whiteradioButton_2->setCheckable(true);
    if_ready=false;
    if_oppo_ready=false;
    ui->gameArea->timer->stop();
    win.exec();
}

void MainWindow::quit(){
    if_ready=true;
    sendMsg(QString("QUIT"));
    if (if_ready&&if_oppo_ready){
        //存盘...
        ui->gameArea->able=false;
        QMessageBox quit(QMessageBox::Information,QString("Quit"),QString("You quit!"));
        ui->startButton->setEnabled(true);
        ui->quitButton->hide();
        ui->blackradioButton->setCheckable(true);
        ui->whiteradioButton_2->setCheckable(true);
        if_ready=false;
        if_oppo_ready=false;
        ui->gameArea->timer->stop();
        quit.exec();
    }
}

void MainWindow::refresh_time(QTime t,QTime r,QTime m,QTime o){
    ui->Totaltimelabel->setText(QString("Total:")+t.toString(QString("HH:mm:ss.zzz")));
    ui->remaintime_label_3->setText(QString("Remaining:")+r.toString(QString("HH:mm:ss.zzz")));
    //qDebug()<<QString("Remaining:")+r.toString(QString("HH:mm:ss.zzz"));
    ui->my_total_timelabel_2->setText(QString("Your total:")+m.toString(QString("HH:mm:ss.zzz")));
    ui->oppotimelabel->setText(QString("The other total:")+o.toString(QString("HH:mm:ss.zzz")));
    if (r==QTime(0,0,0,0)){
        sendMsg(QString("YOURTURN"));
        lasting_withdraw=0;
        ui->gameArea->able=false;
        ui->gameArea->remain_time.setHMS(0,0,MAX_TIME,0);
    }
}

void MainWindow::withdraw(){
    tempstate=ui->gameArea->able;
    if_ready=true;
    sendMsg(QString("WITHDRAW"));
    ui->gameArea->able=false;
}

void MainWindow::save(){
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
//        for (int i=0;i<SIZE;++i){
//            for (int j=0;j<SIZE;++j)
//                sa<<ui->gameArea->gamedata.get_stone(i,j)<<" ";
//            sa<<endl;
//        }
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
        QFile loadf(dia.selectedFiles()[0]);
        loadf.open(QIODevice::Text|QIODevice::ReadOnly);
        QTextStream la(&loadf);
        ui->gameArea->reset();
        QString temps;
        la>>temps;
        ui->gameArea->total_time=QTime::fromString(temps);
        temps.clear();
        la>>temps;
        ui->gameArea->my_total_time=QTime::fromString(temps);
        temps.clear();
        la>>temps;
        ui->gameArea->remain_time=QTime::fromString(temps);
        temps.clear();
        la>>temps;
        ui->gameArea->oppo_total_time=QTime::fromString(temps);
        temps.clear();
        la>>ui->gameArea->player;
        int ab=0;
        la>>ab;
        if (ab==0)
            ui->gameArea->able=false;
        else
            ui->gameArea->able=true;
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
            tempsteps.pop();
        }
    }
}
