#include "InventoryComponent.h"
#include "InspectableItem.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UInventoryComponent::AddItem(TSubclassOf<AInspectableItem> ItemClass, int32 Quantity)
{
	if (ItemClass.Get() == nullptr || Quantity <= 0)
	{
		return false;
	}

	// Размер стопки берём из настроек предмета // Take the stack size from the item defaults
	const AInspectableItem* Def = GetDefault<AInspectableItem>(ItemClass.Get());
	const int32 MaxStack = FMath::Max(1, Def->MaxStack);

	// Считаем, поместится ли всё: свободное место в стопках + свободные ячейки
	// Check if everything fits: free room in stacks + free slots
	int32 FreeInStacks = 0;
	for (const FInventoryEntry& Entry : Items)
	{
		if (Entry.ItemClass == ItemClass)
		{
			FreeInStacks += MaxStack - Entry.Quantity;
		}
	}
	const int32 FreeSlots = MaxSlots - Items.Num();
	if (FreeInStacks + FreeSlots * MaxStack < Quantity)
	{
		return false;
	}

	int32 Remaining = Quantity;

	// Сначала дополняем существующие стопки // First top up the existing stacks
	for (FInventoryEntry& Entry : Items)
	{
		if (Remaining <= 0)
		{
			break;
		}
		if (Entry.ItemClass == ItemClass && Entry.Quantity < MaxStack)
		{
			const int32 ToAdd = FMath::Min(MaxStack - Entry.Quantity, Remaining);
			Entry.Quantity += ToAdd;
			Remaining -= ToAdd;
		}
	}

	// Остаток кладём в новые ячейки // Put the rest into new slots
	while (Remaining > 0)
	{
		FInventoryEntry NewEntry;
		NewEntry.ItemClass = ItemClass;
		NewEntry.Quantity = FMath::Min(MaxStack, Remaining);
		Items.Add(NewEntry);
		Remaining -= NewEntry.Quantity;
	}

	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::RemoveItem(TSubclassOf<AInspectableItem> ItemClass, int32 Quantity)
{
	if (Quantity <= 0 || GetItemCount(ItemClass) < Quantity)
	{
		return false;
	}

	int32 Remaining = Quantity;

	// Забираем с конца, пустые ячейки удаляем // Take from the end, remove empty slots
	for (int32 i = Items.Num() - 1; i >= 0 && Remaining > 0; --i)
	{
		if (Items[i].ItemClass != ItemClass)
		{
			continue;
		}

		const int32 ToTake = FMath::Min(Items[i].Quantity, Remaining);
		Items[i].Quantity -= ToTake;
		Remaining -= ToTake;

		if (Items[i].Quantity <= 0)
		{
			Items.RemoveAt(i);
		}
	}

	OnInventoryChanged.Broadcast();
	return true;
}

int32 UInventoryComponent::GetItemCount(TSubclassOf<AInspectableItem> ItemClass) const
{
	int32 Total = 0;
	for (const FInventoryEntry& Entry : Items)
	{
		if (Entry.ItemClass == ItemClass)
		{
			Total += Entry.Quantity;
		}
	}
	return Total;
}

bool UInventoryComponent::HasItem(TSubclassOf<AInspectableItem> ItemClass, int32 MinQuantity) const
{
	return GetItemCount(ItemClass) >= MinQuantity;
}