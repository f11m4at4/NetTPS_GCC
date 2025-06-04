// Fill out your copyright notice in the Description page of Project Settings.


#include "NetGameInstance.h"

#include "NetTPS.h"
#include "OnlineSessionSettings.h"

void UNetGameInstance::Init()
{
	Super::Init();

	if (auto subsys = IOnlineSubsystem::Get())
	{
		sessionInterface = subsys->GetSessionInterface();

		// 세션 이벤트 콜백 등록
		sessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UNetGameInstance::OnCreateSessionCompelete);
	}

	// 세션 생성
	FTimerHandle handle;
	GetWorld()->GetTimerManager().SetTimer(handle,
		FTimerDelegate::CreateLambda([&]
	{
		CreateMySession(mySessionName, 10);
	}
	), 2, false);
}

void UNetGameInstance::CreateMySession(FString roomName, int32 playerCount)
{
	// 방을 만들려면 몇명, 비밀방인지 등을 설정할 데이터가 필요하다.
	FOnlineSessionSettings sessionSettings;

	// 1. Dedicated server 접속여부
	sessionSettings.bIsDedicated = false;

	// 2. 로컬(랜선)매칭 할지 혹은 Steam 매칭을 사용할지 여부
	FName subsysName = IOnlineSubsystem::Get()->GetSubsystemName();
	sessionSettings.bIsLANMatch = subsysName == "NULL";

	// 3. 매칭이 온라인을 통해 노출될지 여부
	sessionSettings.bShouldAdvertise = true;

	// 4. 나의 온라인 상태(presence) 정보를 활용하게 해줄지 여부
	sessionSettings.bUsesPresence = true;
	// 5. 로비 사용여부
	sessionSettings.bUseLobbiesIfAvailable = true;
	// 6. 게임진행중에 참여 허가할지 여부
	sessionSettings.bAllowJoinViaPresence = true;
	sessionSettings.bAllowJoinInProgress = true;

	// 7. 세션에 참여할 수 있는 최대 참여자 수
	sessionSettings.NumPublicConnections = playerCount;

	// 8. 커스텀 룸네임 설정
	mySessionName = roomName;
	sessionSettings.Set(FName("ROOM_NAME"), mySessionName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	// 9. 호스트네임 설정
	sessionSettings.Set(FName("HOST_NAME"), myHostName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// NetID
	FUniqueNetIdPtr netID = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();

	PRINTLOG(TEXT("Create Session Start : %s"), *mySessionName);
	sessionInterface->CreateSession(*netID, FName(mySessionName), sessionSettings);
}

void UNetGameInstance::OnCreateSessionCompelete(FName sessionName, bool bWasSuccessful)
{
	PRINTLOG(TEXT("SessionName : %s, bWasSuccessful : %d"), *sessionName.ToString(), bWasSuccessful);
}
