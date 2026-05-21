#pragma once
#include <iostream>
#include <string>
using namespace std;

struct PotionRecipe
{
    string potionName;
    string ingredients1;
    string ingredients2;
    
    PotionRecipe(string n = "", string i1 ="", string i2 = ""): potionName(n), ingredients1(i1), ingredients2(i2)
    {
        
    }
    
    void printRecipe() const
    {
        cout << "->" << potionName << "(" << ingredients1 << "x1" << ingredients2 << "x1)" <<endl;
    }
};
