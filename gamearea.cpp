#include "gamearea.h"
#include "ui_gamearea.h"
#include <QDebug>

GameArea::GameArea(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GameArea),
    able(false),
    player(-1),
    remain_time(QTime(0,0,MAX_TIME,0)),
    total_time(QTime(0,0,0,0)),
    my_total_time(QTime(0,0,0,0)),
    oppo_total_time(QTime(0,0,0,0)),
    timer(new QTimer)
{
    ui->setupUi(this);
    border=10;
    qDebug()<<this->width();
    W=(this->width()-2*border);
    inteval = W/(gamedata.size()-1);

    timer->setInterval(TIME_PIECE);//计时单位0.1s
    connect(timer,SIGNAL(timeout()),this,SLOT(record_time()));

    update();
}

GameArea::~GameArea()
{
    delete ui;
    delete timer;
}

void GameArea::reset(){
    gamedata.reset();
    able=false;
    player=-1;
    remain_time.setHMS(0,0,MAX_TIME,0);
    my_total_time.setHMS(0,0,0);
    oppo_total_time.setHMS(0,0,0);
    total_time.setHMS(0,0,0);
    timer->stop();
    update();
}

void GameArea::paintEvent(QPaintEvent *ev){
    QPainter p(this);
    p.setWindow(-border,-border,W+2*border,W+2*border);
    p.setRenderHint(QPainter::Antialiasing);
    p.save();
    //Draw Grid
    p.setPen(QPen(QColor(Qt::black)));
    for (int i=0;i<gamedata.size();++i){
        p.drawLine(0,i*inteval,W,i*inteval);
        p.drawLine(i*inteval,0,i*inteval,W);
    }
    p.restore();

    //Draw stones
    p.save();
    for (int i=0;i<gamedata.size();++i)
        for (int j=0;j<gamedata.size();++j){
            if (gamedata.get_stone(i,j)==0){
                p.setBrush(QColor(Qt::white));
                p.drawEllipse(QPoint(i*inteval,j*inteval),border-1,border-1);
            }
            else if (gamedata.get_stone(i,j)==1){
                p.setBrush(QColor(Qt::black));
                p.drawEllipse(QPoint(i*inteval,j*inteval),border-1,border-1);
            }
        }
    p.restore();
}

void GameArea::mousePressEvent(QMouseEvent *ev){
    if (able&&ev->button()==Qt::LeftButton)
    {
    QPoint cp(ev->pos().x()-border,ev->pos().y()-border);
    QRect ava(border,border,W,W);
    if (ava.contains(ev->pos())){
        int x=(cp.x()/(inteval/2)+1)/2;
        int y=(cp.y()/(inteval/2)+1)/2;
        qDebug()<<x<<" "<<y;
        if (gamedata.get_stone(x,y)==-1)
            if_uncolored=true;
        else
            if_uncolored=false;
        if (gamedata.set_stone(x,y,player)){
            emit stone_set(x,y,player);
        }
    }
    update();
    }
}

void GameArea::mouseReleaseEvent(QMouseEvent *ev){
    if (able&&ev->button()==Qt::LeftButton)
    {
    QPoint cp(ev->pos().x()-border,ev->pos().y()-border);
    QRect ava(border,border,W,W);
    if (ava.contains(ev->pos()))
    {
    int res=gamedata.judge();
    qDebug()<<res;
    if (res==this->player){
        emit win_signal();
    }
    if (if_uncolored){
        this->able=false;
        this->remain_time.setHMS(0,0,MAX_TIME,0);
    }
    }
    }
}

void GameArea::record_time(){
    total_time = total_time.addMSecs(TIME_PIECE);
    remain_time = remain_time.addMSecs(-TIME_PIECE);
    if (able){
        my_total_time = my_total_time.addMSecs(TIME_PIECE);
    }
    else{
        oppo_total_time = oppo_total_time.addMSecs(TIME_PIECE);
    }
    emit timeout(total_time,remain_time,my_total_time,oppo_total_time);
}
