#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class AInspectableItem;

// Одна ячейка инвентаря // One inventory slot
USTRUCT(BlueprintType)
struct FInventoryEntry
{
	GENERATED_BODY()

	// Класс предмета // Item class
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TSubclassOf<AInspectableItem> ItemClass;

	// Сколько штук в ячейке // How many pieces in the slot
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 Quantity = 1;
};

// Сообщает, что содержимое изменилось // Fired when the contents changed
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInventoryChangedSignature);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SILENT_PLANET_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	// Добавить предмет. Возвращает false, если всё не помещается (тогда ничего не добавляется)
	// Add an item. Returns false if it all doesn't fit (then nothing is added)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(TSubclassOf<AInspectableItem> ItemClass, int32 Quantity = 1);

	// Убрать предмет (например, ключ после открытия двери)
	// Remove an item (for example a key after opening a door)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(TSubclassOf<AInspectableItem> ItemClass, int32 Quantity = 1);

	// Сколько таких предметов всего // How many of this item in total
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetItemCount(TSubclassOf<AInspectableItem> ItemClass) const;

	// Есть ли предмет (нужно для замков и дверей) // Do we have the item (for locks and doors)
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(TSubclassOf<AInspectableItem> ItemClass, int32 MinQuantity = 1) const;

	const TArray<FInventoryEntry>& GetItems() const { return Items; }

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FInventoryChangedSignature OnInventoryChanged;

protected:
	// Сколько ячеек в инвентаре // Number of inventory slots
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = 1))
	int32 MaxSlots = 8;

	UPROPERTY()
	TArray<FInventoryEntry> Items;
};