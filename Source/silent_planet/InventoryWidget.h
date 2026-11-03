#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryComponent.h"
#include "InventoryWidget.generated.h"

class UScrollBox;
class UTextBlock;

/**
 *  Экран инвентаря: список слева, описание справа
 *  Inventory screen: list on the left, details on the right
 */
UCLASS(Abstract)
class SILENT_PLANET_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Перерисовать список; SelectedIndex — выбранная строка
	// Redraw the list; SelectedIndex is the selected row
	void Refresh(const TArray<FInventoryEntry>& Items, int32 SelectedIndex);

protected:
	virtual void NativeConstruct() override;

	// Эти элементы должны быть в макете виджета с ТОЧНО такими именами
	// These elements must exist in the widget layout with EXACTLY these names
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ItemList;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* DetailName;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* DetailDescription;

	// Надпись "пусто", когда в инвентаре ничего нет // "Empty" text when there are no items
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* EmptyText;

	// Подсказка по клавишам внизу // Key hint at the bottom
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* HintText;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FText HintMessage = INVTEXT("[W/S] Select   [E] Inspect   [F] Close");

	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 RowFontSize = 28;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FLinearColor SelectedColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FLinearColor NormalColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.f);
};