#pragma once
#include <iostream>
using namespace std;

#include "Player.h"

class Warrior : public Player
{
    public: Warrior(string n, int h, int m, int p, int d):Player{n, h, m, p, d}
    {
        job = "전사";
        defence += 30;
    };
    void attack() override
    {
        cout << "전사" << name << "가 거대한 검을 휘두릅니다. (데미지:" << power <<") \n";
    };
};

class Magician : public Player
{
        public: Magician(string n, int h, int m, int p, int d):Player{n, h, m, p, d}
    {
        job = "마법사";
        mp += 30;
    };
    void attack() override
    {
        cout << "마법사" << name << "이(가) 주문을 외웁니다. (데미지:" << power <<") \n";
    };
};

class Thief : public Player
{
        public: Thief(string n, int h, int m, int p, int d):Player{n, h, m, p, d}
    {
        job = "도적";
        power += 30;
    };
    void attack() override
    {
        cout << "도적" << name << "이 조용히 접근합니다 (데미지:" << power <<") \n";
    };
};

class Archer : public Player
{
        public: Archer(string n, int h, int m, int p, int d):Player{n, h, m, p, d}
    {
        job = "궁수";
        hp += 30;
    };
    void attack() override
    {
        cout << "궁수" << name << "가 멀리서 난사합니다 (데미지:" << power <<") \n";
    };
};