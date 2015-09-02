#ifndef GAMEDATA_H
#define GAMEDATA_H

#define SIZE 15
#define NUM_TO_WIN 5

#include <stack>

struct Step{
    int x;
    int y;
    int player;
};

class GameData
{
public:
    GameData();
    bool set_stone(int,int,int);//放置棋子，返回是否成功
    bool rev_stone(int player);//为玩家player撤回1步棋，返回是否成功
    int get_stone(int,int);//返回棋子信息
    int judge();//判断胜负，返回胜方
    int size(){return SIZE;}
    void reset();
    std::stack<Step> get_steps(){return steps;}
private:
    int area[SIZE][SIZE];
    std::stack<Step> steps;

protected:
    void get_win_stone(int&last_num, int&last_player, int i, int j);
};

#endif // GAMEDATA_H
