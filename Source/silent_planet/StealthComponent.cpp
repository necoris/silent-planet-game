#include "StealthComponent.h"
#include "silent_planetCharacter.h"
#include "silent_planetPlayerController.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"

UStealthComponent::UStealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStealthComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerChar = Cast<Asilent_planetCharacter>(GetOwner());
	if (!OwnerChar)
	{
		return;
	}

	FPMesh = OwnerChar->GetFirstPersonMesh();
	Camera = OwnerChar->GetFirstPersonCameraComponent();

	if (FPMesh)
	{
		BaseRelativeLocation = FPMesh->GetRelativeLocation();
	}

	// Разрешаем приседание и держим низ капсулы на земле
	// Allow crouching and keep the capsule bottom on the ground
	UCharacterMovementComponent* CMC = OwnerChar->GetCharacterMovement();
	CMC->NavAgentProps.bCanCrouch = true;
	CMC->bCrouchMaintainsBaseLocation = true;
	// Было: CMC->CrouchedHalfHeight = CrouchHalfHeight;
	// Новый способ задать высоту капсулы в приседе // New way to set the crouched capsule height
	CMC->SetCrouchedHalfHeight(CrouchHalfHeight);
	CMC->MaxWalkSpeedCrouched = CrouchSpeed;

	// Лёжа капсула не может быть ниже своего радиуса
	// When prone the capsule can't be shorter than its radius
	ProneHalfHeight = FMath::Max(ProneHalfHeight, OwnerChar->GetCapsuleComponent()->GetUnscaledCapsuleRadius());
}

void UStealthComponent::SetupInput(UEnhancedInputComponent* EIC)
{
	if (!EIC)
	{
		return;
	}

	if (CrouchAction)
	{
		EIC->BindAction(CrouchAction, ETriggerEvent::Started, this, &UStealthComponent::ToggleCrouch);
	}
	if (ProneAction)
	{
		EIC->BindAction(ProneAction, ETriggerEvent::Started, this, &UStealthComponent::ToggleProne);
	}
	if (LeanLeftAction)
	{
		EIC->BindAction(LeanLeftAction, ETriggerEvent::Started, this, &UStealthComponent::LeanLeftStarted);
		EIC->BindAction(LeanLeftAction, ETriggerEvent::Completed, this, &UStealthComponent::LeanLeftStopped);
		EIC->BindAction(LeanLeftAction, ETriggerEvent::Canceled, this, &UStealthComponent::LeanLeftStopped);
	}
	if (LeanRightAction)
	{
		EIC->BindAction(LeanRightAction, ETriggerEvent::Started, this, &UStealthComponent::LeanRightStarted);
		EIC->BindAction(LeanRightAction, ETriggerEvent::Completed, this, &UStealthComponent::LeanRightStopped);
		EIC->BindAction(LeanRightAction, ETriggerEvent::Canceled, this, &UStealthComponent::LeanRightStopped);
	}
}

bool UStealthComponent::CanChangeStance() const
{
	if (!OwnerChar)
	{
		return false;
	}

	// Пока осматриваем предмет, позу менять нельзя
	// The stance can't change while an item is being inspected
	if (const Asilent_planetPlayerController* PC = Cast<Asilent_planetPlayerController>(OwnerChar->GetController()))
	{
		if (PC->IsInspecting())
		{
			return false;
		}
	}

	return true;
}

EStealthStance UStealthComponent::GetStance() const
{
	if (!OwnerChar || !OwnerChar->bIsCrouched)
	{
		return EStealthStance::Standing;
	}

	// Различаем присед и лёжа по высоте капсулы
	// Tell crouch from prone by the capsule height
	const float HalfHeight = OwnerChar->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	return (HalfHeight <= ProneHalfHeight + 1.0f) ? EStealthStance::Prone : EStealthStance::Crouching;
}

float UStealthComponent::GetNoiseMultiplier() const
{
	switch (GetStance())
	{
	case EStealthStance::Crouching:
		return CrouchNoise;
	case EStealthStance::Prone:
		return ProneNoise;
	default:
		return StandingNoise;
	}
}

