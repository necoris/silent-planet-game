// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "silent_planetPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;
class AInspectActor;
class AInspectableItem;
class AShooterPickup;
class UStaticMeshComponent;
class UInventoryComponent;
class UInventoryWidget;
class UInteractPromptWidget;
struct FInputActionValue;

UCLASS(abstract, config = "Game")
class SILENT_PLANET_API Asilent_planetPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	Asilent_planetPlayerController();

	// E: взять предмет, на который смотрим, подтвердить оружие или осмотреть выбранное в инвентаре
	// E: pick up the item we are looking at, confirm a weapon, or inspect the selection in the inventory
	UFUNCTION(BlueprintCallable, Category = "Inspect")
	void Interact();

	// Вернуть предмет или оружие на место (F / Esc) // Put the item or weapon back (F / Esc)
	UFUNCTION(BlueprintCallable, Category = "Inspect")
	void EndInspect();

	// Положить предмет в инвентарь (I) // Put the item into the inventory (I)
	UFUNCTION(BlueprintCallable, Category = "Inspect")
	void TakeToInventory();

	// Идёт ли осмотр или открыт инвентарь (тогда нельзя стрелять и менять позу)
	// Is an inspection in progress or the inventory open (then no shooting or stance changes)
	bool IsInspecting() const { return InspectActor != nullptr || bInventoryOpen; }

	// Инвентарь игрока (для дверей, ключей и т.д.) // Player inventory (for doors, keys, etc.)
	UFUNCTION(BlueprintPure, Category = "Inventory")
	UInventoryComponent* GetInventory() const { return Inventory; }

