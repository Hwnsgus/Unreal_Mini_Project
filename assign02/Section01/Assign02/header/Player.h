#pragma once

#include <iostream>
#include <string>
using namespace std;

class Player
{
    protected:
        string name;
        string job;
        int level;
        int hp, mp, power, defence;

    public:
        Player(string n, int h, int m, int p, int d);
    
        virtual ~Player(){}

        virtual void attack() = 0;
    
        void printPlayerstats();
    
        virtual string getName(){return name;}
        string dropItem(){return dropItem();}
    
        int getHp(){return hp;}
        int getPower(){return power;}
        int getDefence(){return defence;}
    
        void setHp(int h){hp=h;}
  
};