void UStealthComponent::ToggleCrouch()
{
	// Из положения лёжа тоже переходим в присед
	// From prone we also go to crouch
	SetStance(GetStance() == EStealthStance::Crouching ? EStealthStance::Standing : EStealthStance::Crouching);
}

void UStealthComponent::ToggleProne()
{
	SetStance(GetStance() == EStealthStance::Prone ? EStealthStance::Standing : EStealthStance::Prone);
}

void UStealthComponent::StandUp()
{
	SetStance(EStealthStance::Standing);
}

void UStealthComponent::SetStance(EStealthStance NewStance)
{
	if (!OwnerChar || !CanChangeStance())
	{
		return;
	}

	UCharacterMovementComponent* CMC = OwnerChar->GetCharacterMovement();

	if (NewStance == EStealthStance::Standing)
	{
		// Если над головой потолок, движок сам попробует встать позже
		// If there is a ceiling above, the engine retries standing up later
		OwnerChar->UnCrouch();
		return;
	}

	// Задаём высоту и скорость выбранной позы
	// Set the height and speed of the chosen stance
	const bool bProne = (NewStance == EStealthStance::Prone);
	// Было: CMC->CrouchedHalfHeight = bProne ? ProneHalfHeight : CrouchHalfHeight;
	// Высота капсулы выбранной позы // Capsule height of the chosen stance
	CMC->SetCrouchedHalfHeight(bProne ? ProneHalfHeight : CrouchHalfHeight);
	CMC->MaxWalkSpeedCrouched = bProne ? ProneSpeed : CrouchSpeed;

	if (OwnerChar->bIsCrouched)
	{
		// Уже пригнулись: применяем новую высоту (присед <-> лёжа)
		// Already crouched: apply the new height (crouch <-> prone)
		CMC->bWantsToCrouch = true;
		CMC->Crouch(false);
	}
	else
	{
		OwnerChar->Crouch();
	}
}

void UStealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerChar || !FPMesh || !Camera)
	{
		return;
	}

	const EStealthStance Stance = GetStance();

	// 1. Опускаем камеру по позе // 1. Lower the camera by stance
	float TargetDrop = 0.f;
	if (Stance == EStealthStance::Crouching)
	{
		TargetDrop = CrouchCameraDrop;
	}
	else if (Stance == EStealthStance::Prone)
	{
		TargetDrop = ProneCameraDrop;
	}
	CurrentDrop = FMath::FInterpTo(CurrentDrop, TargetDrop, DeltaTime, CameraInterpSpeed);

	// 2. Наклон: лёжа и при осмотре не наклоняемся
	// 2. Lean: no leaning while prone or inspecting
	float WantedLean = 0.f;
	if (Stance != EStealthStance::Prone && CanChangeStance())
	{
		WantedLean = ((bLeanRight ? 1.f : 0.f) - (bLeanLeft ? 1.f : 0.f)) * LeanDistance;
	}

	// 3. Не даём камере уйти в стену // 3. Don't let the camera go into a wall
	const FVector Right = OwnerChar->GetActorRightVector();
	if (!FMath::IsNearlyZero(WantedLean))
	{
		const float Side = FMath::Sign(WantedLean);
		const FVector Start = Camera->GetComponentLocation() - Right * CurrentLean;
		const FVector End = Start + Right * Side * FMath::Abs(WantedLean);

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(OwnerChar);

		if (GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Camera, FCollisionShape::MakeSphere(12.f), Params))
		{
			WantedLean = Side * FMath::Max(0.f, Hit.Distance);
		}
	}
	CurrentLean = FMath::FInterpTo(CurrentLean, WantedLean, DeltaTime, LeanInterpSpeed);

	// 4. Сдвигаем меш с руками и камерой (смещение переводим в локальные координаты)
	// 4. Move the arms mesh with the camera (convert the offset to local space)
	const FVector WorldOffset = Right * CurrentLean + FVector(0.f, 0.f, -CurrentDrop);
	const USceneComponent* Parent = FPMesh->GetAttachParent();
	const FVector LocalOffset = Parent ? Parent->GetComponentTransform().InverseTransformVectorNoScale(WorldOffset) : WorldOffset;
	FPMesh->SetRelativeLocation(BaseRelativeLocation + LocalOffset);
}