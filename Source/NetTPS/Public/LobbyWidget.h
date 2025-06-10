// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyWidget.generated.h"

/**
 * 
 */
UCLASS()
class NETTPS_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UButton* btn_createRoom;
	UPROPERTY(meta = (BindWidget))
	class UEditableText* edit_roomName;
	UPROPERTY(meta = (BindWidget))
	class USlider* slider_playerCount;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* txt_playerCount;

	// 메인, -> 방 전환하도록 하는 위젯스위처, 방생성버튼, 방검색 버튼
	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* WidgetSwitcher;
	UPROPERTY(meta = (BindWidget))
	class UButton* btn_createSession;
	UPROPERTY(meta = (BindWidget))
	class UButton* btn_findSession;

	// 뒤로 가기 버튼(main)
	UPROPERTY(meta = (BindWidget))
	class UButton* btn_back;
	UPROPERTY(meta = (BindWidget))
	class UButton* btn_back_1;

	// 방검색 버튼
	UPROPERTY(meta = (BindWidget))
	class UButton* btn_find;

	// 방검색 메시지
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* txt_findingMsg;
	
	// 사용자 이름
	UPROPERTY(meta = (BindWidget))
	class UEditableText* edit_userName;
	
	// 세션 슬롯 추가할 스크롤박스
	UPROPERTY(meta = (BindWidget))
	class UScrollBox* scroll_roomList;

	UPROPERTY(EditDefaultsOnly, Category=SlotWidget)
	TSubclassOf<class USessionSlotWidget> sessionInfoWidget;

	// GameInstance
	UPROPERTY()
	class UNetGameInstance* gi;

public:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void CreateRoom();

	// slider 값 변경하면 호출되는 이벤트 콜백
	UFUNCTION()
	void OnValueChanged(float value);

	// 화면 전환 함수
	UFUNCTION()
	void SwitchCreatePanel();
	UFUNCTION()
	void SwitchFindPanel();

	// 메인으로 돌아가는 함수
	UFUNCTION()
	void BackToMain();

	UFUNCTION()
	void AddSlotWidget(const struct FSessionInfo& sessionInfo);

	// 방검색 버튼 클릭시 호출될 콜백
	UFUNCTION()
	void OnClickedFindSession();

	// 방찾기 상태 이벤트 콜백
	UFUNCTION()
	void OnFindStateEnable(bool bIsSearching);
};
