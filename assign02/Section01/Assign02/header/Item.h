#pragma once
#include <iostream>
#include <string>
using namespace std;

struct Item
{
    string name;
    int price;
    int quantity;
    
    Item(string n = "", int p=0, int q=0) : name(n), price(p), quantity(q) {}
    
    void printInfo()const
    {
        cout <<"- " << name << " (" << price << "G)[보유 수량: " << quantity << "개]"<< endl;
    }
    
};
