#include "InspectActor.h"
#include "Components/StaticMeshComponent.h"

AInspectActor::AInspectActor()
{
    PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    Pivot = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
    Pivot->SetupAttachment(Root);

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(Pivot);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    TargetRotation = FQuat::Identity;
}

void AInspectActor::SetMesh(UStaticMesh* NewMesh)
{
    Mesh->SetStaticMesh(NewMesh);

    // Центрируем меш по его bounds, чтобы он крутился вокруг своего центра
    // Center the mesh based on its bounds so that it rotates around its center
    const FVector Center = Mesh->Bounds.Origin - Mesh->GetComponentLocation();
    Mesh->SetRelativeLocation(-Center);
}

void AInspectActor::AddInspectInput(const FVector2D& Input)
{
    PendingInput += Input;
}

void AInspectActor::AddZoomInput(float Value)
{
    TargetZoom = FMath::Clamp(TargetZoom + Value, MinZoom, MaxZoom);
}

void AInspectActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // 1. Превращаем ввод в поворот.
    //    Ось Yaw — мировая "вверх" (Z), ось Pitch — "вправо" относительно камеры.
    //    Камера смотрит по +X, поэтому "вправо" для неё это +Y.
    // 1. Convert input into a rotation. 
    //    The Yaw axis is the world "up" (Z), and the Pitch axis is "to the right" relative to the camera. 
    //    The camera faces along +X, so "to the right" from its perspective is +Y.
    const float YawDeg = -PendingInput.X * RotationSpeed * DeltaSeconds;
    const float PitchDeg = PendingInput.Y * RotationSpeed * DeltaSeconds;
    PendingInput = FVector2D::ZeroVector;

    const FQuat YawQuat(FVector::UpVector, FMath::DegreesToRadians(YawDeg));
    const FQuat PitchQuat(FVector::RightVector, FMath::DegreesToRadians(PitchDeg));

    // Порядок важен: новые повороты умножаются СЛЕВА, т.е. применяются в мировых осях.
    // Order matters: new rotations are multiplied from the LEFT, i.e., they are applied in world-space axes.
    TargetRotation = YawQuat * PitchQuat * TargetRotation;
    TargetRotation.Normalize();

    // 2. Плавно догоняем целевой поворот
    // 2. Smoothly catch up to the target rotation
    const FQuat Current = Pivot->GetRelativeRotation().Quaternion();
    const FQuat NewRot = FQuat::Slerp(Current, TargetRotation,
        FMath::Clamp(DeltaSeconds * RotationInterpSpeed, 0.f, 1.f));
    Pivot->SetRelativeRotation(NewRot);

    // 3. Зум (двигаем актор вдоль оси к камере)
    // 3. Zoom (move the actor along the axis toward the camera)
    FVector Loc = Root->GetRelativeLocation();
    Loc.X = FMath::FInterpTo(Loc.X, TargetZoom, DeltaSeconds, 10.f);
    Root->SetRelativeLocation(Loc);
}