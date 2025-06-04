// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetTPSCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HealthBar.h"
#include "InputActionValue.h"
#include "MainUI.h"
#include "NetPlayerAnimInstance.h"
#include "NetPlayerController.h"
#include "NetTPS.h"
#include "Components/HorizontalBox.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystem.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ANetTPSCharacter

ANetTPSCharacter::ANetTPSCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bUseControllerDesiredRotation = true; // Character moves in the direction of input...	
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetRelativeLocation(FVector(0, 40, 60));
	CameraBoom->TargetArmLength = 150.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	// gun 컴포넌트 추가
	gunComp = CreateDefaultSubobject<USceneComponent>(TEXT("GunComp"));
	gunComp->SetupAttachment(GetMesh(), TEXT("GunPosition"));
	gunComp->SetRelativeLocation(FVector(-15.826352,4.000000,3.015192));
	gunComp->SetRelativeRotation(FRotator(0.867172,85.075584,9.962711));

	// ia_takepistol 애셋 로드해서 등록해주기
	ConstructorHelpers::FObjectFinder<UInputAction> tempTakePistol(TEXT("'/Game/Net/Inputs/IA_TakePistol.IA_TakePistol'"));

	if (tempTakePistol.Succeeded())
	{
		ia_TakePistol = tempTakePistol.Object;
	}

	ConstructorHelpers::FObjectFinder<UInputAction> tempReleasePistol(TEXT("'/Game/Net/Inputs/IA_ReleasePistol.IA_ReleasePistol'"));

	if (tempReleasePistol.Succeeded())
	{
		ia_ReleaseAction = tempReleasePistol.Object;
	}

	ConstructorHelpers::FObjectFinder<UInputAction> tempFireAction(TEXT("'/Game/Net/Inputs/IA_Fire.IA_Fire'"));

	if (tempFireAction.Succeeded())
	{
		ia_FireAction = tempFireAction.Object;
	}

	ConstructorHelpers::FObjectFinder<UParticleSystem> tempBulletEffect(TEXT("'/Game/StarterContent/Particles/P_Explosion.P_Explosion'"));

	if (tempBulletEffect.Succeeded())
	{
		gunEffect = tempBulletEffect.Object;
	}

	
	ConstructorHelpers::FObjectFinder<UInputAction> tempReloadAction(TEXT("'/Game/Net/Inputs/IA_Reload.IA_Reload'"));

	if (tempReloadAction.Succeeded())
	{
		ia_Relaod = tempReloadAction.Object;
	}

	// UI 위젯컴포넌트 추가
	hpUIComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("hpUIComp"));
	hpUIComp->SetupAttachment(GetMesh());

	ConstructorHelpers::FClassFinder<UHealthBar>tempHP(TEXT("'/Game/Net/UIs/WBP_HealthBar.WBP_HealthBar_C'"));
	if (tempHP.Succeeded())
	{
		hpUIComp->SetWidgetClass(tempHP.Class);
	}

	ConstructorHelpers::FClassFinder<UCameraShakeBase> tempCS(TEXT("'/Game/Net/Blueprints/BP_CameraShake.BP_CameraShake_C'"));
	if (tempCS.Succeeded())
	{
		damageCameraShake = tempCS.Class;
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ANetTPSCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ANetTPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANetTPSCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANetTPSCharacter::Look);

		// 총잡기
		EnhancedInputComponent->BindAction(ia_TakePistol, ETriggerEvent::Started, this, &ANetTPSCharacter::TakePistol);

		// 총 놓기
		EnhancedInputComponent->BindAction(ia_ReleaseAction, ETriggerEvent::Started, this, &ANetTPSCharacter::ReleasePistol);

		// 총쏘기
		EnhancedInputComponent->BindAction(ia_FireAction, ETriggerEvent::Started, this, &ANetTPSCharacter::Fire);

		// 재장전
		EnhancedInputComponent->BindAction(ia_Relaod, ETriggerEvent::Started, this, &ANetTPSCharacter::ReloadPistol);
		
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ANetTPSCharacter::PossessedBy(AController* NewController)
{
	PRINTLOG(TEXT("Begin"));
	Super::PossessedBy(NewController);

	// 내꺼 일때
	if (IsLocallyControlled())
	{
		// InitUIWidget 호출해주자.
		InitUIWidget();
	}
	PRINTLOG(TEXT("End"));
}

void ANetTPSCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 총 검색
	TArray<AActor*> allActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(),allActors);
	for (auto tempPistol : allActors)
	{
		// 2.2 이름을 조사해서 pistol 인 녀석으로 구분
		if (tempPistol->GetActorNameOrLabel().Contains("BP_Pistol"))
		{
			pistolActors.Add(tempPistol);
		}
	}

	// 컨트롤러가 있어야 한다.
	// 클라이언트에서 실행되고 있어야한다. -> local role => autonomous
	if (IsLocallyControlled() && HasAuthority() == false)
	{
		InitUIWidget();
	}
}

// main ui 위젯을 만들어 화면에 표시한다.
void ANetTPSCharacter::InitUIWidget()
{
	PRINTLOG(TEXT("[%s] Begin"), Controller?TEXT("PLAYER") : TEXT("NO PLAYER"));

	// 내 캐릭터일 때만 mainUI 만들도록 처리
	auto pc = Cast<ANetPlayerController>(Controller);
	if (pc == nullptr)
	{
		return;
	}
	
	if (pc->mainUIWidget)
	{
		// 기존에 이미 생성했으면 그녀석을 할당해주자
		if (pc->mainUI == nullptr)
		{
			pc->mainUI = Cast<UMainUI>(CreateWidget(GetWorld(), pc->mainUIWidget));
		}
		mainUI = pc->mainUI;
		mainUI->AddToViewport();
		mainUI->ShowCrosshair(false);

		// HP
		hp = maxHP;
		mainUI->hp = 1.0f;
		
		// 총알세팅
		// 먼저 총알 UI에 있는 총알 다 제거해주자.
		mainUI->RemoveAllAmmo();
		bulletCount = maxBulletCount;
		for (int i=0; i < bulletCount; i++)
		{
			mainUI->AddBullet();
		}

		// hpuiComp 화면에 보여지면 안된다.
		if (hpUIComp)
		{
			hpUIComp->SetVisibility(false);
		}
	}
}

void ANetTPSCharacter::ReloadPistol(const struct FInputActionValue& value)
{
	// 총을 갖고 있지 않으면 처리 하지 않는다.
	// 혹은 이미지 재장전 중일때도 처리하지 않는다.
	if (bHasPistol == false || isReloading)
	{
		return;
	}

	auto anim = Cast<UNetPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	if (anim)
	{
		anim->PlayReloadAnimation();
	}
	
	// 재장전 중으로 설정
	isReloading = true;
}

void ANetTPSCharacter::ServerRPC_ReloadPistol_Implementation()
{
	// 총알을 다시 최대 개수만큼 채워주자.
	bulletCount = maxBulletCount;
	ClientRPC_ReloadPistol(maxBulletCount);
}

void ANetTPSCharacter::ClientRPC_ReloadPistol_Implementation(int bc)
{
	bulletCount = bc;
	mainUI->RemoveAllAmmo();
	for (int i=0; i < bulletCount; i++)
	{
		mainUI->AddBullet();
	}

	// 재장전 종료
	isReloading = false;
}

// 호출되는 시점이 언제냐면 -> 애니메이션 종료되는 시점에 호출된다.
void ANetTPSCharacter::InitAmmoUI()
{
	// 서버에 요청하기
	ServerRPC_ReloadPistol();
}


void ANetTPSCharacter::TakePistol(const struct FInputActionValue& value)
{
	// F 키를 눌렀을 때 호출되는 이벤트 콜백 함수
	// 이미 총을 잡고있지 않다면 일정 범위 안에있는 총을 잡는다.
	// 필요속성 : 총소유 여부, 소유중인 총, 총 검색 범위
	// 1. 총을 잡고 있지 않아야 한다.
	if (bHasPistol)
	{
		return;
	}

	// 서버에 총잡기 버튼을 눌렀다고 Server RPC 보내자.
	ServerRPC_TakePistol();
}

void ANetTPSCharacter::ServerRPC_TakePistol_Implementation()
{
	// 2. 범위 안에 있어야한다.
	// -> 월드에 있는 모든 총을 가져와서 범위 검사를 해야 한다.
	// 2.1 월드에 있는 모든 액터를 가져온다.
	
	for (auto tempPistol : pistolActors)
	{
		// 2.2 이름을 조사해서 pistol 인 녀석으로 구분
		// -> 단, 소유자가 있으면 안된다.
		if (tempPistol && tempPistol->IsPendingKillPending() == false && IsValid(tempPistol) && tempPistol->GetOwner() != nullptr)
		{
			continue;
		}
		// 2.2 범위 검사를 한다.
		// 총과의 거리를 구한다.
		float distance = FVector::Dist(GetActorLocation(), tempPistol->GetActorLocation());
		if (distance > distanceToGun)
		{
			continue;
		}
		// 3. 총을 잡고싶다.
		ownedPistol = tempPistol;
		ownedPistol->SetOwner(this);
		bHasPistol = true;

		// 모든 클라이언트들한테 시각화처리 요청하기
		MultiRPC_TakePistol(tempPistol);
		break;
	}	
}

