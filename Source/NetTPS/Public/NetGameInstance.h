// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "NetGameInstance.generated.h"

/**
 * 
 */
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
	FString myHostName = "Brad";
};
