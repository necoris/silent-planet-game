#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *  Всё, с чем можно взаимодействовать на E (панели, щитки, двери...)
 *  Everything you can interact with using E (panels, fuse boxes, doors...)
 */
class SILENT_PLANET_API IInteractableInterface
{
	GENERATED_BODY()

public:

	// Игрок нажал E, глядя на этот объект // The player pressed E while looking at this object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnInteract(AActor* Interactor);

	// Текст подсказки без "[E]", например "Ввести код". Пусто = подсказки нет
	// Prompt text without "[E]", for example "Enter code". Empty = no prompt
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractionText() const;
};