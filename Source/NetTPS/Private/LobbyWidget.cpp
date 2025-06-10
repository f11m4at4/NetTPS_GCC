// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyWidget.h"

#include "NetGameInstance.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "SessionSlotWidget.h"
#include "Components/ScrollBox.h"

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	gi = Cast<UNetGameInstance>(GetWorld()->GetGameInstance());

	// 세션 찾았을 때 호출될 콜백 함수 등록해주기
	gi->OnSearchComplete.AddDynamic(this, &ULobbyWidget::AddSlotWidget);
	// 방검색 중일때 호출될 콜백 함수 등록
	gi->OnSearchState.AddDynamic(this, &ULobbyWidget::OnFindStateEnable);
	
	
	btn_createRoom->OnClicked.AddDynamic(this, &ULobbyWidget::CreateRoom);
	slider_playerCount->OnValueChanged.AddDynamic(this, &ULobbyWidget::OnValueChanged);

	// widget switcher 이벤트
	btn_createSession->OnClicked.AddDynamic(this, &ULobbyWidget::SwitchCreatePanel);
	btn_findSession->OnClicked.AddDynamic(this, &ULobbyWidget::SwitchFindPanel);
	// 뒤로가기
	btn_back->OnClicked.AddDynamic(this, &ULobbyWidget::BackToMain);
	btn_back_1->OnClicked.AddDynamic(this, &ULobbyWidget::BackToMain);

	// 방검색
	btn_find->OnClicked.AddDynamic(this, &ULobbyWidget::OnClickedFindSession);
}

void ULobbyWidget::CreateRoom()
{
	if (gi && edit_roomName->GetText().IsEmpty() == false)
	{
		FString roomName = edit_roomName->GetText().ToString();
		int32 playerCount = slider_playerCount->GetValue();
		gi->CreateMySession(roomName, playerCount);
	}
}

void ULobbyWidget::OnValueChanged(float value)
{
	txt_playerCount->SetText(FText::AsNumber(value));
}

void ULobbyWidget::SwitchCreatePanel()
{
	if (edit_userName->GetText().IsEmpty() == false)
	{
		gi->myName = edit_userName->GetText().ToString();
	}
	WidgetSwitcher->SetActiveWidgetIndex(1);
}

// 방목록 검색 화면
void ULobbyWidget::SwitchFindPanel()
{
	if (edit_userName->GetText().IsEmpty() == false)
	{
		gi->myName = edit_userName->GetText().ToString();
	}
	WidgetSwitcher->SetActiveWidgetIndex(2);
	OnClickedFindSession();
}

void ULobbyWidget::BackToMain()
{
	WidgetSwitcher->SetActiveWidgetIndex(0);
}

void ULobbyWidget::AddSlotWidget(const struct FSessionInfo& sessionInfo)
{
	// slot widget 만들어서 스크롤박스에 추가해주자
	auto slot = CreateWidget<USessionSlotWidget>(this, sessionInfoWidget);
	slot->Set(sessionInfo);

	scroll_roomList->AddChild(slot);
}

void ULobbyWidget::OnClickedFindSession()
{
	// 기존 슬롯이 있다면 모두 지운다.
	scroll_roomList->ClearChildren();

	if (gi)
	{
		gi->FindOtherSessions();
	}
}

// 방검색 중일때 검색 UI 표시 해주는 함수
void ULobbyWidget::OnFindStateEnable(bool bIsSearching)
{
	// 검색 버튼은 비활성화
	btn_find->SetIsEnabled(!bIsSearching);

	// 검색중... UI visibility 속성 변경
	if (bIsSearching)
	{
		txt_findingMsg->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		txt_findingMsg->SetVisibility(ESlateVisibility::Hidden);
	}
}
