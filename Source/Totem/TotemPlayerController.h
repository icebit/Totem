// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TotemPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class TOTEM_API ATotemPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
  UFUNCTION(BlueprintNativeEvent)
  void OnDeath();

  UFUNCTION(BlueprintNativeEvent)
  void NotifyOutOfAmmo();

  UFUNCTION(BlueprintNativeEvent)
  void NotifyHitEnemy();
};