void ANetTPSCharacter::MultiRPC_TakePistol_Implementation(AActor* pistolActor)
{
		// 총 붙이기
		AttachPistol(pistolActor);
}

void ANetTPSCharacter::AttachPistol(AActor* pistolActor)
{
	// gunComp 컴포넌트에 붙이기
	auto meshComp = pistolActor->GetComponentByClass<UStaticMeshComponent>();
	// 물리기능 꺼주기
	meshComp->SetSimulatePhysics(false);
	meshComp->AttachToComponent(gunComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	// 내 캐릭터 일때만 -> Player Controller 에 의해서 제어 되고 있을 때만.
	// Role -> Autonomous 일때
	if (IsLocallyControlled() && mainUI)
	{
		mainUI->ShowCrosshair(true);
	}
}

void ANetTPSCharacter::ReleasePistol(const struct FInputActionValue& value)
{
	// 총을 잡고 있지 않으면 처리 하지 않는다.
	if (bHasPistol == false || isReloading || IsLocallyControlled() == false)
	{
		return;
	}

	// 서버에 총놓기 요청하기
	ServerRPC_ReleasePistol();
}

void ANetTPSCharacter::ServerRPC_ReleasePistol_Implementation()
{
	// 잡은 총을 놓고 싶다.
	if (ownedPistol)
	{
		// 모든 클라이언트한테 총놓기 메시지 보내기
		MultiRPC_ReleasePistol(ownedPistol);
		
		bHasPistol = false;
		ownedPistol->SetOwner(nullptr);
		ownedPistol = nullptr;
	}
}

void ANetTPSCharacter::MultiRPC_ReleasePistol_Implementation(AActor* pistolActor)
{
	DetachPistol(pistolActor);
}

void ANetTPSCharacter::DetachPistol(AActor* pistolActor)
{
	auto meshComp = pistolActor->GetComponentByClass<UStaticMeshComponent>();
	meshComp->SetSimulatePhysics(true);
	meshComp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);

	if (IsLocallyControlled() && mainUI)
	{
		mainUI->ShowCrosshair(false);
	}
}

void ANetTPSCharacter::Fire(const struct FInputActionValue& value)
{
	// 총이 없으면 처리하지 않는다.
	if (bHasPistol == false || bulletCount <= 0 || isReloading)
	{
		return;
	}

	// Server 에 총쏘기 처리 요청을 한다.
	ServerRPC_FirePistol();
}

void ANetTPSCharacter::ServerRPC_FirePistol_Implementation()
{
	// 총쏘기
	// Line Trace
	// 선에 부딪혔을 때 그 지점에 P_Explosion 가 재생 되도록 하고 싶다.
	// 1. 선이 필요하다.
	FVector startPos = GetFollowCamera()->GetComponentLocation();
	FVector endPos = startPos + GetFollowCamera()->GetForwardVector() * 10000.0f;
	// 2. 선을 쏴야한다.
	FHitResult hitInfo;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	bool bHit = GetWorld()->LineTraceSingleByChannel(hitInfo, startPos, endPos, ECC_Visibility, params);
	// 3. 선이 부딪혔으니까
	if (bHit)
	{
		// 5. 맞은 대상이 상대방일 경우 피격 처리
		auto otherPlayer = Cast<ANetTPSCharacter>(hitInfo.GetActor());
		if (otherPlayer)
		{
			otherPlayer->DamageProcess();
		}
	}

	// 총알 제거
	bulletCount--;
	//OnRep_BulletCount();
	
	// 클라이언트들한테 결과 처리 하도록 보내기
	MultiRPC_FirePistol(bHit, hitInfo, bulletCount);
}

// 클라이언트에서 호출되는 RPC 함수
void ANetTPSCharacter::MultiRPC_FirePistol_Implementation(bool bHit, const FHitResult& hitInfo, int bc)
{
	bulletCount = bc;
	if (bHit)
	{
		// 4. 효과를 재생하고 싶다.
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), gunEffect, hitInfo.Location);
	}

	// 애니메이션 재생
	auto anim = Cast<UNetPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	if (anim)
	{
		anim->PlayFireAnimation();
	}

	if (mainUI)
	{
		mainUI->PopBullet(bulletCount);
	}
}

