// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NetPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class NETTPS_API ANetPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	ANetPlayerController();
	
private:
	UPROPERTY()
	class ANetTPSGameMode* gm;

public: // -------------- UI -----------------
	UPROPERTY(EditDefaultsOnly, Category=UI)
	TSubclassOf<class UMainUI> mainUIWidget;
	UPROPERTY()
	class UMainUI* mainUI;
	
public:
	virtual void BeginPlay() override;

public: // ------------- 리스폰 rpc--------------
	UFUNCTION(Server, Reliable)
	void ServerRPC_RespawnPlayer();

public: // ------------- 관전자 ----------------
	UFUNCTION(Server, Reliable)
	void ServerRPC_ChangeToSpectator();
};
