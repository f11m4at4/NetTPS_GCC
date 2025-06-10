// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NetPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class NETTPS_API ANetPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	// 사용자 이름을 서버에 알려주기 위한 RPC 함수
	UFUNCTION(Server, Reliable)
	void ServerRPC_SetUserName(const FString& name);
};
