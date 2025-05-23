// Fill out your copyright notice in the Description page of Project Settings.


#include "NetActor.h"

#include "EngineUtils.h"
#include "NetTPS.h"
#include "NetTPSCharacter.h"


// Sets default values
ANetActor::ANetActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	meshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = meshComp;
	meshComp->SetRelativeScale3D(FVector(0.5f));

	// 서버와 동기화 할지 여부(데이터 복제 여부)
	bReplicates = true;
}

// Called when the game starts or when spawned
void ANetActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PrintNetLog();

	FindOwner();

	// 검출영역 시각화
	DrawDebugSphere(GetWorld(), GetActorLocation(), searchDistance, 30, FColor::Red, false, 0, 0, 1);
}

void ANetActor::FindOwner()
{
	// 캐릭터가 검색 범위 안에 들어오면 Owner를 설정하고 싶다.
	// 반드시 서버일때만 처리 해야 한다.
	// 만약 클라이언트에서 하려고하면 서버에서 다시 원래대로 돌려놓는다.
	// 1. 서버인지 검증해야한다.
	if (HasAuthority())
	{
		// 2.검색 범위 안에 캐릭터를 찾아서
		// 찾은 액터 기억할 변수
		AActor* newOwner = nullptr;
		float minDist = searchDistance;
		// -> 월드에서 NetTPSCharacter 를 모두 찾는다.
		for (TActorIterator<ANetTPSCharacter> it(GetWorld()); it; ++it)
		{
			AActor* otherActor = *it;
			float dist = GetDistanceTo(otherActor);
			// -> 거리가 가장 가까운 녀석을 선별한다.
			if (dist < minDist)
			{
				minDist = dist;
				newOwner = otherActor;
			}
		}
		// 3. owner 설정하고 싶다.
		// -> 가장 가까운 녀석을 Owner 로 설정한다.
		if (GetOwner() != newOwner)
		{
			SetOwner(newOwner);
		}
	}
}

void ANetActor::PrintNetLog()
{
	// 네트워크 상태 로그 출력
	const FString connStr = GetNetConnection() != nullptr ? TEXT("Valid Connection") : TEXT("Invalid Connection");
	// Owner 출력
	const FString ownerName = GetOwner() ? GetOwner()->GetName() : TEXT("No Owner");
	// Role 출력
	const FString logStr = FString::Printf(TEXT("Connection : %s\nOwner Name : %s\nLocal Role : %s\nRemote Role : %s"), *connStr, *ownerName, *LOCALROLE, *REMOTEROLE);
	
	DrawDebugString(GetWorld(), GetActorLocation(), logStr, nullptr,FColor::Yellow, 0, true, 1);
}
