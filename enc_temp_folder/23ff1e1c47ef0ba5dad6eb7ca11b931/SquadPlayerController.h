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


public:
	//APlayerController();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION()
	FCommandPoint CreateCommandPoint(FHitResult HitResult);
	UFUNCTION()
	FCommandPoint AssignLocation(FCommandPoint CommandPoint, FHitResult HitResult);
	UFUNCTION()
	FCommandPoint AssignType(FCommandPoint CommandPoint, FHitResult HitResult);
	UFUNCTION(BlueprintCallable)
	TArray<AActor*> GetRooms(AActor* Building);
	UFUNCTION()
	void CheckRoomValues(ARoom* Room);
	UFUNCTION()
	void AssignRoom(ARoom* Room);
	UFUNCTION()
	void DeployInvestigate(FCommandPoint CommandPoint);
	UFUNCTION()
	void AssignPriorityCommand(FCommandPoint CommandPoint);
	UFUNCTION(BlueprintCallable)
	ASquadAIController* GetAvailableMember(FCommandPoint CommandPoint);
	UFUNCTION()
	void SetNewAITarget(AActor* NewTarget);


	virtual void Tick(float DeltatTime) override;

	UFUNCTION()
	void FireProjectile();
	UFUNCTION()
	void MoveUpCommand();
	UFUNCTION(BlueprintCallable)
	void FormUpCommand();


	virtual void SetupInputComponent() override;

	//UPROPERTY(EditAnywhere)
	//TArray<FCommandPoint> CommandList;
	UPROPERTY(VisibleAnywhere)
	TArray<AActor*> SquadMembers;
	TArray<AActor*> RoomsInBuilding;


private:

	UPROPERTY(EditAnywhere)
	float MaxRange = 2000;
	UPROPERTY(EditAnywhere)

	FVector CameraLocation;
	FRotator CameraRotation;
	APawn* ControlledPawn;
	TArray<AActor*> DisposableList;

};
