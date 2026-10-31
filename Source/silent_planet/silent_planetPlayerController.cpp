// Copyright Epic Games, Inc. All Rights Reserved.


#include "silent_planetPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/StaticMesh.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Camera/PlayerCameraManager.h"
#include "silent_planetCameraManager.h"
#include "Blueprint/UserWidget.h"
#include "silent_planet.h"
#include "InspectActor.h"
#include "Widgets/Input/SVirtualJoystick.h"

Asilent_planetPlayerController::Asilent_planetPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = Asilent_planetCameraManager::StaticClass();
}

void Asilent_planetPlayerController::BeginPlay()
{
	Super::BeginPlay();


	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController() && ShouldUseTouchControls())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		}
		else {

			UE_LOG(Logsilent_planet, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void Asilent_planetPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}

		// Привязка ввода для осмотра предметов
		if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
		{
			if (InspectAction)
			{
				EIC->BindAction(InspectAction, ETriggerEvent::Triggered, this, &Asilent_planetPlayerController::OnInspectInput);
			}
			if (ZoomAction)
			{
				EIC->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &Asilent_planetPlayerController::OnZoomInput);
			}
		}
	}

}

bool Asilent_planetPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

void Asilent_planetPlayerController::StartInspect(UStaticMesh* ItemMesh)
{
	if (InspectActor || !InspectActorClass || !ItemMesh)
	{
		return;
	}

	// Ставим предмет перед камерой
	const FVector SpawnLoc = PlayerCameraManager->GetCameraLocation()
		+ PlayerCameraManager->GetCameraRotation().Vector() * 80.f;

	FActorSpawnParameters Params;
	InspectActor = GetWorld()->SpawnActor<AInspectActor>(InspectActorClass, SpawnLoc, FRotator::ZeroRotator, Params);
	if (!InspectActor)
	{
		return;
	}
	InspectActor->SetMesh(ItemMesh);

	// Отключаем обычное управление персонажем
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);

	if (UEnhancedInputLocalPlayerSubsystem* Sub =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (InspectMappingContext)
		{
			Sub->AddMappingContext(InspectMappingContext, 10);
		}
	}
}

void Asilent_planetPlayerController::EndInspect()
{
	if (!InspectActor)
	{
		return;
	}

	InspectActor->Destroy();
	InspectActor = nullptr;

	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);

	if (UEnhancedInputLocalPlayerSubsystem* Sub =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (InspectMappingContext)
		{
			Sub->RemoveMappingContext(InspectMappingContext);
		}
	}
}

void Asilent_planetPlayerController::OnInspectInput(const FInputActionValue& Value)
{
	if (InspectActor)
	{
		InspectActor->AddInspectInput(Value.Get<FVector2D>());
	}
}

void Asilent_planetPlayerController::OnZoomInput(const FInputActionValue& Value)
{
	if (InspectActor)
	{
		InspectActor->AddZoomInput(Value.Get<float>());
	}
}