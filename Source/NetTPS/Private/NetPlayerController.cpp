// Fill out your copyright notice in the Description page of Project Settings.


#include "NetPlayerController.h"

#include "NetTPSGameMode.h"

void ANetPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Server 일때만 게임모드가 존재한다.
	if (HasAuthority())
		gm = Cast<ANetTPSGameMode>(GetWorld()->GetAuthGameMode());
}

void ANetPlayerController::ServerRPC_RespawnPlayer_Implementation()
{
	// 기존 pawn 은 어떻게해???
	auto player = GetPawn();
	// -> possess 를 풀어야한다.
	UnPossess();
	// -> 삭제해야한다.
	player->Destroy();
	// 새로운 pawn 을 스폰하고 싶다.
	gm->RestartPlayer(this);
}
