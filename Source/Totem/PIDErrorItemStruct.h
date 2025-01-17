#pragma once

#include "PIDErrorItemStruct.generated.h"

USTRUCT(BlueprintType)
struct FPIDErrorItemStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PID Controller")
	float MeasuredValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PID Controller")
	float DeltaTime;

	//Default constructor
	FPIDErrorItemStruct()
	{
		MeasuredValue = 0.0f;
		DeltaTime = 0.0f;
	}
};
