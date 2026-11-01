#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InspectActor.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UStaticMesh;

UCLASS()
class SILENT_PLANET_API AInspectActor : public AActor
{
    GENERATED_BODY()

public:
    AInspectActor();

    // Настроить предмет по мешу из мира // Set up from the world item's mesh
    void SetupFromComponent(UStaticMeshComponent* Source);

    // Ввод вращения: X = вправо/влево, Y = вверх/вниз // Rotation input: X = right/left, Y = up/down
    void AddInspectInput(const FVector2D& Input);
    void AddZoomInput(float Value);

    void SetMesh(UStaticMesh* NewMesh);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere)
    USceneComponent* Root;

    // Pivot нужен, чтобы вращать вокруг центра предмета // Pivot is used to rotate around the item's center
    UPROPERTY(VisibleAnywhere)
    USceneComponent* Pivot;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;

    UPROPERTY(EditAnywhere, Category = "Inspect")
    float RotationSpeed = 120.f;   // градусов в секунду // degrees per second

    UPROPERTY(EditAnywhere, Category = "Inspect")
    float RotationInterpSpeed = 12.f;  // плавность // smoothness

    UPROPERTY(EditAnywhere, Category = "Inspect")
    float MinZoom = -40.f;

    UPROPERTY(EditAnywhere, Category = "Inspect")
    float MaxZoom = 40.f;

    // Размер предмета в руках (радиус) // Item size in hands (radius)
    UPROPERTY(EditAnywhere, Category = "Inspect")
    float TargetRadius = 25.f;

private:
    FQuat TargetRotation;
    FVector2D PendingInput = FVector2D::ZeroVector;
    float TargetZoom = 0.f;

    // Точка, где предмет появился // Where the item spawned
    FVector BaseLocation = FVector::ZeroVector;
    // Текущее смещение зума // Current zoom offset
    float CurrentZoom = 0.f;
};