// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InspectActor.h"
#include "silent_planetPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;
class AInspectActor;
class UStaticMesh;
struct FInputActionValue;

/**
 *  Simple first person Player Controller
 *  Manages the input mapping context.
 *  Overrides the Player Camera Manager class.
 */
UCLASS(abstract, config = "Game")
class SILENT_PLANET_API Asilent_planetPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	/** Constructor */
	Asilent_planetPlayerController();

	/** Начать осмотр предмета */
	UFUNCTION(BlueprintCallable, Category = "Inspect")
	void StartInspect(UStaticMesh* ItemMesh);

	/** Закончить осмотр */
	UFUNCTION(BlueprintCallable, Category = "Inspect")
	void EndInspect();

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category = "Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	// ---------- Осмотр предметов ----------

	UPROPERTY(EditDefaultsOnly, Category = "Inspect")
	TSubclassOf<AInspectActor> InspectActorClass;

	UPROPERTY(EditDefaultsOnly, Category = "Inspect")
	UInputMappingContext* InspectMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Inspect")
	UInputAction* InspectAction;

	UPROPERTY(EditDefaultsOnly, Category = "Inspect")
	UInputAction* ZoomAction;

	UPROPERTY()
	AInspectActor* InspectActor;

	void OnInspectInput(const FInputActionValue& Value);
	void OnZoomInput(const FInputActionValue& Value);

	// --------------------------------------

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;
};