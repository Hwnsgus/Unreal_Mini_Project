#include "header/Monster.h"
#include "header/Item.h"
#include "header/Game.h"
#include <string>
#include <iostream>
using namespace std;

Monster::Monster(string n, int h, int p, int d, string dName, int dPrice) : name(n), hp(h), power(p), defence(d),
dropItemName(dName), dropItemPrice(dPrice)
{
}

void Monster::attack(Player* player)
{
    //데미지 공식
    int damage = power - player->getDefence();
    
    //데미지가 0 이하면 1로 고정
    //무효였다 등으로 표기해도 좋을 듯 후에
    if (damage < 0)
    {
        damage = 1;
    }
    
    cout << name << "의 공격" << endl;
    cout << player->getName()<<"에게" << damage << "데미지"<< endl;
    
    
    //플레이어 체력감소
    int currentHP = player->getHp();
    player->setHp(currentHP - damage);
    
    
};