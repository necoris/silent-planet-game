// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "ShooterPickup.generated.h"

class USphereComponent;
class UPrimitiveComponent;
class AShooterWeapon;

/**
 *  Holds information about a type of weapon pickup
 */
USTRUCT(BlueprintType)
struct FWeaponTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Mesh to display on the pickup */
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UStaticMesh> StaticMesh;

	/** Weapon class to grant on pickup */
	UPROPERTY(EditAnywhere)
	TSubclassOf<AShooterWeapon> WeaponToSpawn;
};

/**
 *  Simple shooter game weapon pickup
 */
UCLASS(abstract)
class SILENT_PLANET_API AShooterPickup : public AActor
{
	GENERATED_BODY()

	/** Collision sphere */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* SphereCollision;

	/** Weapon pickup mesh. Its mesh asset is set from the weapon data table */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;

protected:

	/** Data on the type of picked weapon and visuals of this pickup */
	UPROPERTY(EditAnywhere, Category = "Pickup")
	FDataTableRowHandle WeaponType;

	/** Type to weapon to grant on pickup. Set from the weapon data table. */
	TSubclassOf<AShooterWeapon> WeaponClass;

	// true = подбирать только на E, false = подбирать, наступив на предмет
	// true = pick up only with E, false = pick up by walking over it
	UPROPERTY(EditAnywhere, Category = "Pickup")
	bool bPickupOnInteract = true;

	// true = предмет появится снова через RespawnTime, false = исчезнет навсегда
	// true = the pickup respawns after RespawnTime, false = it disappears forever
	UPROPERTY(EditAnywhere, Category = "Pickup")
	bool bRespawn = false;

	/** Time to wait before respawning this pickup */
	UPROPERTY(EditAnywhere, Category = "Pickup", meta = (ClampMin = 0, ClampMax = 120, Units = "s"))
	float RespawnTime = 4.0f;

	/** Timer to respawn the pickup */
	FTimerHandle RespawnTimer;

public:

	/** Constructor */
	AShooterPickup();

	// Отдать оружие тому, кто подбирает (вызывается по E или при наступании)
	// Give the weapon to whoever picks it up (called by E or by overlap)
	bool TryPickup(AActor* Taker);

	// Название и описание, которые покажутся при осмотре (задаются на каждом пикапе в редакторе)
	// Name and description shown while inspecting (set per pickup in the editor)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (MultiLine = true))
	FText ItemDescription;

	// Меш пикапа, нужен для осмотра // Pickup mesh, needed for inspecting
	UStaticMeshComponent* GetPickupMesh() const { return Mesh; }

	// Спрятать пикап, пока его осматривают, и вернуть на стойку
	// Hide the pickup while it is inspected, and put it back on the stand
	void SetHeldState(bool bHeld);

protected:

	/** Native construction script */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Gameplay Initialization*/
	virtual void BeginPlay() override;

	/** Gameplay cleanup */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Handles collision overlap */
	UFUNCTION()
	virtual void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:

	/** Called when it's time to respawn this pickup */
	void RespawnPickup();

	/** Passes control to Blueprint to animate the pickup respawn. Should end by calling FinishRespawn */
	UFUNCTION(BlueprintImplementableEvent, Category = "Pickup", meta = (DisplayName = "OnRespawn"))
	void BP_OnRespawn();

	/** Enables this pickup after respawning */
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void FinishRespawn();
};