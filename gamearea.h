#ifndef GAMEAREA_H
#define GAMEAREA_H

#include <QWidget>
#include <QPainter>
#include "gamedata.h"
#include <QMouseEvent>
#include <QRect>
#include <QTimer>
#include <QTime>

#define MAX_TIME 20
#define TIME_PIECE 100//ms

namespace Ui {
class GameArea;
}

class GameArea : public QWidget
{
    Q_OBJECT

friend class MainWindow;

public:
    explicit GameArea(QWidget *parent = 0);
    ~GameArea();
    void reset();

public slots:
    void record_time();

private:
    Ui::GameArea *ui;
    GameData gamedata;
    int W;
    int inteval;
    int border;
    bool able;//表示是否可以操作
    int player;//表示当前玩家，0为白棋，1为黑棋

    QTime remain_time;//本局剩余时间
    QTime my_total_time;//我方走棋总时间
    QTime oppo_total_time;//对方总时间
    QTime total_time;//游戏总时间
    QTimer* timer;

protected:
    void paintEvent(QPaintEvent *ev);
    void mousePressEvent(QMouseEvent* ev);
    void mouseReleaseEvent(QMouseEvent *ev);

signals:
    stone_set(int,int,int);
    win_signal();
    timeout(QTime,QTime,QTime,QTime);
};

#endif // GAMEAREA_H
