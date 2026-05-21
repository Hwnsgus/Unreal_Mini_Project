#include "header/Game.h"

// main.cpp
int main() {
    
    Game game;
    Player* player = nullptr;
    
    player= game.run();
    if (player != nullptr)
    {
        // [ ] 생성된 캐릭터 정보 확인
        player->printPlayerstats();
        
        // [ ] 다형성을 이용한 공격 호출
        game.MainMenu(player);

        // [ ] 프로그램 종료 전 메모리 해제
        delete player;
        player = nullptr;
    }
    return 0;
}