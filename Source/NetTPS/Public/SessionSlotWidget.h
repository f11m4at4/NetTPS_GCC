// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SessionSlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class NETTPS_API USessionSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* txt_roomName;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* txt_hostName;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* txt_playerCount;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* txt_pingSpeed;

	int32 sessionNumber = 0;

	void Set(const struct FSessionInfo& sessionInfo);
};
