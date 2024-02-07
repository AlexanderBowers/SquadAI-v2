// Fill out your copyright notice in the Description page of Project Settings.


#include "SquadPlayerController.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Components.h"
#include "Engine/Engine.h"
#include "CommandComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SquadInterface.h"
#include "SquadAIController.h"
#include <Kismet/GameplayStatics.h>
#include "UObject/Class.h"
#include "GameFramework/Character.h"
#include "Room.h"

void ASquadPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (ASquadPlayerController::GetPawn())
	{
		ControlledPawn = GetPawn();
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASquadAIController::StaticClass(), SquadMembers); // Get all Squad Member controllers.
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASquadAIController::StaticClass(), DisposableList); // Get all Squad Member controllers.

	}

}


FCommandPoint ASquadPlayerController::CreateCommandPoint(FHitResult HitResult)
{
	//if we get a collision, create a FCommandPoint. 
	//If the collided actor has a Command Component, get its type and add to the CommandList.

	FCommandPoint CommandPoint;
	CommandPoint = AssignLocation(CommandPoint, HitResult);
	CommandPoint = AssignType(CommandPoint, HitResult);
	return CommandPoint;

}

FCommandPoint ASquadPlayerController::AssignLocation(FCommandPoint CommandPoint, FHitResult HitResult)
{  //Starting location of a CommandPoint to be given to an AI. This may change depending on its type.
	CommandPoint.Location = HitResult.Location;
	return CommandPoint;

}

