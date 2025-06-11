// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetActor.generated.h"

UCLASS()
class NETTPS_API ANetActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANetActor();

protected:
	FTimerHandle handle;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* meshComp;

	// 네트워크 상태로그 출력함수
	void PrintNetLog();

	// Owner 검출 영역
	UPROPERTY(EditAnywhere)
	float searchDistance = 200;

	// Owner 찾아서 설정하는것 함수
	void FindOwner();

public: // ---------- 회전 동기화 처리 ------------
	UPROPERTY(ReplicatedUsing=OnRep_RotYaw)
	float rotYaw = 0;

	// rotYaw 변수가 변경 됐을 때 호출되는 이벤트 콜백
	UFUNCTION()
	void OnRep_RotYaw();
	
	float rotSpeed = 50;

	// 부드럽게 회전 보간 하기 위한 변수
	float currentTime = 0;
	float lastTime = 0;

public: // 재질 동기화기를 위한 속성
	UPROPERTY()
	class UMaterialInstanceDynamic* mat;
	// 재질에 동기화될 색상
	UPROPERTY(ReplicatedUsing=OnRep_ChangeMatColor)
	FLinearColor matColor;
	UFUNCTION()
	void OnRep_ChangeMatColor();

public: // ----------- RPC -------------
	UFUNCTION(Server, Reliable)
	void ServerRPC_ChangeColor(const FLinearColor& newColor);
	UFUNCTION(Client, Reliable)
	void ClientRPC_ChangeColor(const FLinearColor& newColor);
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticastRPC_ChangeColor(const FLinearColor& newColor);
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
