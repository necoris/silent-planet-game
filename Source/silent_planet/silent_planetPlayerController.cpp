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
#include "ShooterPickup.h"
#include "Components/StaticMeshComponent.h"

// Склеить описание и подсказку в один текст // Join the description and the hint into one text
static FText MakeInspectText(const FText& Description, const FText& Hint)
{
	return FText::Format(INVTEXT("{0}\n\n{1}"), Description, Hint);
}

Asilent_planetPlayerController::Asilent_planetPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = Asilent_planetCameraManager::StaticClass();

	// Подсказки по умолчанию, свои текст задаётся в редакторе
	// Default hints, set your own text in the editor
	ItemHintText = INVTEXT("[I] Put in inventory   [F] Put back");
	WeaponHintText = INVTEXT("[E] Take in hands   [F] Put back");
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

		// Привязка ввода для осмотра предметов // Input bindings for item inspection
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

void Asilent_planetPlayerController::Interact()
{
	// Уже осматриваем: E подтверждает только оружие, обычные предметы не трогает
	// Already inspecting: E only confirms a weapon, regular items are not affected
	if (InspectActor)
	{
		if (HeldPickup)
		{
			ConfirmPickup();
		}
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
		AActor* HitActor = Hit.GetActor();

		if (AInspectableItem* Item = Cast<AInspectableItem>(HitActor))
		{
			// Предмет для осмотра // Item to inspect
			StartInspectItem(Item);
		}
		else if (AShooterPickup* Pickup = Cast<AShooterPickup>(HitActor))
		{
			// Оружие на стойке: сначала осматриваем // Weapon on a stand: inspect it first
			StartInspectPickup(Pickup);
		}
	}
}

bool Asilent_planetPlayerController::BeginInspect(UStaticMeshComponent* SourceMesh)
{
	if (InspectActor || !InspectActorClass || !SourceMesh)
	{
		return false;
	}

	// Ставим предмет перед камерой на InspectDistance // Place the item in front of the camera at InspectDistance
	const FVector SpawnLoc = PlayerCameraManager->GetCameraLocation()
		+ PlayerCameraManager->GetCameraRotation().Vector() * InspectDistance;

	FActorSpawnParameters Params;
	// Поворот по камере, чтобы мышь крутила предмет правильно // Rotate by camera so mouse rotation feels correct
	InspectActor = GetWorld()->SpawnActor<AInspectActor>(InspectActorClass, SpawnLoc, PlayerCameraManager->GetCameraRotation(), Params);

	if (!InspectActor)
	{
		return false;
	}
	InspectActor->SetupFromComponent(SourceMesh);

	// Отключаем обычное управление персонажем // Disable normal character control
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

	return true;
}

void Asilent_planetPlayerController::FinishInspect()
{
	// Убираем копию из рук // Remove the copy from hands
	if (InspectActor)
	{
		InspectActor->Destroy();
		InspectActor = nullptr;
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

void Asilent_planetPlayerController::StartInspectItem(AInspectableItem* Item)
{
	if (!Item || !BeginInspect(Item->Mesh))
	{
		return;
	}

	// Прячем предмет в мире // Hide the item in the world
	HeldItem = Item;
	HeldItem->SetHeldState(true);

	OnInspectStarted(Item->ItemName, MakeInspectText(Item->ItemDescription, ItemHintText));
}

void Asilent_planetPlayerController::StartInspectPickup(AShooterPickup* Pickup)
{
	if (!Pickup || !BeginInspect(Pickup->GetPickupMesh()))
	{
		return;
	}

	// Убираем оружие со стойки, пока его крутят // Take the weapon off the stand while it is inspected
	HeldPickup = Pickup;
	HeldPickup->SetHeldState(true);

	OnInspectStarted(Pickup->ItemName, MakeInspectText(Pickup->ItemDescription, WeaponHintText));
}

void Asilent_planetPlayerController::ConfirmPickup()
{
	if (!InspectActor || !HeldPickup)
	{
		return;
	}

	AShooterPickup* Pickup = HeldPickup;
	HeldPickup = nullptr;

	// Закрываем осмотр // Close the inspection
	FinishInspect();

	// Пикап надо показать, иначе он решит, что уже подобран
	// The pickup must be unhidden, otherwise it thinks it is already taken
	Pickup->SetHeldState(false);

	// Оружие переходит в руки персонажа // The weapon goes to the character's hands
	Pickup->TryPickup(GetPawn());
}

void Asilent_planetPlayerController::EndInspect()
{
	// F / Esc: вернуть предмет или оружие на место // F / Esc: put the item or weapon back in place
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

	// Оружие в инвентарь не кладётся // A weapon can't go to the inventory
	if (HeldPickup && bToInventory)
	{
		return;
	}

	// Оружие возвращается на стойку // The weapon goes back on the stand
	if (HeldPickup)
	{
		HeldPickup->SetHeldState(false);
		HeldPickup = nullptr;
	}

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

	FinishInspect();
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
		InspectActor->AddZoomInput(-Value.Get<float>());
	}
}