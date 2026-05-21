// Game.h
#pragma once
#include <map>
#include <string>
#include "Item.h"
#include "Player.h"
using namespace std;

class Game {
    
private:
    map<string, Item> inventory;
    
    
public:
    // 게임을 실행하는 메인 로직 함수
    Player* run();
    
    
    
    // 요구사항에 명시된 상태 출력 함수
    void printStatus(std::string name, int stat[]);
    
    void battle(Player* player);
    void MainMenu(Player* player);
    void showInventory();
};
