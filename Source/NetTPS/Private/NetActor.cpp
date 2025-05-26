// Fill out your copyright notice in the Description page of Project Settings.


#include "NetActor.h"

#include "EngineUtils.h"
#include "NetTPS.h"
#include "NetTPSCharacter.h"
#include "Net/UnrealNetwork.h"


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

	// 대역폭 조정
	// SetNetUpdateFrequency(1.0f);
}

// Called when the game starts or when spawned
void ANetActor::BeginPlay()
{
	Super::BeginPlay();

	// 머티리얼인스턴스 생성
	mat = meshComp->CreateDynamicMaterialInstance(0);
	// 타이머를 이용해서 색상을 랜덤으로 일정시간에 한번씩 변경시켜주자.
	// 서버에서만 처리한다.
	if (HasAuthority())
	{
		FTimerHandle handle;

		// 처리함수
		auto changeColorFunc = [&]()
		{
			// 색을 랜덤으로 설정해서 넣어주기
			matColor = FLinearColor(FMath::RandRange(0.0f, 0.3f), FMath::RandRange(0.0f, 0.3f), FMath::RandRange(0.0f, 0.3f), 1.0f);
			OnRep_ChangeMatColor();
		};
		GetWorldTimerManager().SetTimer(handle, changeColorFunc, 1, true);
	}
}
+
// matColor 가 네트워크에서 변경됐을 때 호출되는 콜백
// -> 클라이언트에서만 호출되는 함수다.
void ANetActor::OnRep_ChangeMatColor()
{
	if (mat)
	{
		mat->SetVectorParameterValue(TEXT("FloorColor"), matColor);
	}
}


// Called every frame
void ANetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PrintNetLog();

	FindOwner();

	// 검출영역 시각화
	DrawDebugSphere(GetWorld(), GetActorLocation(), searchDistance, 30, FColor::Red, false, 0, 0, 1);

	// 회전 처리
	// 서버일때만 처리하도록 수정
	if (HasAuthority())
	{
		AddActorLocalRotation(FRotator(0, rotSpeed*DeltaTime, 0));
		rotYaw = GetActorRotation().Yaw;
	}
	else
	{
		// 네트워크 지연이 생기면 뚝뚝 끊김 현상이 생긴다.
		// 이를 부드럽게 보간처리해서 사용자가 불편함 없도록 처리하자.
		// 시간이 흘러야한다.
		currentTime += DeltaTime;
		// 0으로 나눠지지 않도록 검증
		if (lastTime < KINDA_SMALL_NUMBER)
		{
			return;
		}
		// 1. t 를 구해야 한다.
		// -> 구간 비율로 구하자
		// current / lastTime
		float lerpRatio = currentTime / lastTime;
		// 2.회전하고 싶다.
		//  -> R = R0 + vt -> Lerp
		float newYaw = rotYaw + rotSpeed * lastTime;
		float lerpYaw = FMath::Lerp(rotYaw, newYaw, lerpRatio);
		FRotator curRot = GetActorRotation();
		curRot.Yaw = lerpYaw;
		SetActorRotation(curRot);
	}
}

void ANetActor::OnRep_RotYaw()
{
	// 클라일때 복제된 값으로 회전을 적용하도록 처리
	FRotator newRot = GetActorRotation();
	newRot.Yaw = rotYaw;
	SetActorRotation(newRot);

	// 업데이트된 경과시간 저장
	lastTime = currentTime;
	// 경과시간
	currentTime = 0;
}

void ANetActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetActor, rotYaw);
	DOREPLIFETIME(ANetActor, matColor);
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
