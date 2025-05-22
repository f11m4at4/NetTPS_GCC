// Fill out your copyright notice in the Description page of Project Settings.


#include "MainUI.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/UniformGridPanel.h"

UMainUI::UMainUI(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FClassFinder<UUserWidget> tempBullet(TEXT("'/Game/Net/UIs/WBP_Bullet.WBP_Bullet_C'"));
	if (tempBullet.Succeeded())
	{
		bulletUIFactory = tempBullet.Class;
	}
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
