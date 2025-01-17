// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseProjectile.h"
#include "ExplodingProjectile.generated.h"

/**
 * 
 */
UCLASS()
class TOTEM_API AExplodingProjectile : public ABaseProjectile
{
  GENERATED_BODY()

public:
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Explosion)
  float ExplosionRadius;

  //UFUNCTION()
  virtual void OnImpact(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
};
