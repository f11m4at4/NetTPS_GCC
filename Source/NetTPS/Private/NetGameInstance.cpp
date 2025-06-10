// Fill out your copyright notice in the Description page of Project Settings.


#include "NetGameInstance.h"

#include "NetTPS.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

void UNetGameInstance::Init()
{
	Super::Init();

	if (auto subsys = IOnlineSubsystem::Get())
	{
		sessionInterface = subsys->GetSessionInterface();

		// 세션 이벤트 콜백 등록
		sessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UNetGameInstance::OnCreateSessionCompelete);
		// 세션 검색 이벤트 등록
		sessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UNetGameInstance::OnFindSessionsComplete);
		// 세션 입장 이벤트 등록
		sessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UNetGameInstance::OnJoinSessionComplete);
		// 세션 종료 이벤트 등록
		sessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UNetGameInstance::OnMyExitRoomComplete);
	}

	// 세션 생성
	// FTimerHandle handle;
	// GetWorld()->GetTimerManager().SetTimer(handle,
	// 	FTimerDelegate::CreateLambda([&]
	// {
	// 	// CreateMySession(mySessionName, 10);
	// 		FindOtherSessions();
	// }
	// ), 2, false);
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
	sessionSettings.Set(FName("HOST_NAME"), myName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// NetID
	FUniqueNetIdPtr netID = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();

	PRINTLOG(TEXT("Create Session Start : %s"), *mySessionName);
	sessionInterface->CreateSession(*netID, FName(mySessionName), sessionSettings);
}

void UNetGameInstance::OnCreateSessionCompelete(FName sessionName, bool bWasSuccessful)
{
	PRINTLOG(TEXT("SessionName : %s, bWasSuccessful : %d"), *sessionName.ToString(), bWasSuccessful);
	// 방만드는게 성공하면
	// -> Server Travel (Game Server open)
	if (bWasSuccessful)
	{
		GetWorld()->ServerTravel(TEXT("/Game/Net/Maps/BattleMap?listen?port=7777"));
		FString url;
		sessionInterface->GetResolvedConnectString(sessionName, url);
		PRINTLOG(TEXT("URL : %s"), *url);
	}
}

void UNetGameInstance::FindOtherSessions()
{
	// 검색 시작 : 이때 화면 비활성화 시켜줘야한다.
	OnSearchState.Broadcast(true);
	
	// 찾을 조건들을 설정
	sessionSearch = MakeShareable(new FOnlineSessionSearch());

	// 1. 존재여부를 검색 가능하게 해 놓은 녀석만 찾자.
	sessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	// 2. Lan 사용여부
	sessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == FName("NULL");
	// 3. 최대 검색 세션(방) 수
	sessionSearch->MaxSearchResults = 10;
	// 4. 세션 검색
	sessionInterface->FindSessions(0, sessionSearch.ToSharedRef());
}

void UNetGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	// 로비위젯 방검색 비활성화 종료
	OnSearchState.Broadcast(false);
	
	// 찾기 실패시 아무것도 하지 말자.
	if (bWasSuccessful == false)
	{
		PRINTLOG(TEXT("Session search failed!!!"));
		return;
	}

	// 세션 검색결과 배열
	auto results = sessionSearch->SearchResults;

	PRINTLOG(TEXT("Search Result Count : %d"), results.Num());

	for (int i=0; i < results.Num(); i++)
	{
		auto sr = results[i];
		if (sr.IsValid() == false)
		{
			continue;
		}

		// 세션 정보 저장할 구조체
		FSessionInfo sessionInfo;
		sessionInfo.index = i;
		
		// 방이름
		sr.Session.SessionSettings.Get(FName("ROOM_NAME"), sessionInfo.roomName);
		sr.Session.SessionSettings.Get(FName("HOST_NAME"), sessionInfo.hostName);

		// 최대 입장가능한 플레이어 수
		int32 maxPlayerCount = sr.Session.SessionSettings.NumPublicConnections; 
		// 현재 입장한 플레이어수 (최대 - 현재 입장가능한 수)
		int32 currentPlayerCount = maxPlayerCount - sr.Session.NumOpenPublicConnections;

		sessionInfo.playerCount = FString::Printf(TEXT("(%d/%d)"), currentPlayerCount, maxPlayerCount);
		
		// 핑정보
		sessionInfo.pingSpeed = sr.PingInMs;

		PRINTLOG(TEXT("%s"), *sessionInfo.ToString());

		// 델리게이트로 위젯에 알려주기
		OnSearchComplete.Broadcast(sessionInfo);
	}
}

void UNetGameInstance::JoinSelectedSession(int32 index)
{
	// 검색 결과 목록(배열)중 index 번째 녀석으로 방 입장하기
	auto sr = sessionSearch->SearchResults[index];
	sr.Session.SessionSettings.Get(FName("ROOM_NAME"), mySessionName);
	sr.Session.SessionSettings.bUseLobbiesIfAvailable = true;
	sessionInterface->JoinSession(0, FName(mySessionName), sr);
}

void UNetGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		// 방으로 Client Travel
		auto pc = GetWorld()->GetFirstPlayerController();
		FString url;
		sessionInterface->GetResolvedConnectString(SessionName, url);
		PRINTLOG(TEXT("Join session URL : %s"), *url);
		if (url.IsEmpty() == false)
		{
			pc->ClientTravel(url, TRAVEL_Absolute);
		}
	}
	else
	{
		PRINTLOG(TEXT("JoinSessionComplete Failed : %d"), Result);
	}
}

void UNetGameInstance::ExitRoom()
{
}

void UNetGameInstance::ServerRPC_ExitRoom_Implementation()
{
}

void UNetGameInstance::MultiRPC_ExitRoom_Implementation()
{
}

void UNetGameInstance::OnMyExitRoomComplete(FName sessionName, bool bWasSuccessful)
{
}





