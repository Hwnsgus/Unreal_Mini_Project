#pragma once
#include <map>
#include <string>
#include "PotionRecipe.h"
using namespace std;

class AlchemyWorkshop
{
    private:
    map<string, PotionRecipe> recipes;
    
    public:
    AlchemyWorkshop();
    void runWorkshop();
    
    void showAllRecipes();
    void searchByName(string name);
    void searchByIngredient(string ingredient);
    
};
