#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InspectableItem.generated.h"

class UStaticMeshComponent;

UCLASS()
class SILENT_PLANET_API AInspectableItem : public AActor
{
    GENERATED_BODY()

public:
    AInspectableItem();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
    UStaticMeshComponent* Mesh;

    // Название и описание, которые покажутся внизу экрана и в инвентаре
    // Name and description shown at the bottom of the screen and in the inventory
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FText ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (MultiLine = true))
    FText ItemDescription;

    // Сколько штук лежит в этой куче // How many pieces are in this pile
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ClampMin = 1))
    int32 Quantity = 1;

    // Максимум в одной ячейке инвентаря (1 = не складывается)
    // Max per inventory slot (1 = does not stack)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ClampMin = 1))
    int32 MaxStack = 1;

    // Прячет/показывает предмет в мире, пока его крутят в руках
    // Hides/shows the item in the world while it is rotated in hands
    void SetHeldState(bool bHeld);
};