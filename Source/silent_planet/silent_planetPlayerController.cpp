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
#include "InventoryComponent.h"
#include "InventoryWidget.h"
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

	// Инвентарь игрока // Player inventory
	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));

	// Подсказки по умолчанию, свой текст задаётся в редакторе
	// Default hints, set your own text in the editor
	ItemHintText = INVTEXT("[I] Put in inventory   [F] Put back");
	WeaponHintText = INVTEXT("[E] Take in hands   [F] Put back");
	InventoryInspectHintText = INVTEXT("[F] Back");
}

void Asilent_planetPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Следим за изменениями инвентаря // Watch for inventory changes
	if (Inventory)
	{
		Inventory->OnInventoryChanged.AddDynamic(this, &Asilent_planetPlayerController::HandleInventoryChanged);
	}

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

		// Привязка ввода для осмотра предметов и инвентаря // Input bindings for item inspection and the inventory
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
			// Инвентарь: открыть/закрыть и выбор строки // Inventory: open/close and row selection
			if (InventoryAction)
			{
				EIC->BindAction(InventoryAction, ETriggerEvent::Started, this, &Asilent_planetPlayerController::OnInventoryPressed);
			}
			if (InventoryNavigateAction)
			{
				EIC->BindAction(InventoryNavigateAction, ETriggerEvent::Started, this, &Asilent_planetPlayerController::OnInventoryNavigate);
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
	// Инвентарь открыт: E осматривает выбранный предмет // Inventory is open: E inspects the selected item
	if (bInventoryOpen)
	{
		if (!InspectActor)
		{
			InspectSelectedInventoryItem();
		}
		return;
	}

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
	// Инвентарь открыт, а предмет не осматриваем: F закрывает инвентарь
	// The inventory is open and nothing is inspected: F closes the inventory
	if (bInventoryOpen && !InspectActor)
	{
		CloseInventory();
		return;
	}

	EndInspect();
}

void Asilent_planetPlayerController::CloseInspect(bool bToInventory)
{
	if (!InspectActor)
	{
		return;
	}

	// Осмотр из инвентаря: F или I возвращают к списку // Inspecting from the inventory: F or I go back to the list
	if (bInspectingFromInventory)
	{
		ReturnToInventoryList();
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
			// Кладём в инвентарь и убираем предмет из мира
			// Put it into the inventory and remove the item from the world
			if (Inventory && Inventory->AddItem(HeldItem->GetClass(), HeldItem->Quantity))
			{
				OnItemTakenToInventory(HeldItem->ItemName);
				HeldItem->Destroy();
			}
			else
			{
				// Инвентарь полон: предмет остаётся в руках // The inventory is full: the item stays in hands
				OnInventoryFull(HeldItem->ItemName);
				return;
			}
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

// ---------- Инвентарь // Inventory ----------

void Asilent_planetPlayerController::OnInventoryPressed()
{
	if (bInventoryOpen)
	{
		if (InspectActor)
		{
			// Осматриваем предмет из списка: Tab возвращает к списку
			// Inspecting a list item: Tab goes back to the list
			if (bInspectingFromInventory)
			{
				ReturnToInventoryList();
			}
		}
		else
		{
			CloseInventory();
		}
		return;
	}

	// Во время обычного осмотра инвентарь не открываем // Don't open the inventory during a regular inspection
	if (!InspectActor)
	{
		OpenInventory();
	}
}

void Asilent_planetPlayerController::OnInventoryNavigate(const FInputActionValue& Value)
{
	if (!bInventoryOpen || InspectActor || !Inventory)
	{
		return;
	}

	const int32 Count = Inventory->GetItems().Num();
	if (Count == 0)
	{
		return;
	}

	// Вверх даёт +1, вниз даёт -1 (Negate в Input Mapping Context)
	// Up gives +1, down gives -1 (Negate in the Input Mapping Context)
	const int32 Step = Value.Get<float>() > 0.f ? -1 : 1;

	// Переход по кругу // Wrap around
	SelectedIndex = (SelectedIndex + Step + Count) % Count;
	RefreshInventoryWidget();
}

void Asilent_planetPlayerController::OpenInventory()
{
	if (bInventoryOpen || InspectActor || !InventoryWidgetClass)
	{
		return;
	}

	InventoryWidget = CreateWidget<UInventoryWidget>(this, InventoryWidgetClass);
	if (!InventoryWidget)
	{
		return;
	}
	InventoryWidget->AddToViewport();

	bInventoryOpen = true;
	SelectedIndex = 0;

	// Пока инвентарь открыт, персонаж стоит // The character stands still while the inventory is open
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);

	RefreshInventoryWidget();
}

void Asilent_planetPlayerController::CloseInventory()
{
	if (!bInventoryOpen)
	{
		return;
	}

	// Если смотрели предмет из списка, сначала возвращаемся к списку
	// If an item from the list was being inspected, return to the list first
	if (InspectActor && bInspectingFromInventory)
	{
		ReturnToInventoryList();
	}

	if (InventoryWidget)
	{
		InventoryWidget->RemoveFromParent();
		InventoryWidget = nullptr;
	}

	bInventoryOpen = false;

	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);
}

void Asilent_planetPlayerController::RefreshInventoryWidget()
{
	if (!InventoryWidget || !Inventory)
	{
		return;
	}

	// Не даём выбору выйти за пределы списка // Keep the selection inside the list
	const int32 Count = Inventory->GetItems().Num();
	SelectedIndex = (Count > 0) ? FMath::Clamp(SelectedIndex, 0, Count - 1) : 0;

	InventoryWidget->Refresh(Inventory->GetItems(), SelectedIndex);
}

void Asilent_planetPlayerController::InspectSelectedInventoryItem()
{
	if (!Inventory || !bInventoryOpen || InspectActor)
	{
		return;
	}

	const TArray<FInventoryEntry>& Items = Inventory->GetItems();
	if (!Items.IsValidIndex(SelectedIndex) || Items[SelectedIndex].ItemClass.Get() == nullptr)
	{
		return;
	}

	// Берём меш и текст из настроек класса предмета // Take the mesh and text from the item class defaults
	const AInspectableItem* Def = GetDefault<AInspectableItem>(Items[SelectedIndex].ItemClass.Get());
	if (!Def || !BeginInspect(Def->Mesh))
	{
		return;
	}

	bInspectingFromInventory = true;

	// Прячем список, пока предмет в руках // Hide the list while the item is in hands
	if (InventoryWidget)
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	OnInspectStarted(Def->ItemName, MakeInspectText(Def->ItemDescription, InventoryInspectHintText));
}

void Asilent_planetPlayerController::ReturnToInventoryList()
{
	bInspectingFromInventory = false;

	// Убираем копию из рук и подпись внизу // Remove the copy from hands and the bottom text
	FinishInspect();

	// Показываем список обратно // Show the list again
	if (InventoryWidget)
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Visible);
	}
	RefreshInventoryWidget();
}

void Asilent_planetPlayerController::HandleInventoryChanged()
{
	RefreshInventoryWidget();
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