FCommandPoint ASquadPlayerController::AssignType(FCommandPoint CommandPoint, FHitResult HitResult)
{ //CommandPoints can have a variety of types. 
  //Move: self explanatory. This is the default fallback.
  //Target: All squad members should set this to their target actor.
  //Detonate: One AI temporarily has this set as a priority. They will place a bomb then return. Selecting this again will blow up the object.
  //Cover: crouch at location; this is set in SquadAIController::HandleCommand
  //Investigate: CommandPoint.Location gets set to a specific component.
  //FirePoint: One AI gets set this as a priority to move to or recalled from. See SquadPlayerController::GetAvailableMembers() for assignment.
  //Return: used in SquadPlayerController::FormUpCommand. Otherwise used as a backup if there was no actor found from the HitResult.

	AActor* Actor = HitResult.GetActor();
	if (Actor)
	{
		UActorComponent* Component = Actor->FindComponentByClass<UCommandComponent>();
		if (Component)
		{
			FString TagType = Component->ComponentTags[0].ToString(); //Always look for the first tag on an actor to determine the type of CommandPoint.
			if (TagType.Len() > 0)
			{
				CommandPoint.Type = FName(TagType);
				CommandPoint.OwnerActor = Actor;
				DrawDebugSphere(GetWorld(), HitResult.Location, 20, 8, FColor::Green, false, 2, 0, 1.f);

				if (CommandPoint.Type == FName("Detonate"))
				{
					if (Actor->Implements<USquadInterface>())
					{
						UStaticMeshComponent* BombPoint = Cast<UStaticMeshComponent>(Actor->GetDefaultSubobjectByName(TEXT("BombLocation")));
						if (BombPoint)
						{
							ISquadInterface::Execute_CheckAssignedMember(Actor, CommandPoint);
							CommandPoint.Location.X = 0.00f; //prevents squad members who didn't receive the assignment from moving to it.
						}
					}
				}
				if (CommandPoint.Type == FName("Target"))
				{
					//SetNewAITarget(Actor);
					CommandPoint.Location.X = 0.00f;
				}
				if (CommandPoint.Type == FName("Investigate")) //Grab a static mesh called EndLocation on the actor. That will be the new location to move to.
				{
					UStaticMeshComponent* EndLocation = Cast<UStaticMeshComponent>(Actor->GetDefaultSubobjectByName(TEXT("EndLocation")));
					if (EndLocation)
					{
						FVector RightLocation = EndLocation->GetComponentLocation();
						CommandPoint.Location = RightLocation;
						return CommandPoint;
					}
				}
				if (CommandPoint.Type == FName("FirePoint"))
				{
					if (Actor->Implements<USquadInterface>())
					{	//We're checking to see if this actor has an assigned member. If not, assign one through SquadPlayerController::GetAvailableMember. If it already does, recall them.
						ISquadInterface::Execute_CheckAssignedMember(Actor, CommandPoint);
						CommandPoint.Location.X = 0.00f; //This is to restrict other AI other than the one specified in GetAvailableMember from moving to this location.
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Found Component but not tag!"))
					CommandPoint.Type = FName("Move");
			}
		}
		else //If there is no component, default the type to move.
		{
			CommandPoint.Type = FName("Move");
			DrawDebugSphere(GetWorld(), HitResult.Location, 20, 8, FColor::Red, false, 2, 0, 1.f);
		}
		return CommandPoint;

	}
	//If there is no actor hit, return to the player
	UE_LOG(LogTemp, Warning, TEXT("No actor found. Returning to player."))
		CommandPoint.Location = this->GetPawn()->GetActorLocation();
	CommandPoint.Type = FName("Return");
	return CommandPoint;

}

TArray<AActor*> ASquadPlayerController::GetRooms(AActor* Building)
{
	//Search through all Rooms in the building
	//Check if a Room is cleared.
	//If it's not cleared, check if it's assigned.
	//If it isn't assigned, assign the first SquadMember who doesn't have a room assigned.
	Building->GetAllChildActors(RoomsInBuilding);
	UE_LOG(LogTemp, Warning, TEXT("Start GetRooms"));
	for (AActor* Room : RoomsInBuilding)
	{
		FVector RoomLocation = Room->GetActorLocation();
		ARoom* BPRoom = Cast<ARoom>(Room);
		CheckRoomValues(BPRoom);

	}
	return RoomsInBuilding;
}

void ASquadPlayerController::CheckRoomValues(ARoom* Room)
{
	if (!Room->bIsCleared) //Check to see if the value of bIsCleared is false.
	{
		if (Room->AssignedSquadMember == nullptr) //Check to see if there is a property by the name of AssignedSquadMember
		{
					AssignRoom(Room);


		}
	}
}

void ASquadPlayerController::AssignRoom(ARoom* Room)
{
	for (AActor* Member : SquadMembers)
	{
		ASquadAIController* Commando = Cast<ASquadAIController>(Member);
		if (Commando) //Check to see if the squad member is valid
		{
			UBlackboardComponent* Blackboard = Commando->GetBlackboardComponent();
			if (Blackboard)
			{
				if(Blackboard->GetValueAsObject(FName("Room")) == nullptr && Room->AssignedSquadMember == nullptr)
				{
					Room->AssignedSquadMember = Commando;
					Blackboard->SetValueAsObject(FName("Room"), Room);
					Commando->ClearRoom(Room->GetActorLocation());
					return;
				}
			}
		}
	}
}

void ASquadPlayerController::DeployInvestigate(FCommandPoint CommandPoint)
{ //I think this was a prototype for my Investigate CommandPoint type. Safe to delete?

	UE_LOG(LogTemp, Warning, TEXT("Test 0: DeployInvestigate"));
	if (DisposableList.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Test 1: DisposableList is > 0"));
		for (int32 Index = DisposableList.Num() - 1; Index >= 0; --Index)
		{
			UE_LOG(LogTemp, Warning, TEXT("Test 2: Inside loop"));
			AActor* CurrentActor = DisposableList[Index];
			ASquadAIController* Commando = Cast<ASquadAIController>(CurrentActor);
			if (Commando)
			{
				UE_LOG(LogTemp, Warning, TEXT("Test 3: Cast successful"))
					//Commando->MoveToCommand(CommandPoint);
				DisposableList.RemoveAt(Index);
				break;
			}

		}
		UE_LOG(LogTemp, Warning, TEXT("Test 4: Outside loop"));



		// Perform any required actions with CurrentActor

		// Remove the element at the end of the iteration

	}
}

void ASquadPlayerController::AssignPriorityCommand(FCommandPoint CommandPoint) // Could probably be part of refactoring GetAvailableMember.
{

}

ASquadAIController* ASquadPlayerController::GetAvailableMember(FCommandPoint CommandPoint)
{  // Get the first AIController that doesn't have a priority command

	FVector CommandLocation = CommandPoint.Location;
	float BestDistance = 100000000; //Arbitrary number.
	ASquadAIController* ClosestMember = nullptr;
	for (AActor* Actor : SquadMembers)
	{
		ASquadAIController* SquadMember = Cast<ASquadAIController>(Actor);
		if (SquadMember)
		{
			UBlackboardComponent* Blackboard = SquadMember->GetBlackboardComponent();
			if (!Blackboard->GetValueAsBool(FName("bIsAssigned")))
			{
				FVector MemberLocation = SquadMember->GetCharacter()->GetActorLocation();
				if (FVector::Distance(MemberLocation, CommandLocation) <= BestDistance)
				{
					BestDistance = FVector::Distance(MemberLocation, CommandLocation);

					ClosestMember = SquadMember;
				}
			}
		}
	}
	if (ClosestMember) //AIController gets all the info he needs to move to the spot. Assignment of AssignedLocation pointer set in the CheckAssignedMember event.
	{
		UBlackboardComponent* Blackboard = ClosestMember->GetBlackboardComponent();
		if (Blackboard)
		{
		
			UE_LOG(LogTemp, Warning, TEXT("bIsAssigned == false"));
			Blackboard->SetValueAsBool(FName("bIsAssigned"), true);
			Blackboard->SetValueAsVector(FName("AssignedLocation"), CommandPoint.Location);
			UE_LOG(LogTemp, Warning, TEXT("found closest member."));
			ClosestMember->ResetFollow();
			ClosestMember->MoveToCommand(CommandPoint);
			
		}
		return ClosestMember;
	}
	UE_LOG(LogTemp, Warning, TEXT("No member found! returning nullptr"));
	return nullptr;
}

void ASquadPlayerController::SetNewAITarget(AActor* NewTarget)
{
	for (AActor* Actor : SquadMembers)
	{
		ASquadAIController* SquadMember = Cast<ASquadAIController>(Actor);
		if (SquadMember)
		{
			UBlackboardComponent* Blackboard = SquadMember->GetBlackboardComponent();
			if (Blackboard)
			{
				Blackboard->SetValueAsObject(FName("TargetActor"), NewTarget);
			}
		}
	}
}

void ASquadPlayerController::Tick(float DeltatTime)
{

}

void ASquadPlayerController::FireProjectile()
{
	if (Implements<USquadInterface>())
	{
		ISquadInterface::Execute_FireBPProjectile(this);
	}
}

void ASquadPlayerController::MoveUpCommand()
{
	//Use a line trace to find a location for AI to move to.

	if (ControlledPawn)
	{
		GetPlayerViewPoint(CameraLocation, CameraRotation);
		FVector End = CameraLocation + CameraRotation.Vector() * MaxRange;

		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(ControlledPawn);

		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, End, ECC_Visibility, CollisionParams);
		if (bHit)
		{
			//CreateCommandPointy checks to see if the hitresult actor has a Command Component and its type.
			FCommandPoint CommandPoint = CreateCommandPoint(HitResult);
			for (AActor* AI : SquadMembers)
			{
				ASquadAIController* Commando = Cast<ASquadAIController>(AI);
				if (Commando)
				{
					UBlackboardComponent* Blackboard = Commando->GetBlackboardComponent();
					if (!Blackboard->GetValueAsBool(FName("bIsAssigned")))
					{
						Commando->ResetFollow();
						Commando->MoveToCommand(CommandPoint);
					}
				}
			}
		}
	}
}

void ASquadPlayerController::FormUpCommand() //Generic recall function to return all AI regardless if they have assgned locations.
{
	if (ControlledPawn)
	{
		/*for (AActor* AI : SquadMembers)
		{
			ASquadAIController* Commando = Cast<ASquadAIController>(AI);
			if (Commando)
			{
				Commando->ResetPriorityCommand();
			}
		}
		DrawDebugSphere(GetWorld(), ControlledPawn->GetActorLocation(), 20, 20, FColor::Purple, false, 2, 0, 1.f);*/
		if (this->Implements<USquadInterface>())
		{
			ISquadInterface::Execute_ResetAssignedMember(this);
		}

	}
}

void ASquadPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("MoveUpCommand", IE_Pressed, this, &ASquadPlayerController::MoveUpCommand);
	InputComponent->BindAction("FormUpCommand", IE_Pressed, this, &ASquadPlayerController::FormUpCommand);
	InputComponent->BindAction("FireProjectile", IE_Pressed, this, &ASquadPlayerController::FireProjectile);

}
