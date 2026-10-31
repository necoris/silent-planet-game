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

    // Вызывается из контроллера каждый кадр: Input.X = вправо/влево, Input.Y = вверх/вниз
    // Called from the controller every frame: Input.X = right/left, Input.Y = up/down
    void AddInspectInput(const FVector2D& Input);
    void AddZoomInput(float Value);

    void SetMesh(UStaticMesh* NewMesh);

protected:
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere)
    USceneComponent* Root;

    // Pivot нужен, чтобы вращать вокруг центра предмета, а не вокруг его origin
    // Pivot is needed to rotate around the center of the object, not around its origin
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

private:
    FQuat TargetRotation;
    FVector2D PendingInput = FVector2D::ZeroVector;
    float TargetZoom = 0.f;   // смещение по оси X (к камере / от камеры) // offset along the X-axis (towards the camera / away from the camera)
};
