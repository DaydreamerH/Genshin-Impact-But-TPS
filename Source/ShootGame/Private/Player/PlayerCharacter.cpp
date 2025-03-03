#include "ShootGame/Public/Player/PlayerCharacter.h"

#include "EnhancedInputSubsystemInterface.h"
#include "../Plugins/EnhancedInput/Source/EnhancedInput/Public/EnhancedInputSubsystems.h"
#include "../Plugins/EnhancedInput/Source/EnhancedInput/Public/EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/CombatComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameMode/LobbyGameMode.h"
#include "GameMode/ShootGameMode.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Net/UnrealNetwork.h"
#include "Player/MyPlayerState.h"
#include "PlayerController/MyPlayerController.h"
#include "ShootGame/ShootGame.h"
#include "Weapon/Weapon.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // 在CameraBoom已经旋转

	bUseControllerRotationYaw = false; // Player此时不应该和控制器一起旋转，他朝自己的方向而非控制器方向移动
	GetCharacterMovement()->bOrientRotationToMovement = true;

	bReplicates = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);
	

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void APlayerCharacter::OnRep_PlayerIndex() 
{
	this->PlayerIndex = PlayerIndex;
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerCharacter, PlayerIndex);
	DOREPLIFETIME_CONDITION(APlayerCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(APlayerCharacter, Health);
	DOREPLIFETIME(APlayerCharacter, bDisableGameplay);
}

void APlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if(Combat)
	{
		Combat->Character = this;
	}
}

void APlayerCharacter::Elim()
{
	if(Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->Dropped();
	}
	
	MulticastElim();
	StartHealthRecovery();
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ThisClass::ElimTimerFinished,
		ElimDelay
	);
}

void APlayerCharacter::MulticastElim_Implementation()
{
	if(PlayerController)
	{
		PlayerController->SetHUDWeaponAmmo(0);
	}
	
	bElimmed = true;
	PlayElimMontage();
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	bDisableGameplay = true;
	
	if(IsLocallyControlled() && Combat
		&& Combat->bAiming && Combat->EquippedWeapon
		&& Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		ShowSniperScopeWidget(false);
	}
}

void APlayerCharacter::ElimTimerFinished()
{
	if(AShootGameMode* ShootGameMode = GetWorld()->GetAuthGameMode<AShootGameMode>())
	{
		ShootGameMode->RequestRespawn(this, PlayerController);
	}
	
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if(HasAuthority())
	{
		if (ALobbyGameMode* lGM = Cast<ALobbyGameMode>(GetWorld()->GetAuthGameMode()))
		{
			PlayerIndex = lGM->GetPlayerIndex();
		}
		else if(AShootGameMode* sGM = Cast<AShootGameMode>(GetWorld()->GetAuthGameMode()))
		{
			PlayerIndex = sGM->GetPlayerIndex();
		}

		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
	}
	
	UpdateHUDHealth();
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	this->UpdateMPC();
	AimOffset(DeltaTime);
	HideCamera();
	PollInit();
}

void APlayerCharacter::OnActionMoveForward(const FInputActionValue& InputActionValue)
{
	if(bDisableGameplay)return;
	
	float inputValue = InputActionValue.Get<float>();
	if(Controller&&inputValue!=0)
	{
		// 获取方向
		const FRotator controllerYawRotator(0, Controller->GetControlRotation().Yaw, 0);
		const FVector direction = controllerYawRotator.RotateVector(FVector::ForwardVector);
		
		AddMovementInput(direction, inputValue);
	}
}

void APlayerCharacter::OnActionMoveRight(const FInputActionValue& InputActionValue)
{
	if(bDisableGameplay)return;
	float inputValue = InputActionValue.Get<float>();
	if(Controller&&inputValue!=0)
	{
		// 获取方向
		const FRotator controllerYawRotator(0, Controller->GetControlRotation().Yaw, 0);
		const FVector direction = controllerYawRotator.RotateVector(FVector::RightVector);
		
		AddMovementInput(direction, inputValue);
	}
}

void APlayerCharacter::OnActionLookUp(const FInputActionValue& InputActionValue)
{
	if(bDisableGameplay)return;
	float inputValue = InputActionValue.Get<float>();
	AddControllerPitchInput(inputValue);
}

void APlayerCharacter::OnActionLookRight(const FInputActionValue& InputActionValue)
{
	if(bDisableGameplay)return;
	float inputValue = InputActionValue.Get<float>();
	AddControllerYawInput(inputValue);
}

void APlayerCharacter::OnActionJump(const FInputActionValue& InputActionValue)
{
	if(bDisableGameplay)return;
	Jump();
}

void APlayerCharacter::OnActionEquip(const FInputActionValue& InputActionValue)
{
	if(bDisableGameplay)return;
	if(HasAuthority())
	{
		if(Combat)
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
	}
	else
	{
		ServerOnActionEquip();
	}
}

