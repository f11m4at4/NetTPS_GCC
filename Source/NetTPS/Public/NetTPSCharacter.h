// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "NetTPSCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ANetTPSCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

public:
	ANetTPSCharacter();
	

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			

protected:

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

public:
	// 총을 자식으로 붙일 컴포넌트
	UPROPERTY(VisibleAnywhere)
	class USceneComponent* gunComp;

public: // --------- 총잡기 -----------
	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* ia_TakePistol;

	// 필요속성 : 총소유 여부, 소유중인 총, 총 검색 범위
	bool bHasPistol = false;
	UPROPERTY()
	AActor* ownedPistol = nullptr;
	UPROPERTY(EditAnywhere, Category=Gun)
	float distanceToGun = 200;

	// 월드에 배치된 총들
	UPROPERTY()
	TArray<AActor*> pistolActors;

	virtual void BeginPlay() override;
	
	void TakePistol(const struct FInputActionValue& value);
	// 총을 컴포넌에 붙이는 함수
	void AttachPistol(AActor* pistolActor);

public: // ----------- 총 놓기 -------------
	// 입력처리 멤버들 선언
	UPROPERTY(EditDefaultsOnly, Category=Input)
	class UInputAction* ia_ReleaseAction;
	// 총 놓기 처리 함수
	void ReleasePistol(const struct FInputActionValue& value);
	// 총을 컴포넌트에서 분리
	void DetachPistol(AActor* pistolActor);

public: // ----------- 총쏘기 -------------
	UPROPERTY(EditDefaultsOnly, Category=Input)
	class UInputAction* ia_FireAction;

	// 총알 이펙트
	UPROPERTY(EditDefaultsOnly, Category=Gun)
	class UParticleSystem* gunEffect;

	// 총쏘기 처리함수
	void Fire(const struct FInputActionValue& value);

public: // -------------- UI -----------------
	UPROPERTY(EditDefaultsOnly, Category=UI)
	TSubclassOf<class UMainUI> mainUIWidget;
	UPROPERTY()
	class UMainUI* mainUI;

	// 최대 총알개수
	UPROPERTY(EditDefaultsOnly, Category=Bullet)
	int32 maxBulletCount = 10;
	// 남은 총알개수
	int32 bulletCount = maxBulletCount;

	// UI 초기화 함수
	void InitUIWidget();

public: // ------------- 재장전 ------------
	UPROPERTY(EditDefaultsOnly, Category=Input)
	class UInputAction* ia_Relaod;
	// 재장전 중인지 여부를 기억하는 변수
	bool isReloading = false;
	
	void ReloadPistol(const struct FInputActionValue& value);

	// 총알 UI 초기화 함수
	void InitAmmoUI();

public: // ------------ 플레이어 체력 --------------
	UPROPERTY(EditDefaultsOnly, Category=HP)
	float maxHP = 3;
	float HP = maxHP;

	UPROPERTY(VisibleAnywhere)
	class UWidgetComponent* hpUIComp;

public: // ----------- 피격 처리 --------------
	void DamageProcess();
};

