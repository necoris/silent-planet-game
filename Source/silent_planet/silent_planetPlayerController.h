// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "silent_planetPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;
class AInspectActor;
class AInspectableItem; // Это исправление ошибки // This fixes the compile error
class UStaticMesh;
struct FInputActionValue;

UCLASS(abstract, config = "Game")
class SILENT_PLANET_API Asilent_planetPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	Asilent_planetPlayerController();

	// Взять предмет, на который смотрим (клавиша E) // Pick up the item we are looking at (E key)
	UFUNCTION(BlueprintCallable, Category = "Inspect")
	void Interact();

	// Старый тестовый осмотр по мешу // Old test inspect by mesh
	UFUNCTION(BlueprintCallable, Category = "Inspect")
	void StartInspect(UStaticMesh* ItemMesh);

	// Вернуть предмет на место (Esc) // Put the item back in place (Esc)
	UFUNCTION(BlueprintCallable, Category = "Inspect")
	void EndInspect();

	// Положить предмет в инвентарь (I) // Put the item into the inventory (I)
	UFUNCTION(BlueprintCallable, Category = "Inspect")
	void TakeToInventory();

protected:

	// Расстояние от камеры до предмета при осмотре // Distance from camera to the item while inspecting
	UPROPERTY(EditDefaultsOnly, Category = "Inspect")
	float InspectDistance = 120.f;

	UPROPERTY(EditDefaultsOnly, Category = "Inspect")
	float InteractDistance = 300.f;

	// Предмет, который сейчас в руках // The item currently in hands
	UPROPERTY()
	AInspectableItem* HeldItem;

	// Что лежит в инвентаре (классы предметов) // What is in the inventory (item classes)
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TArray<TSubclassOf<AInspectableItem>> InventoryItems;

	void StartInspectItem(AInspectableItem* Item);

	// bToInventory = true: предмет уходит в инвентарь; false: возвращается на место
	// bToInventory = true: item goes to the inventory; false: it returns to its place
	void CloseInspect(bool bToInventory);

	// Реализуется в Blueprint: показать/убрать надпись внизу
	// Implemented in Blueprint: show/hide the text at the bottom
	UFUNCTION(BlueprintImplementableEvent, Category = "Inspect")
	void OnInspectStarted(const FText& ItemName, const FText& ItemDescription);

	UFUNCTION(BlueprintImplementableEvent, Category = "Inspect")
	void OnInspectEnded();

	// Вызывается, когда предмет попал в инвентарь (можно показать "Предмет добавлен")
	// Called when an item went into the inventory (can show "Item added")
	UFUNCTION(BlueprintImplementableEvent, Category = "Inspect")
	void OnItemTakenToInventory(const FText& ItemName);

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

	// Кнопка "вернуть" (Esc) // "Put back" button (Esc)
	UPROPERTY(EditDefaultsOnly, Category = "Inspect")
	UInputAction* CancelAction;

	UPROPERTY()
	AInspectActor* InspectActor;

	void OnInspectInput(const FInputActionValue& Value);
	void OnZoomInput(const FInputActionValue& Value);
	void OnTakePressed();
	void OnCancelPressed();

	// --------------------------------------

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	bool ShouldUseTouchControls() const;
};