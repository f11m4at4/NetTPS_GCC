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
	btn_createRoom->OnClicked.AddDynamic(this, &ULobbyWidget::CreateRoom);
	slider_playerCount->OnValueChanged.AddDynamic(this, &ULobbyWidget::OnValueChanged);

	// widget switcher 이벤트
	btn_createSession->OnClicked.AddDynamic(this, &ULobbyWidget::SwitchCreatePanel);
	btn_findSession->OnClicked.AddDynamic(this, &ULobbyWidget::SwitchFindPanel);
	// 뒤로가기
	btn_back->OnClicked.AddDynamic(this, &ULobbyWidget::BackToMain);
	btn_back_1->OnClicked.AddDynamic(this, &ULobbyWidget::BackToMain);
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
	WidgetSwitcher->SetActiveWidgetIndex(1);
}

void ULobbyWidget::SwitchFindPanel()
{
	WidgetSwitcher->SetActiveWidgetIndex(2);
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
