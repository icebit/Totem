#pragma once

#include "CoreMinimal.h"
#include "PIDErrorItemStruct.h"
#include "Components/ActorComponent.h"
#include "PIDComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TOTEM_API UPIDComponent : public UActorComponent
{
  GENERATED_BODY()
public:
	// Sets default values for this component
	UPIDComponent();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/**
	* Minimum of 3 errors need to be stored. Storing more errors is necessary for Integral part of the controller
	* as it corrects systematic errors of measurements taken over time.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PID Settings", meta = (ClampMin = "3", UIMin = "3"), meta = (ToolTip = "Minimum of 3 errors need to be stored. Storing more errors is necessary for Integral part of the controller as it corrects systematic errors of measurements taken over time."))
	int32 NumOfErrorsToStore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PID Settings", meta = (ToolTip = "Proportional coefficient is responsible for how fast error should be corrected"))
	float ProportionalCoefficient;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PID Settings", meta = (ToolTip = "Integral coefficient is responsible for correcting systematic errors of measurement over time"))
	float IntegralCoefficient;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PID Settings", meta = (ToolTip = "Derivative coefficient is responsible for correcting oscillation introduced by derivative coefficient"))
	float DerivativeCoefficient;

	/**
	*	Add measurement error and delta time to PID Controller's memory
	*	@param PIDErrorItem	measurement error and delta time item
	*/
	UFUNCTION(BlueprintCallable, Category = "PID Controller")
	void AddMeasuredError(const FPIDErrorItemStruct& PIDErrorItem);

	/**
	*	Add measurement error and delta time to PID Controller's memory
	*	@param PIDErrorItem	measurement error and delta time item
	*/
	UFUNCTION(BlueprintCallable, Category = "PID Controller")
	void AddError(float MeasuredValue, float DeltaTime);

	/**
	*	Get calculated control signal
	*	@return calculated control signal
	*/
	UFUNCTION(BlueprintCallable, Category = "PID Controller")
	float GetControlCoefficient();

	/**
	*	Resets PID Controller by deleting all previously measured errors
	*/
	UFUNCTION(BlueprintCallable, Category = "PID Controller")
	void ResetController();

private:
	UPROPERTY()
	TArray<FPIDErrorItemStruct> MeasuredErrors;
};
