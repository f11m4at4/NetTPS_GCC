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
	UPROPERTY(Replicated)
	bool bHasPistol = false;
	
	UPROPERTY(Replicated)
	AActor* ownedPistol = nullptr;

	virtual void PostNetInit() override;
	
	UPROPERTY(EditAnywhere, Category=Gun)
	float distanceToGun = 200;

	// 월드에 배치된 총들
	UPROPERTY()
	TArray<AActor*> pistolActors;

	// Pawn 이 PC 한테 Possess 됐을때 호출되는 함수
	virtual void PossessedBy(AController* NewController) override;
	
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
	// UPROPERTY(EditDefaultsOnly, Category=UI)
	// TSubclassOf<class UMainUI> mainUIWidget;
	UPROPERTY()
	class UMainUI* mainUI;

	// 최대 총알개수
	UPROPERTY(EditDefaultsOnly, Category=Bullet)
	int32 maxBulletCount = 10;
	
	// 남은 총알개수
	// UPROPERTY(ReplicatedUsing=OnRep_BulletCount)
	int32 bulletCount = maxBulletCount;
	UFUNCTION()
	void OnRep_BulletCount();

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
	UPROPERTY(ReplicatedUsing=OnRep_HP)
	float hp = maxHP;
	UFUNCTION()
	void OnRep_HP();

	// 머리는 변수, 몸통은 함수인 get/set property 로 변경
	__declspec(property(get = GetHP, put = SetHP))
	float HP;
	
	float GetHP() const { return hp; }
	void SetHP(float value);

	UPROPERTY(VisibleAnywhere)
	class UWidgetComponent* hpUIComp;

public: // ----------- 피격 처리 --------------
	void DamageProcess();

	// 카메라셰이크
	UPROPERTY(EditDefaultsOnly, Category="CameraShake")
	TSubclassOf<class UCameraShakeBase> damageCameraShake;

public: // ------------ 죽음 처리 --------------
	bool isDead = false;

	void DieProcess();

public: // ---------------- Network -----------------
	virtual  void Tick(float DeltaSeconds) override;
	// 네트워크 상태로그 출력함수
	void PrintNetLog();

public: // -------------- RPC --------------------
	// -> 총잡기
	UFUNCTION(Server, Reliable)
	void ServerRPC_TakePistol();
	UFUNCTION(NetMulticast, Reliable)
	void MultiRPC_TakePistol(AActor* pistolActor);
	// -> 총놓기
	UFUNCTION(Server, Reliable)
	void ServerRPC_ReleasePistol();
	UFUNCTION(NetMulticast, Reliable)
	void MultiRPC_ReleasePistol(AActor* pistolActor);
	// -> 총쏘기
	UFUNCTION(Server, Reliable)
	void ServerRPC_FirePistol();
	// 부딪힌 결과 정보, hitinfo 정보
	UFUNCTION(NetMulticast, Reliable)
	void MultiRPC_FirePistol(bool bHit, const FHitResult& hitInfo, int bc);

	// -> 재장전
	UFUNCTION(Server, Reliable)
	void ServerRPC_ReloadPistol();
	UFUNCTION(Client, Reliable)
	void ClientRPC_ReloadPistol(int bc);
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public: // --------- Voice Chat -------------
	UPROPERTY(EditDefaultsOnly, Category=Input)
	UInputAction* voiceAction;

	void StartVoiceChat();
	void StopVoiceChat();
};

