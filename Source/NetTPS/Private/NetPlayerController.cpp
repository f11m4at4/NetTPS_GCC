// Fill out your copyright notice in the Description page of Project Settings.


#include "NetPlayerController.h"

#include "NetTPSGameMode.h"
#include "MainUI.h"
#include "NetGameInstance.h"
#include "NetTPS.h"
#include "GameFramework/SpectatorPawn.h"

ANetPlayerController::ANetPlayerController()
{
	ConstructorHelpers::FClassFinder<UMainUI> tempMainUI(TEXT("'/Game/Net/UIs/WBP_MainUI.WBP_MainUI_C'"));
	if (tempMainUI.Succeeded())
	{
		mainUIWidget = tempMainUI.Class;
	}
}

void ANetPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Server 일때만 게임모드가 존재한다.
	if (HasAuthority())
		gm = Cast<ANetTPSGameMode>(GetWorld()->GetAuthGameMode());
}

void ANetPlayerController::ServerRPC_ChangeToSpectator_Implementation()
{
	// 관전자가 플레이어의 위치에서 생성될 수 있도록 플레이어 정보를 가져온다.
	auto player = GetPawn();
	if (player)
	{
		FActorSpawnParameters param;
		param.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		// 1. 관전자 객체 만들기
		auto spectator = GetWorld()->SpawnActor<ASpectatorPawn>(gm->SpectatorClass, player->GetActorTransform(), param );
		// 2. 관전자를 possess 한다.
		Possess(spectator);

		// 3. 이전 플레이어는 제거
		player->Destroy();

		// 5초후에 리스폰 시키기
		FTimerHandle handle;
		GetWorldTimerManager().SetTimer(handle, this, &ANetPlayerController::ServerRPC_RespawnPlayer_Implementation, 5, false);
	}
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
