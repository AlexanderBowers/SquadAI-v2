// Fill out your copyright notice in the Description page of Project Settings.


#include "SquadAIController.h"
#include "SquadPlayerController.h"
#include "Engine/EngineTypes.h"
#include "SquadInterface.h"
#include "Room.h"
#include "SquadPlayerController.h"
#include "Navigation/PathFollowingComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

void ASquadAIController::BeginPlay()
{
	Super::BeginPlay();
	if (GetWorld()->GetFirstPlayerController())
	{
		PlayerController = GetWorld()->GetFirstPlayerController<ASquadPlayerController>();

		//Giving them their own fake command to have something to compare to the first command.
		FCommandPoint BaseCommand;
		BaseCommand.Location = PlayerController->GetPawn()->GetActorLocation();
		BaseCommand.Type = FName("Move");
		LastCommand = BaseCommand;

	}
	TheBlackboard = GetBlackboardComponent();
}

void ASquadAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (PlayerController)
	{
		if (TheBlackboard)
		{
			if (TheBlackboard->GetValueAsBool(FName("bShouldFollow")))
			{
				FollowPlayer();

			}

			if (FVector::Distance(GetCharacter()->GetActorLocation(), PlayerController->GetPawn()->GetActorLocation()) >= 2000.0f)
			{
				ResetPriorityCommand();

			}
		}
	}
}

void ASquadAIController::MoveToCommand(FCommandPoint CommandPoint) //If they receive a new command, move to it.
{
	if (TheBlackboard)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Destination: %s"), *CommandPoint.Location.ToString());

		if (CommandPoint.Type == FName("Target"))
		{
			if (GetPawn()->Implements<USquadInterface>())
			{
				if (CommandPoint.OwnerActor != nullptr)
				{
					ISquadInterface::Execute_SetNewTarget(GetPawn(), CommandPoint.OwnerActor);
					return;
				}
			}
		}
		if (CommandPoint.Type == FName("Return"))
		{
			ResetPriorityCommand();
		}
		if (CommandPoint.Location.X == 0.00f)
		{
			return;
		}
\
		if (GetCharacter()->bIsCrouched)
		{
			GetCharacter()->UnCrouch();

		}
		if (this->Implements<USquadInterface>())
		{
			ISquadInterface::Execute_StopFollow(this);
		}
		MoveToLocation(CommandPoint.Location, 100);
		HandleCommand(CommandPoint);
	}

}

void ASquadAIController::HandleCommand(FCommandPoint CommandPoint) //Check if they need to crouch, suppress, etc.
{
	if (TheBlackboard)
	{
		//Called when AIController reached their destination
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "HandleCommand", CommandPoint);
		float DistanceThreshold = 100.0f;
		float DistanceToCommand = FVector::Distance(GetPawn()->GetActorLocation(), CommandPoint.Location);
		TheBlackboard->SetValueAsBool(FName("bIsMovingToCommand"), true);
		if (DistanceToCommand <= DistanceThreshold)
		{
			UE_LOG(LogTemp, Warning, TEXT("Distance threshold met."));
			if (CommandPoint.Type == FName("Cover")) // trying to convert this to switch statement
			{
				GetCharacter()->Crouch();

			}
			if (CommandPoint.Type == FName("Return"))
			{
				ResetPriorityCommand();
				TheBlackboard->SetValueAsBool(FName("bShouldFollow"), true);
			}
			Delegate.Unbind();
		}
		else
		{
			GetWorldTimerManager().SetTimer(TimerHandle, Delegate, 1.0f, false, 0.0f);

		}
	}
}

