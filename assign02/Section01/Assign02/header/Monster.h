#pragma once
#include<string>

#include "Player.h"
using namespace std;

class Monster
{
    private:
    string name;
    int hp, power, defence;
    string dropItemName;
    int dropItemPrice;
    
    
    public:
    Monster(string n, int h, int p, int d, string dName, int dPrice);
    
    string getName(){return name;}
    string getdropItemName(){return dropItemName;}
    
    void attack(Player* player);
 
    
    int getHp(){return hp;}
    int getPower(){return power;}
    int getDefence(){return defence;}
    void setHp(int h){hp=h;}

};