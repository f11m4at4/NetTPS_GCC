// Fill out your copyright notice in the Description page of Project Settings.


#include "MainUI.h"

#include "NetGameInstance.h"
#include "NetPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "ChatWidget.h"
#include "NetTPSCharacter.h"
#include "Components/ScrollBox.h"
#include "Kismet/GameplayStatics.h"

UMainUI::UMainUI(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FClassFinder<UUserWidget> tempBullet(TEXT("'/Game/Net/UIs/WBP_Bullet.WBP_Bullet_C'"));
	if (tempBullet.Succeeded())
	{
		bulletUIFactory = tempBullet.Class;
	}
}

void UMainUI::NativeConstruct()
{
	Super::NativeConstruct();

	btn_retry->OnClicked.AddDynamic(this, &UMainUI::OnRetryClicked);
	btn_exit->OnClicked.AddDynamic(this, &UMainUI::OnExitClicked);
	btn_send->OnClicked.AddDynamic(this, &UMainUI::SendMsg);
}

void UMainUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 사용자 정보 갱신해주기
	// -> 플레어들 전부를 가져와야한다.
	TArray<APlayerState*> playerArr = GetWorld()->GetGameState()->PlayerArray;
	FString nameStr;
	for (auto pState : playerArr)
	{
		// 이름 : 점수
		nameStr.Append(FString::Printf(TEXT("%s : %d\n"),
			*pState->GetPlayerName(), (int32)pState->GetScore()));
	}

	txt_users->SetText(FText::FromString(nameStr));
}

void UMainUI::ShowCrosshair(bool isShow)
{
	if (isShow)
	{
		img_Crosshair->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		img_Crosshair->SetVisibility(ESlateVisibility::Hidden);
	}
}

// 총알위젯 만들어서 패널에 추가하기
void UMainUI::AddBullet()
{
	auto bulletWidget = CreateWidget(GetWorld(), bulletUIFactory);
	BulletPanel->AddChildToUniformGrid(bulletWidget, 0, BulletPanel->GetChildrenCount());
}

void UMainUI::PopBullet(int32 index)
{
	BulletPanel->RemoveChildAt(index);
}

void UMainUI::RemoveAllAmmo()
{
	BulletPanel->ClearChildren();
}

void UMainUI::PlayDamageAnimation()
{
	PlayAnimation(DamageAnim);
}

void UMainUI::OnRetryClicked()
{
	// 1. UI 안보이도록 처리
	GameoverUI->SetVisibility(ESlateVisibility::Hidden);
	// 2. 서버에 리스폰 요청을 하자.
	auto pc = Cast<ANetPlayerController>(GetWorld()->GetFirstPlayerController());
	if (pc)
	{
		pc->SetShowMouseCursor(false);
		// pc->ServerRPC_RespawnPlayer();
		pc->ServerRPC_ChangeToSpectator();
	}
}

void UMainUI::OnExitClicked()
{
	auto gi = Cast<UNetGameInstance>(GetWorld()->GetGameInstance());
	if (gi)
	{
		gi->ExitRoom();
	}
}

void UMainUI::SendMsg()
{
	// 서버에 메시지 전송한다.
	FString msg = edit_input->GetText().ToString();
	// 입력창 내용은 지워지게 하자.
	edit_input->SetText(FText::GetEmpty());
	// 보낼 메시지가 있을 때 만 서버로 보내자
	if (msg.IsEmpty() == false)
	{
		// 서버 RPC 로 메시지 전송
		auto pc = Cast<ANetPlayerController>(GetWorld()->GetFirstPlayerController());
		auto player = Cast<ANetTPSCharacter>(pc->GetPawn());
		player->ServerRPC_SendMsg(msg);
	}
}

void UMainUI::ReceiveMsg(const FString& msg)
{
	// chat widget 하나 만들어서 메시지 할당하기
	auto msgWidget = CreateWidget<UChatWidget>(GetWorld(), chatWidgetFactory);
	msgWidget->txt_msg->SetText(FText::FromString(msg));
	// msgList chat widget 추가하기
	scroll_msgList->AddChild(msgWidget);
	scroll_msgList->ScrollToEnd();
}
