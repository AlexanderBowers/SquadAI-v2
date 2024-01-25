// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
//#include "SquadAIController.h"
#include "Room.generated.h"

UCLASS()
class SQUADV2_API ARoom : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoom();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
	bool bIsCleared = false;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
	//ASquadAIController* AssignedSquadMember;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
