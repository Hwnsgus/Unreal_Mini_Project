// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/Assign07GameMode.h"

#include "Character/Assign07Character.h"
#include "UObject/ConstructorHelpers.h"

AAssign07GameMode::AAssign07GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
