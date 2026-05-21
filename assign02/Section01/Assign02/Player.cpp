#include "header/Player.h"
#include <iostream>
#include <string>
using namespace std;


Player::Player(string n, int h, int m, int p, int d) : name(n), hp(h), mp(m), power(p), defence(d)
{
    level = 1;
    job = "초보자";
}

void Player::printPlayerstats()
{
    std::cout << "\n[" << name << " (" << job << ") 의 능력치]\n";
    std::cout << "Lv: " << level << " | HP: " << hp << " | MP: " << mp << "\n";
    std::cout << "공격력: " << power << " | 방어력: " << defence << "\n";
}