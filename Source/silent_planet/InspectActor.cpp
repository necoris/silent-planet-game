#include "InspectActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

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

// Новая функция: запоминаем, где предмет появился
// New function: remember where the item spawned
void AInspectActor::BeginPlay()
{
    Super::BeginPlay();
    BaseLocation = GetActorLocation();
}

void AInspectActor::SetupFromComponent(UStaticMeshComponent* Source)
{
    if (!Source || !Source->GetStaticMesh())
    {
        return;
    }

    UStaticMesh* SM = Source->GetStaticMesh();
    Mesh->SetStaticMesh(SM);

    // Копируем материалы с предмета в мире // Copy materials from the world item
    for (int32 i = 0; i < Source->GetNumMaterials(); ++i)
    {
        Mesh->SetMaterial(i, Source->GetMaterial(i));
    }

    // Подгоняем размер, чтобы любой предмет был одного размера в руках
    // Fit the size so any item looks the same size in hands
    const FBoxSphereBounds B = SM->GetBounds();
    const float Scale = (B.SphereRadius > KINDA_SMALL_NUMBER) ? TargetRadius / B.SphereRadius : 1.f;

    Mesh->SetRelativeScale3D(FVector(Scale));
    // Центрируем, чтобы крутился вокруг своей середины // Center it to rotate around its middle
    Mesh->SetRelativeLocation(-B.Origin * Scale);
}

void AInspectActor::SetMesh(UStaticMesh* NewMesh)
{
    Mesh->SetStaticMesh(NewMesh);

    // Центрируем меш по его bounds // Center the mesh based on its bounds
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

    // 1. Превращаем ввод в поворот // 1. Convert input into a rotation
    const float YawDeg = -PendingInput.X * RotationSpeed * DeltaSeconds;
    const float PitchDeg = PendingInput.Y * RotationSpeed * DeltaSeconds;
    PendingInput = FVector2D::ZeroVector;

    const FQuat YawQuat(FVector::UpVector, FMath::DegreesToRadians(YawDeg));
    const FQuat PitchQuat(FVector::RightVector, FMath::DegreesToRadians(PitchDeg));

    // Новые повороты умножаются слева // New rotations are multiplied from the left
    TargetRotation = YawQuat * PitchQuat * TargetRotation;
    TargetRotation.Normalize();

    // 2. Плавно догоняем целевой поворот // 2. Smoothly catch up to the target rotation
    const FQuat Current = Pivot->GetRelativeRotation().Quaternion();
    const FQuat NewRot = FQuat::Slerp(Current, TargetRotation,
        FMath::Clamp(DeltaSeconds * RotationInterpSpeed, 0.f, 1.f));
    Pivot->SetRelativeRotation(NewRot);

    // 3. Зум вдоль взгляда камеры // 3. Zoom along the camera view direction
    CurrentZoom = FMath::FInterpTo(CurrentZoom, TargetZoom, DeltaSeconds, 10.f);
    SetActorLocation(BaseLocation + GetActorForwardVector() * CurrentZoom);
}