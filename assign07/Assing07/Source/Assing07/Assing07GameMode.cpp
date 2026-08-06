// Copyright Epic Games, Inc. All Rights Reserved.

#include "Assing07GameMode.h"
#include "Assing07Character.h"
#include "UObject/ConstructorHelpers.h"

AAssing07GameMode::AAssing07GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
