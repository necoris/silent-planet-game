#include "InventoryWidget.h"
#include "InspectableItem.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Styling/CoreStyle.h"

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (HintText)
	{
		HintText->SetText(HintMessage);
	}
}

void UInventoryWidget::Refresh(const TArray<FInventoryEntry>& Items, int32 SelectedIndex)
{
	if (!ItemList)
	{
		return;
	}

	ItemList->ClearChildren();

	// Показываем "пусто", если предметов нет // Show "empty" when there are no items
	if (EmptyText)
	{
		EmptyText->SetVisibility(Items.Num() == 0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	const AInspectableItem* SelectedDef = nullptr;

	for (int32 i = 0; i < Items.Num(); ++i)
	{
		UClass* ItemClass = Items[i].ItemClass.Get();
		const AInspectableItem* Def = ItemClass ? GetDefault<AInspectableItem>(ItemClass) : nullptr;
		if (!Def)
		{
			continue;
		}

		const bool bSelected = (i == SelectedIndex);
		if (bSelected)
		{
			SelectedDef = Def;
		}

		// Строка вида "> Ключ x2" // A row like "> Key x2"
		FText Line = Def->ItemName;
		if (Items[i].Quantity > 1)
		{
			Line = FText::Format(INVTEXT("{0} x{1}"), Line, Items[i].Quantity);
		}
		if (bSelected)
		{
			Line = FText::Format(INVTEXT("> {0}"), Line);
		}

		UTextBlock* Row = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Row->SetText(Line);
		Row->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", RowFontSize));
		Row->SetColorAndOpacity(FSlateColor(bSelected ? SelectedColor : NormalColor));
		ItemList->AddChild(Row);
	}

	// Название и описание выбранного предмета // Name and description of the selected item
	if (DetailName)
	{
		DetailName->SetText(SelectedDef ? SelectedDef->ItemName : FText::GetEmpty());
	}
	if (DetailDescription)
	{
		DetailDescription->SetText(SelectedDef ? SelectedDef->ItemDescription : FText::GetEmpty());
	}
}