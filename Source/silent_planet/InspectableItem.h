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

    // Название и описание, которые покажутся внизу экрана
    // Title and description that will appear at the bottom of the screen
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FText ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (MultiLine = true))
    FText ItemDescription;

    // Прячет/показывает предмет в мире, пока его крутят в руках
    // Hides/shows an item in the world while it is being rotated in one's hands
    void SetHeldState(bool bHeld);
};