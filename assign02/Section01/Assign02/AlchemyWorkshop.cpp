#include "header/AlchemyWorkshop.h"
#include <iostream>
using namespace std;

AlchemyWorkshop::AlchemyWorkshop()
{
    recipes["HP포션"] = PotionRecipe("HP포션", "허브", "맑은물");
    recipes["MP포션"] = PotionRecipe("MP포션", "마나초", "맑은물");
    recipes["스태미나포션"] = PotionRecipe("스태미나포션", "허브", "베리");
}

void AlchemyWorkshop::runWorkshop()
{
    bool isQuit = false;
    while (!isQuit) {
        cout << "\n=== 포션 제작소 ===\n";
        cout << "1. 전체 레시피 보기\n";
        cout << "2. 포션 이름으로 검색\n";
        cout << "3. 재료로 검색\n";
        cout << "0. 돌아가기\n";
        cout << "선택: ";

        int choice;
        cin >> choice;

        string input;
        switch (choice) {
        case 1:
            showAllRecipes();
            break;
        case 2:
            cout << "검색할 포션 이름: ";
            cin >> input;
            searchByName(input);
            break;
        case 3:
            cout << "검색할 재료: ";
            cin >> input;
            searchByIngredient(input);
            break;
        case 0:
            isQuit = true;
            break;
        default:
            cout << "잘못된 입력입니다.\n";
            break;
        }
    }
}

//전체 레시피
void AlchemyWorkshop::showAllRecipes()
{
    cout << "\n전체 레시피\n";
    // TODO: recipes 맵을 순회하면서 모든 레시피의 printRecipe()를 호출하세요.
    // 힌트: 전통적 방식(for (const auto& pair : recipes)) 이나 
    //       최신 방식(for (auto const& [key, recipe] : recipes)) 중 편한 것을 써보세요.
    
    // [여기에 코드를 직접 작성해 보세요!]
    
    for (const auto& pair: recipes)
    {
       pair.second.printRecipe();
    }
}

//이름으로 검색
void AlchemyWorkshop::searchByName(string name)
{
    if (recipes.find(name) != recipes.end())
    {
        recipes[name].printRecipe();
    }
    else
    {
        cout<<"찾을 수 없습니다." << endl;
    }
}

//재료로 검색
//first key값 second value값
void AlchemyWorkshop::searchByIngredient(string ingredient)
{
    int count = 0;
    // 힌트: 재료는 Key가 아니기 때문에, 결국 맵 전체를 한 번 순회해야 합니다.
    // 전체를 돌면서 각 레시피의 ingredient1 이나 ingredient2 가 검색어와 일치하는지 비교하세요.
    
    // [여기에 전체 순회 for문과 if 조건문을 직접 조합해 보세요!]
    // 힌트: pair를 사용한다면 pair.second.ingredient1 처럼 꺼내야 합니다.
    for (auto it = recipes.begin(); it != recipes.end(); ++it)
    {
        if (it->second.ingredients1 == ingredient || it->second.ingredients2 == ingredient)
        {
            it->second.printRecipe();
            count++;
        }
    }
    
    
    if (count > 0)
    {
        cout << "총"<<count<<"개의 레시피를 찾았습니다." << endl;
    }else
    {
        cout << "찾을 수 없습니다." << endl;
    }
    
}