void ASquadAIController::FollowPlayer()
{
	if (TheBlackboard)
	{
		/*if (GetCharacter()->bIsCrouched)
		{
			GetCharacter()->UnCrouch();

		}
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "FollowPlayer");
		MoveToLocation(PlayerController->GetPawn()->GetActorLocation(), 200);
		GetWorldTimerManager().SetTimer(TimerHandle, Delegate, 0.5f, TheBlackboard->GetValueAsBool(FName("bShouldFollow")), 0.0f);
		Delegate.Unbind();*/
		AssignedPosition = TheBlackboard->GetValueAsObject(FName("AssignedPosition"));
		AActor* AssignedPositionPtr = Cast<AActor>(AssignedPosition);
		if (AssignedPosition != nullptr)
		{
			FTimerDelegate Delegate;
			Delegate.BindUFunction(this, "FollowPlayer");
			float DistanceThreshold = 100.0f;
			float DistanceToCommand = FVector::Distance(this->GetPawn()->GetActorLocation(), AssignedPositionPtr->GetActorLocation());
			UE_LOG(LogTemp, Warning, TEXT("DistanceToCommand: %f"), DistanceToCommand);
			if (DistanceToCommand >= DistanceThreshold)
			{
				MoveToLocation(AssignedPositionPtr->GetActorLocation());
				GetPawn()->SetActorRotation(AssignedPositionPtr->GetActorRotation());
			}
			if (DistanceToCommand <= DistanceThreshold)
			{
				TheBlackboard->SetValueAsBool(FName("bIsMovingToCommand"), false);
				Delegate.Unbind();

			}
			else
			{
				GetWorldTimerManager().SetTimer(TimerHandle, Delegate, 3.0f, false, 0.0f);

			}

		}
	}
}

void ASquadAIController::ClearRoom(FVector RoomLocation)
{
	//Check if the controller has a room assigned. Move there, wait 5 seconds, then return to the player. 
	if (TheBlackboard)
	{
		if (TheBlackboard->GetValueAsObject(FName("Room")) != nullptr)
		{
			UObject* RoomObject = TheBlackboard->GetValueAsObject(FName("Room"));
			ARoom* Room = Cast<ARoom>(RoomObject);
			if (Room)
			{
				UE_LOG(LogTemp, Warning, TEXT("Room != nullptr"));
				TheBlackboard->SetValueAsBool(FName("bShouldFollow"), false);
				FCommandPoint RoomPoint;
				RoomPoint.Location = RoomLocation;
				RoomPoint.Type = FName("Cover");
				MoveToCommand(RoomPoint);
				
			}
		}
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "ClearRoom", RoomLocation);
		float DistanceThreshold = 500.0f;
		float DistanceToCommand = FVector::Distance(this->GetPawn()->GetActorLocation(), RoomLocation);
		if (DistanceToCommand <= DistanceThreshold)
		{
			TheBlackboard->SetValueAsBool(FName("bShouldFollow"), true);
			Delegate.Unbind();

		}
		else
		{
			GetWorldTimerManager().SetTimer(TimerHandle, Delegate, 5.0f, false, 5.0f);

		}
	}

}

void ASquadAIController::ResetPriorityCommand()
{
	if (TheBlackboard->GetValueAsObject(FName("AssignedPosition")))
	{
		AssignedPosition = Cast<AActor>(TheBlackboard->GetValueAsObject(FName("AssignedPosition")));
	}
	if (AssignedPosition)
	{
		if (TheBlackboard->GetValueAsObject(FName("AssignedPosition"))->Implements<USquadInterface>())
		{
			ISquadInterface::Execute_ResetAssignedMember(AssignedPosition);
		}
		TheBlackboard->SetValueAsObject(FName("AssignedPosition"), nullptr);
	}
	AssignedPosition = nullptr;
	TheBlackboard->SetValueAsBool(FName("bShouldFollow"), true);
	TheBlackboard->SetValueAsBool(FName("bHasPriority"), false);
	TheBlackboard->SetValueAsBool(FName("bIsAssigned"), false);
	
	return;
}

void ASquadAIController::ResetFollow()
{
	TheBlackboard->SetValueAsBool(FName("bShouldFollow"), !TheBlackboard->GetValueAsBool(FName("bShouldFollow")));

	if (TheBlackboard->GetValueAsObject(FName("AssignedPosition")))
	{
		AssignedPosition = Cast<AActor>(TheBlackboard->GetValueAsObject(FName("AssignedPosition")));
	}
	if (AssignedPosition)
	{
		if (TheBlackboard->GetValueAsObject(FName("AssignedPosition"))->Implements<USquadInterface>())
		{
			ISquadInterface::Execute_ResetAssignedMember(AssignedPosition);
		}
		TheBlackboard->SetValueAsObject(FName("AssignedPosition"), nullptr);
	}
	AssignedPosition = nullptr;
	TheBlackboard->SetValueAsBool(FName("bHasPriority"), false);
	TheBlackboard->SetValueAsBool(FName("bIsAssigned"), false);

	return;
}



//TODO: Allow multiple Squad AI to respond to command. DONE!
//Allow multiple types of commands to be implemented (cover, move, suppress)
//NEXT STEPS:
//Get Following player working. Use a boolean to determine whether the AI should stick to the player. DONE!
