// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SquadAIController.h"
#include "SquadPlayerController.h"
#include "SquadInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USquadInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class SQUADV2_API ISquadInterface
{
	GENERATED_BODY()



	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AssignMember")
	void CheckAssignedMember(FCommandPoint CommandPoint);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AssignMember")
	void SetAssignedMember(ASquadAIController* AssignedSquadMember);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AssignMember")
	void ResetAssignedMember();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "EndLocation")
	void SetDesiredEndLocation(FCommandPoint CommandPoint);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Setup")
	void SetBehaviorTree(ASquadAIController* TheController);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	void SetNewTarget(AActor* NewTarget);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Perception")
	void UpdatePerception(AActor* NewActor);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	void FireBPProjectile();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AssignMember")
	void DetonateBomb(FCommandPoint CommandPoint);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MoveSquad")
	void GiveOrder(FCommandPoint CommandPoint);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MoveSquad")
	void StopFollow();
};