void APlayerCharacter::OnActionCrouch(const FInputActionValue& InputActionValue)
{
	if(bDisableGameplay)return;
	if(bIsCrouched)
	{
		UnCrouch();
		
	}
	else
	{
		Crouch();
	}
}

void APlayerCharacter::OnActionAimPressed(const FInputActionValue& InputActionValue)
{
	if(bDisableGameplay)return;
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void APlayerCharacter::OnActionAimReleased(const FInputActionValue& InputActionValue)
{
	if(bDisableGameplay)return;
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void APlayerCharacter::OnActionFirePressed(const FInputActionValue& InputActionValue)
{
	if(bDisableGameplay)return;
	if(Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void APlayerCharacter::OnActionFireReleased(const FInputActionValue& InputActionValue)
{
	if(bDisableGameplay)return;
	if(Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void APlayerCharacter::OnActionReload(const FInputActionValue& InputActionValue)
{
	if(bDisableGameplay)return;
	if(Combat)
	{
		Combat->Reload();
	}
}

void APlayerCharacter::AimOffset(float DeltaTime)
{
	if(Combat && Combat->EquippedWeapon==nullptr)return;
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();
	if(Speed == 0.f && !bIsInAir)
	{
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation =
			UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		bUseControllerRotationYaw = true;
		if(TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		TurnInPlace(DeltaTime);
	}
	if(Speed>0.f||bIsInAir)
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
	
	AO_Pitch = GetBaseAimRotation().Pitch;
	if(!IsLocallyControlled() && AO_Pitch>90.f)
	{
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void APlayerCharacter::ServerOnActionEquip_Implementation()
{
	if(Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void APlayerCharacter::UpdateMPC()
{
	if(PlayerIndex != -1)
	{
		USkeletalMeshComponent* mesh = GetMesh();
		if(mesh)
		{
			FVector location = mesh->GetComponentLocation();

			if(MPC_Position)
			{
				if(UMaterialParameterCollectionInstance* mpcInst = GetWorld()->GetParameterCollectionInstance(MPC_Position))
				{
					FString ParameterNameString = FString::Printf(TEXT("Position_%d"), PlayerIndex);
					FName ParameterName = FName(*ParameterNameString);
                    
					FLinearColor NewColor(location.X, location.Y, location.Z);
					mpcInst->SetVectorParameterValue(ParameterName, NewColor);

					float TargetRadius = (bIsCrouched) ? 300.f : 600.f;

					if (FMath::IsNearlyEqual(CurrentRadius, TargetRadius, 1.f)) 
					{
						CurrentRadius = TargetRadius;
					}
					else
					{
						CurrentRadius = FMath::FInterpTo(CurrentRadius, TargetRadius, GetWorld()->GetDeltaSeconds(), 5.f); // 5.f 是插值速度，可以调整
					}

					ParameterNameString = FString::Printf(TEXT("Radius_%d"), PlayerIndex);
					ParameterName = FName(*ParameterNameString);
					mpcInst->SetScalarParameterValue(ParameterName, CurrentRadius);
				}
			}
		}
	}
}



void APlayerCharacter::Jump()
{
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void APlayerCharacter::ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType,
	AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();

	if(Health<=0.f)
	{
		if(AShootGameMode* ShootGameMode = GetWorld()->GetAuthGameMode<AShootGameMode>())
		{
			if((PlayerController = PlayerController == nullptr ? Cast<AMyPlayerController>(GetController()):PlayerController))
			{
				AMyPlayerController* AttackerController =
					Cast<AMyPlayerController>(InstigatorController);
				
				ShootGameMode->PlayerEliminated
				(this, PlayerController, AttackerController);
			}
		}	
	}
}

void APlayerCharacter::PollInit()
{
	if(MyPlayerState==nullptr)
	{
		MyPlayerState = GetPlayerState<AMyPlayerState>();
		if(MyPlayerState)
		{
			MyPlayerState->AddToScore(0.f);
			MyPlayerState->AddToDefeats(0);
		}
		
	}
}

void APlayerCharacter::PlayFireMontage(bool bAiming) const
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr)return;
	if(UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && FireWeaponMontage && !AnimInstance->Montage_IsPlaying(FireWeaponMontage))
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		const FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void APlayerCharacter::PlayHitReactMontage() const
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr)return;
	if(UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		const FName SectionName ("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void APlayerCharacter::PlayElimMontage() const
{
	if(UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void APlayerCharacter::PlayReloadMontage() const
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr)return;
	if(UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && ReloadMontage && !AnimInstance->Montage_IsPlaying(ReloadMontage))
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;

		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_ShotGun:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("Rifle");
			break;
		default:
			break;
		}
		
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

FVector APlayerCharacter::GetHitTarget() const
{
	if(Combat == nullptr)return  FVector();
	return Combat->HitTarget;
}

void APlayerCharacter::SetCrosshairShootingFactor() const
{
	if(Combat && Combat->EquippedWeapon)
	{
		Combat->SetCrosshairShootingFactor(Combat->EquippedWeapon->GetDeltaCrosshairShootingFactor());
	}
}

ECombatState APlayerCharacter::GetCombatState() const
{
	if(Combat)
	{
		return Combat->CombatState;
	}
	return ECombatState::ECS_MAX;
}

void APlayerCharacter::CancelCombatComponentFireButtonPressed() const
{
	if(Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void APlayerCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon) const
{
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if(LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void APlayerCharacter::TurnInPlace(float DeltaTime)
{
	if(AO_Yaw>=90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if(AO_Yaw<=-90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if(TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 10.f);
		AO_Yaw = InterpAO_Yaw;
		if(FMath::Abs(AO_Yaw)<3.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
	}
}

void APlayerCharacter::HideCamera()
{
	if(!IsLocallyControlled())return;
	if((FollowCamera->GetComponentLocation()-GetActorLocation()).Size()<CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if(Combat&&Combat->EquippedWeapon&&Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if(Combat&&Combat->EquippedWeapon&&Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void APlayerCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	PlayHitReactMontage();
}

void APlayerCharacter::UpdateHUDHealth()
{
	if(!IsLocallyControlled())return;
	PlayerController = PlayerController == nullptr?Cast<AMyPlayerController>(Controller):PlayerController;
	if(PlayerController)
	{
		PlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void APlayerCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	
	OverlappingWeapon = Weapon;
	if(IsLocallyControlled())
	{
		if(OverlappingWeapon)
			OverlappingWeapon->ShowPickupWidget(true);
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 设置Mapping
	if(APlayerController* pc = CastChecked<APlayerController>(GetController()))
	{
		if(UEnhancedInputLocalPlayerSubsystem* subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(pc->GetLocalPlayer()))
		{
			subsystem->AddMappingContext(InputMappingContext, 100);
		}
	}

	// 绑定事件
	if(UEnhancedInputComponent* inputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if(IA_MoveForward)
		{
			inputComponent->BindAction(IA_MoveForward, ETriggerEvent::Triggered, this, &ThisClass::OnActionMoveForward);
		}

		if(IA_MoveRight)
		{
			inputComponent->BindAction(IA_MoveRight, ETriggerEvent::Triggered, this, &ThisClass::OnActionMoveRight);
		}

		if(IA_LookUp)
		{
			inputComponent->BindAction(IA_LookUp, ETriggerEvent::Triggered, this, &ThisClass::OnActionLookUp);
		}

		if(IA_LookRight)
		{
			inputComponent->BindAction(IA_LookRight, ETriggerEvent::Triggered, this, &ThisClass::OnActionLookRight);
		}

		if(IA_Jump)
		{
			inputComponent->BindAction(IA_Jump, ETriggerEvent::Started, this, &ThisClass::OnActionJump);
		}

		if(IA_Equip)
		{
			inputComponent->BindAction(IA_Equip, ETriggerEvent::Triggered, this, &ThisClass::OnActionEquip);
		}

		if(IA_Crouch)
		{
			inputComponent->BindAction(IA_Crouch, ETriggerEvent::Started, this, &ThisClass::OnActionCrouch);
		}

		if(IA_Aim)
		{
			inputComponent->BindAction(IA_Aim, ETriggerEvent::Started, this, &ThisClass::OnActionAimPressed);
			inputComponent->BindAction(IA_Aim, ETriggerEvent::Completed, this, &ThisClass::OnActionAimReleased);
		}

		if(IA_Fire)
		{
			inputComponent->BindAction(IA_Fire, ETriggerEvent::Triggered, this, &ThisClass::OnActionFirePressed);
			inputComponent->BindAction(IA_Fire, ETriggerEvent::Completed, this, &ThisClass::OnActionFireReleased);
		}

		if(IA_Reload)
		{
			inputComponent->BindAction(IA_Reload, ETriggerEvent::Triggered, this, &ThisClass::OnActionReload);
		}
	}
	
}

bool APlayerCharacter::IsWeaponEquipped() const
{
	return (Combat && Combat->EquippedWeapon);
}

bool APlayerCharacter::IsAiming() const
{
	return (Combat && Combat->bAiming);
}

AWeapon* APlayerCharacter::GetEuippedWeapon() const
{
	if(Combat == nullptr)return nullptr;
	return Combat->EquippedWeapon;
}

void APlayerCharacter::StartHealthRecovery()
{
	GetWorld()->GetTimerManager().SetTimer(HealthRecoveryTimerHandle, this, &APlayerCharacter::RecoverHealthTick, GetWorld()->GetDeltaSeconds(), true);
}

void APlayerCharacter::RecoverHealthTick()
{
	if (Health < MaxHealth)
	{
		float HealthIncreasePerTick = MaxHealth / ((ElimDelay-0.1f) / GetWorld()->GetDeltaSeconds());
		Health += HealthIncreasePerTick;
		
		Health = FMath::Min(Health, MaxHealth);

		UpdateHUDHealth();
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(HealthRecoveryTimerHandle);
	}
}