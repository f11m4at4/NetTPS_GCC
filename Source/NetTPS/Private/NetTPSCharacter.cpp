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
#include "InputActionValue.h"
#include "MainUI.h"
#include "NetPlayerAnimInstance.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
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

	ConstructorHelpers::FClassFinder<UMainUI> tempMainUI(TEXT("'/Game/Net/UIs/WBP_MainUI.WBP_MainUI_C'"));
	if (tempMainUI.Succeeded())
	{
		mainUIWidget = tempMainUI.Class;
	}

	ConstructorHelpers::FObjectFinder<UInputAction> tempReloadAction(TEXT("'/Game/Net/Inputs/IA_Reload.IA_Reload'"));

	if (tempReloadAction.Succeeded())
	{
		ia_Relaod = tempReloadAction.Object;
	}

	// UI 위젯컴포넌트 추가
	hpUIComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("hpUIComp"));
	hpUIComp->SetupAttachment(GetMesh());
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

	InitUIWidget();
}

// main ui 위젯을 만들어 화면에 표시한다.
void ANetTPSCharacter::InitUIWidget()
{
	// 내 캐릭터일 때만 mainUI 만들도록 처리
	auto pc = Cast<APlayerController>(Controller);
	if (pc == nullptr)
	{
		return;
	}
	
	if (mainUIWidget)
	{
		mainUI = Cast<UMainUI>(CreateWidget(GetWorld(), mainUIWidget));
		mainUI->AddToViewport();
		mainUI->ShowCrosshair(false);

		// 총알세팅
		bulletCount = maxBulletCount;
		for (int i=0; i < bulletCount; i++)
		{
			mainUI->AddBullet();
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

void ANetTPSCharacter::InitAmmoUI()
{
	// 총알을 다시 최대 개수만큼 채워주자.
	bulletCount = maxBulletCount;
	mainUI->RemoveAllAmmo();
	for (int i=0; i < bulletCount; i++)
	{
		mainUI->AddBullet();
	}

	// 재장전 종료
	isReloading = false;
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

		// 총 붙이기
		AttachPistol(tempPistol);
		break;
	}
}

void ANetTPSCharacter::AttachPistol(AActor* pistolActor)
{
	// gunComp 컴포넌트에 붙이기
	auto meshComp = pistolActor->GetComponentByClass<UStaticMeshComponent>();
	// 물리기능 꺼주기
	meshComp->SetSimulatePhysics(false);
	meshComp->AttachToComponent(gunComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	mainUI->ShowCrosshair(true);
}

void ANetTPSCharacter::ReleasePistol(const struct FInputActionValue& value)
{
	// 총을 잡고 있지 않으면 처리 하지 않는다.
	if (bHasPistol == false || isReloading)
	{
		return;
	}
	
	// 잡은 총을 놓고 싶다.
	if (ownedPistol)
	{
		DetachPistol(ownedPistol);
		bHasPistol = false;
		ownedPistol->SetOwner(nullptr);
		ownedPistol = nullptr;
	}
}

void ANetTPSCharacter::DetachPistol(AActor* pistolActor)
{
	auto meshComp = pistolActor->GetComponentByClass<UStaticMeshComponent>();
	meshComp->SetSimulatePhysics(true);
	meshComp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);

	mainUI->ShowCrosshair(false);
}

void ANetTPSCharacter::Fire(const struct FInputActionValue& value)
{
	// 총이 없으면 처리하지 않는다.
	if (bHasPistol == false || bulletCount <= 0 || isReloading)
	{
		return;
	}

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
		// 4. 효과를 재생하고 싶다.
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), gunEffect, hitInfo.Location);

		// 5. 맞은 대상이 상대방일 경우 피격 처리
		auto otherPlayer = Cast<ANetTPSCharacter>(hitInfo.GetActor());
		if (otherPlayer)
		{
			otherPlayer->DamageProcess();
		}
	}

	// 애니메이션 재생
	auto anim = Cast<UNetPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	if (anim)
	{
		anim->PlayFireAnimation();
	}

	// 총알 제거
	bulletCount--;
	mainUI->PopBullet(bulletCount);
}


void ANetTPSCharacter::DamageProcess()
{
	// 체력 감소시키기
	HP--;

	// UI Update
	// 나일경우는 mainUI hp 를 갱신
	// 상대방일 경우
	// -> healthBar hp 를 갱신
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
