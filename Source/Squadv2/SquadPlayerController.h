// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SquadPlayerController.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
	struct FCommandPoint
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Type;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* OwnerActor;
};
UCLASS()
class SQUADV2_API ASquadPlayerController : public APlayerController
{
	GENERATED_BODY()
};
