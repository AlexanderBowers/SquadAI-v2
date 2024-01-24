// Copyright Epic Games, Inc. All Rights Reserved.

#include "Squadv2GameMode.h"
#include "Squadv2HUD.h"
#include "Squadv2Character.h"
#include "UObject/ConstructorHelpers.h"

ASquadv2GameMode::ASquadv2GameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ASquadv2HUD::StaticClass();
}
