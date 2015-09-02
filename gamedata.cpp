#include "gamedata.h"

GameData::GameData()
{
    for (int i=0;i<SIZE;++i)
        for (int j=0;j<SIZE;++j)
            area[i][j]=-1;
}

bool GameData::set_stone(int x, int y, int player){
    if (area[x][y]!=-1)
        return false;
    area[x][y]=player;
    Step latest;
    latest.x=x;
    latest.y=y;
    latest.player=player;
    steps.push(latest);
    return true;
}

bool GameData::rev_stone(int player){
//    if (steps.size()<2||((n==2)&&(steps.size()<4)))
//        return false;
//    for (int i=n;i>0;i--){
//        area[steps.top().x][steps.top().y]=-1;
//        steps.pop();
//        area[steps.top().x][steps.top().y]=-1;
//        steps.pop();
//    }
//    return true;
    if (steps.empty())
        return false;
    else if (steps.top().player==player){
        area[steps.top().x][steps.top().y]=-1;
        steps.pop();
        return true;
    }
    else{
        area[steps.top().x][steps.top().y]=-1;
        steps.pop();
        if (steps.empty())
            return false;
        if (steps.top().player==player){
            area[steps.top().x][steps.top().y]=-1;
            steps.pop();
        }
        return true;
    }
}

int GameData::get_stone(int x, int y){
    return area[x][y];
}

int GameData::judge(){
    int last_num=0, last_player=-1;
    //检查横向
    for (int j=0;j<SIZE;++j){
        for (int i=0;i<SIZE;++i){
            get_win_stone(last_num,last_player,i,j);
            if (last_num==NUM_TO_WIN)
                return last_player;
        }
        last_num=0;
        last_player=-1;
    }
    //检查纵向
    for (int i=0;i<SIZE;++i){
        for (int j=0;j<SIZE;++j){
            get_win_stone(last_num,last_player,i,j);
            if (last_num==NUM_TO_WIN)
                return last_player;
        }
        last_num=0;
        last_player=-1;
    }
    //检查左上-右下
    for (int sec=0;sec<2*SIZE-1;++sec){
        if (sec<SIZE){
            int i=0; int j=sec;
            while (j>=0){
                get_win_stone(last_num,last_player,i,j);
                if (last_num==NUM_TO_WIN)
                    return last_player;
                i++;
                j--;
            }
            last_num=0;
            last_player=-1;
        }
        else{
            int i=sec-SIZE+1; int j=SIZE-1;
            while (j>=sec-SIZE+1){
                get_win_stone(last_num,last_player,i,j);
                if (last_num==NUM_TO_WIN)
                    return last_player;
                i++;
                j--;
            }
            last_num=0;
            last_player=-1;
        }
    }
    //检查左下-右上
    for (int sec=0;sec<2*SIZE-1;++sec){
        if (sec<SIZE){
            int i=0; int j=sec;
            while (j<SIZE){
                get_win_stone(last_num,last_player,i,j);
                if (last_num==NUM_TO_WIN)
                    return last_player;
                i++;
                j++;
            }
            last_num=0;
            last_player=-1;
        }
        else{
            int i=sec-SIZE+1; int j=0;
            while (i<SIZE){
                get_win_stone(last_num,last_player,i,j);
                if (last_num==NUM_TO_WIN)
                    return last_player;
                i++;
                j++;
            }
            last_num=0;
            last_player=-1;
        }
    }
    return last_player;
}

//记录累积棋子数
void GameData::get_win_stone(int &last_num, int &last_player, int i, int j){
    if (area[i][j]==-1){
        last_num=0;
        last_player=-1;
    }
    else{
        if (area[i][j]==last_player){
            last_num++;
        }
        else{
            last_num=1;
            last_player=area[i][j];
        }
    }
}

void GameData::reset(){
    for (int i=0;i<SIZE;++i)
        for (int j=0;j<SIZE;++j)
            area[i][j]=-1;
    while (!steps.empty())
        steps.pop();
}
