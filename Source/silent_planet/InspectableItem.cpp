#include "InspectableItem.h"
#include "Components/StaticMeshComponent.h"

AInspectableItem::AInspectableItem()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);
    Mesh->SetCollisionProfileName(TEXT("BlockAll"));
}

void AInspectableItem::SetHeldState(bool bHeld)
{
    SetActorHiddenInGame(bHeld);
    SetActorEnableCollision(!bHeld);
}

