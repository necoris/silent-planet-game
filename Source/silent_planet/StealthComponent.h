#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StealthComponent.generated.h"

class Asilent_planetCharacter;
class UInputAction;
class UEnhancedInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;

// Поза персонажа // Character stance
UENUM(BlueprintType)
enum class EStealthStance : uint8
{
	Standing UMETA(DisplayName = "Standing"),
	Crouching UMETA(DisplayName = "Crouching"),
	Prone UMETA(DisplayName = "Prone")
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SILENT_PLANET_API UStealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStealthComponent();

	// Привязать клавиши (вызывает персонаж) // Bind inputs (called by the character)
	void SetupInput(UEnhancedInputComponent* EIC);

	// Присесть / встать // Crouch / stand
	UFUNCTION(BlueprintCallable, Category = "Stealth")
	void ToggleCrouch();

	// Лечь / встать // Go prone / stand
	UFUNCTION(BlueprintCallable, Category = "Stealth")
	void ToggleProne();

	// Встать в полный рост // Stand up
	UFUNCTION(BlueprintCallable, Category = "Stealth")
	void StandUp();

	// Текущая поза // Current stance
	UFUNCTION(BlueprintPure, Category = "Stealth")
	EStealthStance GetStance() const;

	// Множитель шума для ИИ: 1 стоя, меньше в приседе и лёжа
	// Noise multiplier for AI: 1 standing, lower when crouched or prone
	UFUNCTION(BlueprintPure, Category = "Stealth")
	float GetNoiseMultiplier() const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ---------- Ввод // Input (назначаются в Blueprint персонажа // set in the character Blueprint) ----------

	UPROPERTY(EditAnywhere, Category = "Stealth|Input")
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, Category = "Stealth|Input")
	UInputAction* ProneAction;

	UPROPERTY(EditAnywhere, Category = "Stealth|Input")
	UInputAction* LeanLeftAction;

	UPROPERTY(EditAnywhere, Category = "Stealth|Input")
	UInputAction* LeanRightAction;

	// ---------- Присед // Crouch ----------

	// Половина высоты капсулы в приседе // Capsule half height when crouched
	UPROPERTY(EditAnywhere, Category = "Stealth|Crouch")
	float CrouchHalfHeight = 60.f;

	// Скорость в приседе // Speed when crouched
	UPROPERTY(EditAnywhere, Category = "Stealth|Crouch")
	float CrouchSpeed = 150.f;

	// На сколько опустить камеру в приседе (см) // How far to lower the camera when crouched (cm)
	UPROPERTY(EditAnywhere, Category = "Stealth|Crouch")
	float CrouchCameraDrop = 55.f;

	// ---------- Лёжа // Prone ----------

	// Не меньше радиуса капсулы (34) // Not smaller than the capsule radius (34)
	UPROPERTY(EditAnywhere, Category = "Stealth|Prone")
	float ProneHalfHeight = 34.f;

	UPROPERTY(EditAnywhere, Category = "Stealth|Prone")
	float ProneSpeed = 60.f;

	UPROPERTY(EditAnywhere, Category = "Stealth|Prone")
	float ProneCameraDrop = 120.f;

	// ---------- Камера и наклон // Camera and lean ----------

	// Плавность опускания камеры // Camera lowering smoothness
	UPROPERTY(EditAnywhere, Category = "Stealth|Camera")
	float CameraInterpSpeed = 10.f;

	// Насколько далеко наклоняться (см) // How far to lean (cm)
	UPROPERTY(EditAnywhere, Category = "Stealth|Lean")
	float LeanDistance = 35.f;

	// Плавность наклона // Lean smoothness
	UPROPERTY(EditAnywhere, Category = "Stealth|Lean")
	float LeanInterpSpeed = 8.f;

	// ---------- Шум // Noise ----------

	UPROPERTY(EditAnywhere, Category = "Stealth|Noise")
	float StandingNoise = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Stealth|Noise")
	float CrouchNoise = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Stealth|Noise")
	float ProneNoise = 0.2f;

private:
	// Применить позу // Apply a stance
	void SetStance(EStealthStance NewStance);

	// Можно ли сейчас менять позу // Can the stance change right now
	bool CanChangeStance() const;

	// Наклоны: нажали / отпустили // Leaning: pressed / released
	void LeanLeftStarted() { bLeanLeft = true; }
	void LeanLeftStopped() { bLeanLeft = false; }
	void LeanRightStarted() { bLeanRight = true; }
	void LeanRightStopped() { bLeanRight = false; }

	UPROPERTY()
	Asilent_planetCharacter* OwnerChar = nullptr;

	UPROPERTY()
	USkeletalMeshComponent* FPMesh = nullptr;

	UPROPERTY()
	UCameraComponent* Camera = nullptr;

	// Исходное положение меша от первого лица // Original first person mesh location
	FVector BaseRelativeLocation = FVector::ZeroVector;

	float CurrentDrop = 0.f;
	float CurrentLean = 0.f;
	bool bLeanLeft = false;
	bool bLeanRight = false;
};