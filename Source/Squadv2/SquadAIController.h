// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SquadPlayerController.h"
#include "TimerManager.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SquadAIController.generated.h"

/**
 *
 */
UCLASS()
class SQUADV2_API ASquadAIController : public AAIController
{
	GENERATED_BODY()

public:
	FCommandPoint LastCommand;
	TArray<FCommandPoint> LocalCommandList; //In case I need to store multiple commands


	UPROPERTY(EditAnywhere)
	FCommandPoint PriorityCommand;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* AssignedPosition;


	UPROPERTY(VisibleAnywhere)
	UBlackboardComponent* TheBlackboard;

	UFUNCTION()
	void ClearRoom(FVector RoomLocation);
	UFUNCTION()
	void MoveToCommand(FCommandPoint CommandPoint);

	UFUNCTION(BlueprintCallable)
	void ResetPriorityCommand();

	UFUNCTION(BlueprintCallable)
	void OnUpdated(AActor* NewActor);



protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UAIPerceptionComponent* AIPerceptionComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UAISenseConfig_Sight* SightConfig;




	UFUNCTION()
	void HandleCommand(FCommandPoint CommandPoint);
	UFUNCTION()
	void FollowPlayer();


private:
	UPROPERTY()
	ASquadPlayerController* PlayerController;
	UPROPERTY()
	FTimerHandle TimerHandle;

	void SetupPerceptionSystem();


};


