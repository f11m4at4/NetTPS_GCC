// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "NetPlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class NETTPS_API UNetPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	// 총을 소지하고 있는지 여부 속성
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MyAnimSettings)
	bool bHasPistol = false;
};
