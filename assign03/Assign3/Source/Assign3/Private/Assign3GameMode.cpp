#include "Assign3GameMode.h"
#include "MyPawn.h"

AAssign3GameMode::AAssign3GameMode()
{
	// 이전과 동일하게 내가 만든 C++ MyPawn을 기본 빙의 캐릭터로 등록합니다.
	DefaultPawnClass = AMyPawn::StaticClass();
}