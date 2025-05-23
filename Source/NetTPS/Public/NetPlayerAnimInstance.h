// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "NetPlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class NETTPS_API UNetPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UNetPlayerAnimInstance();
	
public:
	// 총을 소지하고 있는지 여부 속성
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MyAnimSettings)
	bool bHasPistol = false;

	// 방향 변수 등록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MyAnimSettings)
	float direction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=MyAnimSettings)
	float speed;

	UPROPERTY()
	class ANetTPSCharacter* player;

	// 총쏘기 몽타주
	UPROPERTY(EditDefaultsOnly, Category=MyAnimSettings)
	class UAnimMontage* fireMontage;

	// 회전값 기억변수
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=MyAnimSettings)
	float pitchAngle;

	// 재장전 몽타주
	UPROPERTY(EditDefaultsOnly, Category=MyAnimSettings)
	class UAnimMontage* reloadMontage;

	// 죽었는지 상태 기억 변수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=MyAnimSettings)
	bool isDead = false;
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// 총쏘기 애니메이션 재생 함수
	void PlayFireAnimation();

	// 재장전 애니메이션 재생
	void PlayReloadAnimation();
	// 재장전 애니메이션 노티파이 이벤트 콜백 처리 함수
	UFUNCTION()
	void AnimNotify_OnReloadFinish();
};