protected:

	// Инвентарь игрока // Player inventory
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	UInventoryComponent* Inventory;

	// Расстояние от камеры до предмета при осмотре // Distance from camera to the item while inspecting
	UPROPERTY(EditDefaultsOnly, Category = "Inspect")
	float InspectDistance = 120.f;

	UPROPERTY(EditDefaultsOnly, Category = "Inspect")
	float InteractDistance = 300.f;

	// Подсказка внизу для обычных предметов // Bottom hint for regular items
	UPROPERTY(EditDefaultsOnly, Category = "Inspect")
	FText ItemHintText;

	// Подсказка внизу для оружия // Bottom hint for weapons
	UPROPERTY(EditDefaultsOnly, Category = "Inspect")
	FText WeaponHintText;

	// Подсказка внизу при осмотре из инвентаря // Bottom hint when inspecting from the inventory
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	FText InventoryInspectHintText;

	// Предмет, который сейчас в руках // The item currently in hands
	UPROPERTY()
	AInspectableItem* HeldItem;

	// Оружие, которое сейчас осматриваем // The weapon pickup being inspected
	UPROPERTY()
	AShooterPickup* HeldPickup;

	// Создать копию в руках и включить ввод осмотра // Create the copy in hands and enable inspect input
	bool BeginInspect(UStaticMeshComponent* SourceMesh);

	// Закрыть осмотр: убрать копию, вернуть управление // Close inspection: remove the copy, give control back
	void FinishInspect();

	void StartInspectItem(AInspectableItem* Item);
	void StartInspectPickup(AShooterPickup* Pickup);

	// Взять осматриваемое оружие в руки // Take the inspected weapon in hands
	void ConfirmPickup();

	// bToInventory = true: предмет уходит в инвентарь; false: возвращается на место
	// bToInventory = true: item goes to the inventory; false: it returns to its place
	void CloseInspect(bool bToInventory);

	// ---------- Инвентарь // Inventory ----------

	void OpenInventory();
	void CloseInventory();
	void RefreshInventoryWidget();
	void InspectSelectedInventoryItem();
	void ReturnToInventoryList();

	// Вызывается, когда содержимое инвентаря изменилось // Called when the inventory contents changed
	UFUNCTION()
	void HandleInventoryChanged();

	// Какой виджет показывать как инвентарь // Which widget to show as the inventory
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UInventoryWidget> InventoryWidgetClass;

	// Открыть/закрыть инвентарь (Tab) // Open/close the inventory (Tab)
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	UInputAction* InventoryAction;

	// Выбор строки в списке (вверх/вниз), Digital не подойдёт: нужен Axis1D
	// Selecting a row in the list (up/down), needs an Axis1D action
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	UInputAction* InventoryNavigateAction;

	UPROPERTY()
	UInventoryWidget* InventoryWidget;

	bool bInventoryOpen = false;
	bool bInspectingFromInventory = false;
	int32 SelectedIndex = 0;

	void OnInventoryPressed();
	void OnInventoryNavigate(const FInputActionValue& Value);

	// Реализуется в Blueprint: предмет положен в инвентарь (например, показать "Предмет добавлен")
	// Implemented in Blueprint: the item was put into the inventory (for example show "Item added")
	UFUNCTION(BlueprintImplementableEvent, Category = "Inspect")
	void OnItemTakenToInventory(const FText& ItemName);

	// Реализуется в Blueprint: инвентарь полон // Implemented in Blueprint: the inventory is full
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnInventoryFull(const FText& ItemName);

	// Реализуется в Blueprint: показать/убрать надпись внизу
	// Implemented in Blueprint: show/hide the text at the bottom
	UFUNCTION(BlueprintImplementableEvent, Category = "Inspect")
	void OnInspectStarted(const FText& ItemName, const FText& ItemDescription);

	UFUNCTION(BlueprintImplementableEvent, Category = "Inspect")
	void OnInspectEnded();

	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	UPROPERTY(EditAnywhere, Category = "Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	// ---------- Осмотр предметов // Item inspection ----------

	UPROPERTY(EditDefaultsOnly, Category = "Inspect")
	TSubclassOf<AInspectActor> InspectActorClass;

	UPROPERTY(EditDefaultsOnly, Category = "Inspect")
	UInputMappingContext* InspectMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Inspect")
	UInputAction* InspectAction;

	UPROPERTY(EditDefaultsOnly, Category = "Inspect")
	UInputAction* ZoomAction;

	// Кнопка "в инвентарь" (I) // "To inventory" button (I)
	UPROPERTY(EditDefaultsOnly, Category = "Inspect")
	UInputAction* TakeAction;

	// Кнопка "вернуть" (F / Esc) // "Put back" button (F / Esc)
	UPROPERTY(EditDefaultsOnly, Category = "Inspect")
	UInputAction* CancelAction;

	UPROPERTY()
	AInspectActor* InspectActor;

	void OnInspectInput(const FInputActionValue& Value);
	void OnZoomInput(const FInputActionValue& Value);
	void OnTakePressed();
	void OnCancelPressed();

	// ---------- Подсказка "[E] ..." // The "[E] ..." prompt ----------

// Какой виджет показывать как подсказку // Which widget to show as the prompt
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	TSubclassOf<UInteractPromptWidget> PromptWidgetClass;

	// Слово в подсказке для предметов: "[E] Осмотреть: Ключ" // Word in the prompt for items: "[E] Examine: Key"
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	FText ItemPromptText;

	// Слово в подсказке для оружия // Word in the prompt for weapons
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	FText WeaponPromptText;

	UPROPERTY()
	UInteractPromptWidget* PromptWidget;

	// Текст, который показан сейчас // The text shown right now
	FText CurrentPrompt;

	// Что у нас под прицелом // What is under the crosshair
	AActor* TraceInteractTarget() const;

	// Какую подсказку показать для этого объекта // Which prompt to show for this object
	FText GetPromptFor(AActor* Target) const;

	// Обновить подсказку каждый кадр // Update the prompt every frame
	void UpdateInteractPrompt();

	virtual void PlayerTick(float DeltaTime) override;

	// --------------------------------------

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	bool ShouldUseTouchControls() const;
};