// Fill out your copyright notice in the Description page of Project Settings.


#include "NetPlayerAnimInstance.h"

#include "NetTPSCharacter.h"

UNetPlayerAnimInstance::UNetPlayerAnimInstance()
{
	ConstructorHelpers::FObjectFinder<UAnimMontage> tempFire(TEXT("'/Game/Net/Animations/MM_Pistol_Fire_Montage.MM_Pistol_Fire_Montage'"));
	if (tempFire.Succeeded())
	{
		fireMontage = tempFire.Object;
	}

	ConstructorHelpers::FObjectFinder<UAnimMontage> tempReload(TEXT("'/Game/Net/Animations/MM_Pistol_Reload_Montage.MM_Pistol_Reload_Montage'"));
	if (tempReload.Succeeded())
	{
		reloadMontage = tempReload.Object;
	}
}

void UNetPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	player = Cast<ANetTPSCharacter>(TryGetPawnOwner());
}

void UNetPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// Player 의 속도와 이동 방향의 값을 가져와서 할당해주기
	// 1. Player 가 있어야 한다.
	if (player)
	{
		// 2. 속도와 방향 값 할당하기
		FVector vel = player->GetVelocity();
		speed = FVector::DotProduct(vel, player->GetActorForwardVector());
		direction = FVector::DotProduct(vel, player->GetActorRightVector());

		bHasPistol = player->bHasPistol;

		// 회전값 적용
		pitchAngle = -player->GetBaseAimRotation().GetNormalized().Pitch;
		pitchAngle = FMath::Clamp(pitchAngle, -60.0f, 60.0f);
	}
}

void UNetPlayerAnimInstance::PlayFireAnimation()
{
	// 총을 갖고 있고, 몽타주가 있을 때 재생하자
	if (bHasPistol && fireMontage)
	{
		Montage_Play(fireMontage);
	}
}

// 재장전 애니메이션 재생
void UNetPlayerAnimInstance::PlayReloadAnimation()
{
	if (bHasPistol && reloadMontage)
	{
		Montage_Play(reloadMontage);
	}
}

// 재장전 애니메이션 종료 이벤트 발생시 처리 콜백함수
void UNetPlayerAnimInstance::AnimNotify_OnReloadFinish()
{
	player->InitAmmoUI();
}
