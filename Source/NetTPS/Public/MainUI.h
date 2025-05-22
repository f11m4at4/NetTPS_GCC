// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainUI.generated.h"

/**
 * 
 */
UCLASS()
class NETTPS_API UMainUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UMainUI(const FObjectInitializer& ObjectInitializer);
public:
	// Mainui 의 Image 위젯 참조 변수
	UPROPERTY(meta=(BindWidget))
	class UImage* img_Crosshair;

	// 총알 위젯이 추가될 패널
	UPROPERTY(meta=(BindWidget))
	class UUniformGridPanel* BulletPanel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Bullet")
	TSubclassOf<class UUserWidget> bulletUIFactory;

	UPROPERTY(BlueprintReadOnly, Category="HP")
	float hp = 1.0f;
public:
	// 크로스헤어 보일지여부 처리 함수
	void ShowCrosshair(bool isShow);

	// 총알위젯 추가 함수
	void AddBullet();
	// 총알 하나씩 제거하는 함수
	void PopBullet(int32 index);
	
	// 모든 총알 UI 제거
	void RemoveAllAmmo();
};
