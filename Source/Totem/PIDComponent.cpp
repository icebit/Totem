// Fill out your copyright notice in the Description page of Project Settings.


#include "PIDComponent.h"

// Sets default values
UPIDComponent::UPIDComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	NumOfErrorsToStore = 5;
	ProportionalCoefficient = 0.1f;
	IntegralCoefficient = 0.0f;
	DerivativeCoefficient = 0.05f;
}


// Called when the game starts or when spawned
void UPIDComponent::BeginPlay()
{
	MeasuredErrors.Reserve(NumOfErrorsToStore);

	Super::BeginPlay();
}

// Add error measurement into array
void UPIDComponent::AddMeasuredError(const FPIDErrorItemStruct& PIDErrorItem)
{
	MeasuredErrors.Insert(PIDErrorItem, 0);

	// remove last item in array if we reached desired amount of errors to track
	if (MeasuredErrors.Num() > NumOfErrorsToStore)
	{
		MeasuredErrors.Pop(true);
	}
}

// Add error measurement into array
void UPIDComponent::AddError(float MeasuredValue, float DeltaTime)
{
	FPIDErrorItemStruct ErrorItem = FPIDErrorItemStruct();
	ErrorItem.MeasuredValue = MeasuredValue;
	ErrorItem.DeltaTime = DeltaTime;
	AddMeasuredError(ErrorItem);
}

//Calculate and return control signal
float UPIDComponent::GetControlCoefficient()
{
	float Result = 0.0f;

	if (MeasuredErrors.Num() < 3)
		return Result;

	// Proportional error is just the last measured error
	if (ProportionalCoefficient != 0.0f)
	{
		Result += ProportionalCoefficient * MeasuredErrors[0].MeasuredValue;
	}

	// Integral error is a sum of all measured errors multiplied by respective delta time of measurement
	if (IntegralCoefficient != 0.0f)
	{
		float IntegralError = 0.0f;

		for (int32 i = 0; i < MeasuredErrors.Num(); i++)
		{
			IntegralError += MeasuredErrors[i].MeasuredValue * MeasuredErrors[i].DeltaTime;
		}

		Result += IntegralCoefficient * IntegralError;
	}

	// Derivative error is difference between last two error divided by delta time of last measurement
	if (DerivativeCoefficient != 0.0f)
	{
		Result += DerivativeCoefficient * (MeasuredErrors[0].MeasuredValue - MeasuredErrors[1].MeasuredValue) / MeasuredErrors[0].DeltaTime;
	}

	return Result;
}

//Clear array of measured errors
void UPIDComponent::ResetController()
{
	MeasuredErrors.Empty();
}
