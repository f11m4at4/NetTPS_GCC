// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "NetGameInstance.generated.h"

// 세션(방) 정보 저장할 구조체
USTRUCT(BlueprintType)
struct FSessionInfo
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	FString roomName;
	UPROPERTY(BlueprintReadOnly)
	FString hostName;
	UPROPERTY(BlueprintReadOnly)
	FString playerCount;
	UPROPERTY(BlueprintReadOnly)
	int32 pingSpeed=0;
	UPROPERTY(BlueprintReadOnly)
	int32 index=0;

	inline FString ToString() const
	{
		return FString::Printf(TEXT("[%d]%s : %s - %s, %dms"), index, *roomName, *hostName, *playerCount, pingSpeed);
	}
};

// 세션 검색 끝났을 때 호출될 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSearchComplete, const FSessionInfo&, sessionInfo);

// 세션 검색 중일때 -> 비활성화 시키고 싶다.
// -> 이때 사용될 델리게이트 필요
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSearchStateSignature, bool, bIsSearching);


UCLASS()
class NETTPS_API UNetGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	void CreateMySession(FString roomName, int32 playerCount);

	// 세션(방) 생성이 성공적으로 완료되면 호출되는 이벤트 콜백
	UFUNCTION()
	void OnCreateSessionCompelete(FName sessionName, bool bWasSuccessful);


public:
	IOnlineSessionPtr sessionInterface;

	// 세션(방) 이름
	FString mySessionName = "Default Room";
	
	// 호스트 사용자 이름
	FString myName = "Brad";

public:
	// ------------- 방검색 --------------
	TSharedPtr<FOnlineSessionSearch> sessionSearch;
	
	void FindOtherSessions();

	// 방찾기 이벤트 콜백
	void OnFindSessionsComplete(bool bWasSuccessful);

	// 방찾기 완료 콜백을 등록할 델리게이트
	FSearchComplete OnSearchComplete;

	// 방찾기 상태 콜백 델리게이트
	FSearchStateSignature OnSearchState;

public: // --------- 세션(방) 입장 ------------
	void JoinSelectedSession(int32 index);

	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

public: // ------------ 나가기 ---------------
	void ExitRoom();

	void OnMyExitRoomComplete(FName sessionName, bool bWasSuccessful);
	
	void OnNetworkFailure(UWorld * World, UNetDriver *NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString = TEXT(""));
	// 아직 게임 방(세션)안에 있는지 여부
	bool IsInRoom();
};
