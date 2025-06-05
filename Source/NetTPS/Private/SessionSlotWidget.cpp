// Fill out your copyright notice in the Description page of Project Settings.


#include "SessionSlotWidget.h"

#include "Components/TextBlock.h"
#include "NetGameInstance.h"

void USessionSlotWidget::Set(const FSessionInfo& sessionInfo)
{
	txt_roomName->SetText(FText::FromString(sessionInfo.roomName));
	txt_hostName->SetText(FText::FromString(sessionInfo.hostName));
	txt_playerCount->SetText(FText::FromString(sessionInfo.playerCount));
	txt_pingSpeed->SetText(FText::FromString(FString::Printf(TEXT("%dms"), sessionInfo.pingSpeed)));

	sessionNumber = sessionInfo.index;
}
