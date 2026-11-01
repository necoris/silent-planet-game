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
#include "InspectableItem.h"
#include "Components/StaticMeshComponent.h"

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
			// Кнопки "в инвентарь" и "вернуть" // "To inventory" and "put back" buttons
			if (TakeAction)
			{
				EIC->BindAction(TakeAction, ETriggerEvent::Started, this, &Asilent_planetPlayerController::OnTakePressed);
			}
			if (CancelAction)
			{
				EIC->BindAction(CancelAction, ETriggerEvent::Started, this, &Asilent_planetPlayerController::OnCancelPressed);
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

	// Ставим предмет перед камерой на InspectDistance // Place the item in front of the camera at InspectDistance
	const FVector SpawnLoc = PlayerCameraManager->GetCameraLocation()
		+ PlayerCameraManager->GetCameraRotation().Vector() * InspectDistance;

	FActorSpawnParameters Params;
	// Поворот по камере, чтобы мышь крутила предмет правильно // Rotate by camera so mouse rotation feels correct
	InspectActor = GetWorld()->SpawnActor<AInspectActor>(InspectActorClass, SpawnLoc, PlayerCameraManager->GetCameraRotation(), Params);

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

void Asilent_planetPlayerController::Interact()
{
	// Пока осматриваем, E ничего не делает // While inspecting, E does nothing
	if (InspectActor)
	{
		return;
	}

	FVector Loc;
	FRotator Rot;
	GetPlayerViewPoint(Loc, Rot);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetPawn());

	if (GetWorld()->LineTraceSingleByChannel(Hit, Loc, Loc + Rot.Vector() * InteractDistance, ECC_Visibility, Params))
	{
		if (AInspectableItem* Item = Cast<AInspectableItem>(Hit.GetActor()))
		{
			StartInspectItem(Item);
		}
	}
}

void Asilent_planetPlayerController::StartInspectItem(AInspectableItem* Item)
{
	if (InspectActor || !InspectActorClass || !Item)
	{
		return;
	}

	// Ставим предмет перед камерой на InspectDistance // Place the item in front of the camera at InspectDistance
	const FVector SpawnLoc = PlayerCameraManager->GetCameraLocation()
		+ PlayerCameraManager->GetCameraRotation().Vector() * InspectDistance;

	FActorSpawnParameters Params;
	// Поворот по камере, чтобы мышь крутила предмет правильно // Rotate by camera so mouse rotation feels correct
	InspectActor = GetWorld()->SpawnActor<AInspectActor>(InspectActorClass, SpawnLoc, PlayerCameraManager->GetCameraRotation(), Params);

	if (!InspectActor)
	{
		return;
	}
	InspectActor->SetupFromComponent(Item->Mesh);

	// Прячем предмет в мире
	HeldItem = Item;
	HeldItem->SetHeldState(true);

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

	OnInspectStarted(Item->ItemName, Item->ItemDescription);
}

void Asilent_planetPlayerController::EndInspect()
{
	// Esc: вернуть предмет на место // Esc: put the item back in place
	CloseInspect(false);
}

void Asilent_planetPlayerController::TakeToInventory()
{
	// I: забрать предмет в инвентарь // I: take the item into the inventory
	CloseInspect(true);
}

void Asilent_planetPlayerController::OnTakePressed()
{
	TakeToInventory();
}

void Asilent_planetPlayerController::OnCancelPressed()
{
	EndInspect();
}

void Asilent_planetPlayerController::CloseInspect(bool bToInventory)
{
	if (!InspectActor)
	{
		return;
	}

	// Убираем копию из рук // Remove the copy from hands
	InspectActor->Destroy();
	InspectActor = nullptr;

	if (HeldItem)
	{
		if (bToInventory)
		{
			// Запоминаем в инвентаре и убираем предмет из мира
			// Store it in the inventory and remove the item from the world
			InventoryItems.Add(HeldItem->GetClass());
			OnItemTakenToInventory(HeldItem->ItemName);
			HeldItem->Destroy();
		}
		else
		{
			// Предмет просто становится видимым на прежнем месте
			// The item just becomes visible again at its old place
			HeldItem->SetHeldState(false);
		}
		HeldItem = nullptr;
	}

	// Возвращаем управление персонажем // Give control back to the character
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

	OnInspectEnded();
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
		// Минус меняет направление зума // The minus flips the zoom direction
		InspectActor->AddZoomInput(-Value.Get<float>());
	}
}