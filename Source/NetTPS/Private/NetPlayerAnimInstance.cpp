// Fill out your copyright notice in the Description page of Project Settings.


#include "NetPlayerAnimInstance.h"

#include "NetTPSCharacter.h"

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
	}
}