// bulletCount 변수가 서버에서 변경되면 자동으로 호출되는 콜백 함수
// -> 클라이언트에서만 호출되는 함수
void ANetTPSCharacter::OnRep_BulletCount()
{
	if (mainUI)
	{
		mainUI->PopBullet(bulletCount);
	}
}

void ANetTPSCharacter::DamageProcess()
{
	HP--;

	// 죽음처리
	if (HP <= 0)
	{
		isDead = true;
	}
}

void ANetTPSCharacter::SetHP(float value)
{
	// 체력 감소시키기
	hp = value;

	// 서버에서 처리할때 (Listen Server 이기 때문에 C, S 같이 존재한다.)
	// -> 따라서 서버와 함께 있는 클라에도 화면 갱신 처리를 해줘야 한다.
	OnRep_HP();
}

// 클라이언트에서만 호출되는 이벤트(C++)
void ANetTPSCharacter::OnRep_HP()
{
	// 죽음처리
	if (hp <= 0)
	{
		// 죽음 상태로 전환
		isDead = true;
		// 총 떨어뜨리기
		ReleasePistol(FInputActionValue());
		// 충돌체 비활성화
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// 못움직이게 처리
		GetCharacterMovement()->DisableMovement();
	}

	
	// UI Update
	float percent = hp / maxHP;
	// 나일경우는 mainUI hp 를 갱신
	//-> PlayerController 있을 때 mainUI 를 생성한다.
	if (mainUI)
	{
		mainUI->hp = percent;
		// 피격효과 처리
		mainUI->PlayDamageAnimation();

		// 카메라셰이크재생
		if (damageCameraShake)
		{
			auto pc = Cast<APlayerController>(Controller);
			pc->ClientStartCameraShake(damageCameraShake);
		}
	}
	// 상대방일 경우
	else
	{
		// -> healthbar 를 갖고있는 컴포넌트를 가져와야 한다.
		// -> 그 컴포넌트에 있는 healthBar hp 를 갱신
		if (hpUIComp)
		{
			auto hpUI = Cast<UHealthBar>(hpUIComp->GetWidget());
			if (hpUI)
				hpUI->hp = percent;
		}
	}
}

// 카메라의 채도를 떨어뜨리자.
// -> UI 를 화면에 표시하고 싶다.
// -> 화면에 마우스를 표시해줘야한다.
void ANetTPSCharacter::DieProcess()
{
	// -> 화면에 마우스를 표시해줘야한다.
	auto pc = Cast<APlayerController>(Controller);
	pc->SetShowMouseCursor(true);
	// 카메라의 채도를 떨어뜨리자.
	GetFollowCamera()->PostProcessSettings.ColorSaturation = FVector4(0.0f, 0.0f, 0.0f, 1.0f);

	// gameover UI 표시
	mainUI->GameoverUI->SetVisibility(ESlateVisibility::Visible);
}

void ANetTPSCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ANetTPSCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

// ------------------------ Network--------------------------------
void ANetTPSCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//PrintNetLog();

	// 빌보딩처리
	if (hpUIComp && hpUIComp->GetVisibleFlag())
	{
		// 카메라를 바라보도록 하고 싶다.
		// 카메라로 향하는 방향이 필요하다.
		// dir = target - me
		// 1. P.C -> 2. Pawn. -> 3. GetFollow
		FVector camLocation = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraLocation();
		FVector direction = camLocation - hpUIComp->GetComponentLocation();
		// direction.Z = 0;
		hpUIComp->SetWorldRotation(direction.ToOrientationRotator());
	}
}



void ANetTPSCharacter::PrintNetLog()
{
	// 네트워크 상태 로그 출력
	const FString connStr = GetNetConnection() != nullptr ? TEXT("Valid Connection") : TEXT("Invalid Connection");
	// Owner 출력
	const FString ownerName = GetOwner() ? GetOwner()->GetName() : TEXT("No Owner");
	// Role 출력
	const FString logStr = FString::Printf(TEXT("Connection : %s\nOwner Name : %s\nLocal Role : %s\nRemote Role : %s"), *connStr, *ownerName, *LOCALROLE, *REMOTEROLE);
	
	DrawDebugString(GetWorld(), GetActorLocation(), logStr, nullptr,FColor::Yellow, 0, true, 1);
}

void ANetTPSCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetTPSCharacter, bHasPistol);
	DOREPLIFETIME(ANetTPSCharacter, hp);
	
	// DOREPLIFETIME(ANetTPSCharacter, bulletCount);
}



