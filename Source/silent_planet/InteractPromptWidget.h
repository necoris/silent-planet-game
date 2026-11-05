#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractPromptWidget.generated.h"

class UTextBlock;

/**
 *  Подсказка "[E] ..." по центру экрана
 *  The "[E] ..." prompt at the center of the screen
 */
UCLASS(Abstract)
class SILENT_PLANET_API UInteractPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Показать текст подсказки // Show the prompt text
	void SetPrompt(const FText& Text);

protected:
	// В макете виджета должен быть Text Block с ТОЧНО таким именем
	// The widget layout must have a Text Block with EXACTLY this name
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PromptText;
};