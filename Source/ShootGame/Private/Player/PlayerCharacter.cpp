#include "ShootGame/Public/Player/PlayerCharacter.h"

#include "EnhancedInputSubsystemInterface.h"
#include "../Plugins/EnhancedInput/Source/EnhancedInput/Public/EnhancedInputSubsystems.h"
#include "../Plugins/EnhancedInput/Source/EnhancedInput/Public/EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/BuffComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/CombatComponent.h"
#include "Components/LagCompensationComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameMode/LobbyGameMode.h"
#include "GameMode/ShootGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Net/UnrealNetwork.h"
#include "Player/MyPlayerState.h"
#include "PlayerController/MyPlayerController.h"
#include "PlayerStart/TeamPlayerStart.h"
#include "ShootGame/ShootGame.h"
#include "Sound/SoundCue.h"
#include "Tracks/MovieSceneMaterialTrack.h"
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
	

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	// 延迟补偿只在服务器上
	LagCompensation =
		CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	AttachGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attach Grenade"));
	AttachGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Head = CreateDefaultSubobject<UBoxComponent>(TEXT("Head"));
	Head->SetupAttachment(GetMesh(), FName(TEXT("頭")));
	HitCollisionBoxes.Add(FName("Head"), Head);

	Neck = CreateDefaultSubobject<UBoxComponent>(TEXT("Neck"));
	Neck->SetupAttachment(GetMesh(), FName(TEXT("首")));
	HitCollisionBoxes.Add(FName("Neck"), Neck);

	Spine_1 = CreateDefaultSubobject<UBoxComponent>(TEXT("Spine_1"));
	Spine_1->SetupAttachment(GetMesh(), FName(TEXT("上半身2")));
	HitCollisionBoxes.Add(FName("Spine_1"), Spine_1);

	Pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("Pelvis"));
	Pelvis->SetupAttachment(GetMesh(), FName(TEXT("上半身")));
	HitCollisionBoxes.Add(FName("Pelvis"), Pelvis);

	RightShoulder = CreateDefaultSubobject<UBoxComponent>(TEXT("RightShoulder"));
	RightShoulder->SetupAttachment(GetMesh(), FName(TEXT("右肩C")));
	HitCollisionBoxes.Add(FName("RightShoulder"), RightShoulder);

	LeftShoulder = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftShoulder"));
	LeftShoulder->SetupAttachment(GetMesh(), FName(TEXT("左肩C")));
	HitCollisionBoxes.Add(FName("LeftShoulder"), LeftShoulder);

	RightArmUpper = CreateDefaultSubobject<UBoxComponent>(TEXT("RightArmUpper"));
	RightArmUpper->SetupAttachment(GetMesh(), FName(TEXT("右腕")));
	HitCollisionBoxes.Add(FName("RightArmUpper"), RightArmUpper);

	LeftArmUpper = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftArmUpper"));
	LeftArmUpper->SetupAttachment(GetMesh(), FName(TEXT("左腕")));
	HitCollisionBoxes.Add(FName("LeftArmUpper"), LeftArmUpper);

	RightArmMid = CreateDefaultSubobject<UBoxComponent>(TEXT("RightArmMid"));
	RightArmMid->SetupAttachment(GetMesh(), FName(TEXT("右腕捩")));
	HitCollisionBoxes.Add(FName("RightArmMid"), RightArmMid);

	LeftArmMid = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftArmMid"));
	LeftArmMid->SetupAttachment(GetMesh(), FName(TEXT("左腕捩")));
	HitCollisionBoxes.Add(FName("LeftArmMid"), LeftArmMid);

	RightArmLower = CreateDefaultSubobject<UBoxComponent>(TEXT("RightArmLower"));
	RightArmLower->SetupAttachment(GetMesh(), FName(TEXT("右手捩")));
	HitCollisionBoxes.Add(FName("RightArmLower"), RightArmLower);

	LeftArmLower = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftArmLower"));
	LeftArmLower->SetupAttachment(GetMesh(), FName(TEXT("左手捩")));
	HitCollisionBoxes.Add(FName("LeftArmLower"), LeftArmLower);

	RightHand = CreateDefaultSubobject<UBoxComponent>(TEXT("RightHand"));
	RightHand->SetupAttachment(GetMesh(), FName(TEXT("右手首")));
	HitCollisionBoxes.Add(FName("RightHand"), RightHand);

	LeftHand = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftHand"));
	LeftHand->SetupAttachment(GetMesh(), FName(TEXT("左手首")));
	HitCollisionBoxes.Add(FName("LeftHand"), LeftHand);

	RightAss = CreateDefaultSubobject<UBoxComponent>(TEXT("RightAss"));
	RightAss->SetupAttachment(GetMesh(), FName(TEXT("腰キャンセル右")));
	HitCollisionBoxes.Add(FName("RightAss"), RightAss);

	LeftAss = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftAss"));
	LeftAss->SetupAttachment(GetMesh(), FName(TEXT("腰キャンセル左")));
	HitCollisionBoxes.Add(FName("LeftAss"), LeftAss);

	RightLegUpper = CreateDefaultSubobject<UBoxComponent>(TEXT("RightLegUpper"));
	RightLegUpper->SetupAttachment(GetMesh(), FName(TEXT("右足D")));
	HitCollisionBoxes.Add(FName("RightLegUpper"), RightLegUpper);

	LeftLegUpper = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftLegUpper"));
	LeftLegUpper->SetupAttachment(GetMesh(), FName(TEXT("左足D")));
	HitCollisionBoxes.Add(FName("LeftLegUpper"), LeftLegUpper);

	RightLegLower = CreateDefaultSubobject<UBoxComponent>(TEXT("RightLegLower"));
	RightLegLower->SetupAttachment(GetMesh(), FName(TEXT("右ひざD")));
	HitCollisionBoxes.Add(FName("RightLegLower"), RightLegLower);

	LeftLegLower = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftLegLower"));
	LeftLegLower->SetupAttachment(GetMesh(), FName(TEXT("左ひざD")));
	HitCollisionBoxes.Add(FName("LeftLegLower"), LeftLegLower);

	RightFoot = CreateDefaultSubobject<UBoxComponent>(TEXT("RightFoot"));
	RightFoot->SetupAttachment(GetMesh(), FName(TEXT("右足先EX")));
	HitCollisionBoxes.Add(FName("RightFoot"), RightFoot);

	LeftFoot = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftFoot"));
	LeftFoot->SetupAttachment(GetMesh(), FName(TEXT("左足先EX")));
	HitCollisionBoxes.Add(FName("LeftFoot"), LeftFoot);

	for(const auto& Box:HitCollisionBoxes)
	{
		if(Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
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
	DOREPLIFETIME(APlayerCharacter, Shield);
}

void APlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if(Combat)
	{
		Combat->Character = this;
	}
	if(Buff)
	{
		Buff->Character = this;
		Buff->SetInitialSpeed
		(GetCharacterMovement()->MaxWalkSpeed,
			GetCharacterMovement()->MaxWalkSpeedCrouched);
		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	if(LagCompensation)
	{
		LagCompensation->Character = this;
		if(Controller)
		{
			LagCompensation->Controller = Cast<AMyPlayerController>(Controller);
		}
	}
}

void APlayerCharacter::Elim(bool bPlayerLeftGame)
{
	if(Combat)
	{
		if(Combat->EquippedWeapon)
		{
			Combat->EquippedWeapon->Dropped();
			if(Combat->SecondaryWeapon)
			{
				Combat->SecondaryWeapon->Dropped();
			}
		}
		if(Combat->bHoldingBomb)
		{
			Combat->EquippedBomb->Dropped();
		}
	}
	bLeftGame = bPlayerLeftGame;
	MulticastElim(bPlayerLeftGame);
	StartHealthRecovery();
	StartShieldRecovery();
}

void APlayerCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
	if(PlayerController)
	{
		PlayerController->SetHUDWeaponAmmo(0);
		PlayerController->RemoveCrosshair();
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
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ThisClass::ElimTimerFinished,
		ElimDelay
	);
}

void APlayerCharacter::ElimTimerFinished()
{
	ShootGameMode = ShootGameMode == nullptr ?
		GetWorld()->GetAuthGameMode<AShootGameMode>() : ShootGameMode;
	if(ShootGameMode && !bLeftGame)
	{
		ShootGameMode->RequestRespawn(this, PlayerController);
	}
	else if(bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}

void APlayerCharacter::SetTeamColor(const ETeam Team)
{
	if(GetWorld()->GetFirstPlayerController()->GetPlayerState<AMyPlayerState>()->GetTeam() == Team)
	{
		GetMesh()->SetOverlayMaterial(FriendMat);
	}
	else
	{
		GetMesh()->SetOverlayMaterial(EnemyMat);
	}
	
	/*switch (Team)
	{
	case ETeam::ET_RedTeam:
		if(EnemyMat && GetMesh())
		{
			GetMesh()->SetOverlayMaterial(EnemyMat);
		}
		break;
	case ETeam::ET_BlueTeam:
		if(EnemyMat && GetMesh())
		{
			GetMesh()->SetOverlayMaterial(FriendMat);
		}
		break;
	default:
		break;
	}*/
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

	if(AttachGrenade)
	{
		AttachGrenade->SetVisibility(false);
	}
	
	UpdateHUDHealth();
	UpdateHUDShield();
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	this->UpdateMPC();
	AimOffset(DeltaTime);
	HideCamera();
	PollInit();

	DropShield(DeltaTime);
	
	if (RecoilOffset != FRotator::ZeroRotator || MaxRecoilAmount != FRotator::ZeroRotator)
	{
		if(bIsRecoiling)
		{
			float InterpSpeed = 10.0f; 
			RecoilOffset = FMath::RInterpConstantTo(RecoilOffset, MaxRecoilAmount, DeltaTime, InterpSpeed);

			if (RecoilOffset.Equals(MaxRecoilAmount, 1.f))
			{
				bIsRecoiling = false;
				MaxRecoilAmount = FRotator::ZeroRotator;
			}
		}
		else
		{
			RecoilOffset =
				FMath::RInterpTo(RecoilOffset, FRotator::ZeroRotator, DeltaTime, RecoilRecoverySpeed);
		}
		const FRotator Rotation = GetControlRotation() + RecoilOffset;
		if (RecoilOffset.Equals(FRotator::ZeroRotator, .15f))
		{
			if (APlayerController* PC = Cast<APlayerController>(GetController()))
			{
				PC->SetControlRotation(FollowCamera->GetComponentRotation());
			}
			RecoilOffset = FRotator::ZeroRotator;
			MaxRecoilAmount = FRotator::ZeroRotator;
		}
		else
		{
			FollowCamera->SetWorldRotation(Rotation);
		}
	}
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
	const float inputValue = InputActionValue.Get<float>();
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
	if(bDisableGameplay || Combat == nullptr || Combat->bHoldingBomb)return;
	Jump();
}

void APlayerCharacter::OnActionEquip(const FInputActionValue& InputActionValue)
{
	if(bDisableGameplay || Combat == nullptr)return;
	ServerOnActionEquip();
}

void APlayerCharacter::OnActionCrouch(const FInputActionValue& InputActionValue)
{
	if(bDisableGameplay || Combat == nullptr || Combat->bHoldingBomb)return;
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
	if(bDisableGameplay || Combat == nullptr || Combat->bHoldingBomb)return;
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void APlayerCharacter::OnActionAimReleased(const FInputActionValue& InputActionValue)
{
	if(bDisableGameplay  || Combat == nullptr || Combat->bHoldingBomb)return;
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void APlayerCharacter::OnActionFirePressed(const FInputActionValue& InputActionValue)
{
	if(bDisableGameplay || Combat == nullptr || Combat->bHoldingBomb)return;
	if(Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void APlayerCharacter::OnActionFireReleased(const FInputActionValue& InputActionValue)
{
	if(bDisableGameplay  || Combat == nullptr || Combat->bHoldingBomb)return;
	if(Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void APlayerCharacter::OnActionReload(const FInputActionValue& InputActionValue)
{
	if(bDisableGameplay || Combat == nullptr || Combat->bHoldingBomb)return;
	if(Combat && IsLocallyControlled())
	{
		Combat->Reload();
	}
}

void APlayerCharacter::OnActionTossGrenade(const FInputActionValue& InputActionValue)
{
	if(Combat && !Combat->bHoldingBomb)
	{
		Combat->TossGrenade();
	}
}

void APlayerCharacter::OnActionSwapWeapons(const FInputActionValue& InputActionValue)
{
	if(bDisableGameplay || Combat==nullptr || !Combat->CouldSwapWeapons() || Combat->bHoldingBomb)return;
	ServerOnActionSwapWeapons();
	if(!HasAuthority())
	{
		PlaySwapMontage();
		Combat->CombatState = ECombatState::ECS_SwapingWeapons;
	}
}

void APlayerCharacter::OnActionQuit(const FInputActionValue& InputActionValue)
{
	PlayerController = PlayerController == nullptr ?
			Cast<AMyPlayerController>(Controller):PlayerController;
	UE_LOG(LogTemp, Log, TEXT("quit"));
	if(PlayerController)
	{
		PlayerController->ShowBackToMainMenu();
	}
}

void APlayerCharacter::AimOffset(float DeltaTime)
{
	if(Combat && (Combat->EquippedWeapon==nullptr||Combat->bHoldingBomb))return;
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
		if(Combat->bHoldingBomb)
		{
			Combat->DropBomb();
		}
		else
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
	}
}

void APlayerCharacter::ServerOnActionSwapWeapons_Implementation()
{
	if(Combat)
	{
		Combat->SwapWeapons();
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
	if(Buff && Buff->IsHealing())
	{
		Buff->StopHealing();
	}

	ShootGameMode = ShootGameMode == nullptr ?
		GetWorld()->GetAuthGameMode<AShootGameMode>() : ShootGameMode;
	if(bElimmed || ShootGameMode == nullptr) return;

	Damage = ShootGameMode->CalculateDamage
	(InstigatorController, Controller, Damage);
	
	float DamageToHealth = Damage;
	if(Shield > 0.f)
	{
		if(Shield>=Damage)
		{
			DamageToHealth = 0.f;
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
		}
		else
		{
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
			Shield = 0.f;
		}
	}
	
	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);

	
	UpdateHUDHealth();
	UpdateHUDShield();
	
	if(Damage<1.f)
	{
		PlayHitReactMontage();
	}

	
	if((PlayerController = PlayerController == nullptr ? Cast<AMyPlayerController>(GetController()):PlayerController))
	{
		AMyPlayerController* AttackerController = Cast<AMyPlayerController>(InstigatorController);
		if(Health<=0.f)
		{
			if(ShootGameMode)
			{
				ShootGameMode->PlayerEliminated(this, PlayerController, AttackerController);
				if(APlayerCharacter* Attacker = Cast<APlayerCharacter>(AttackerController->GetPawn()))
				{
					Attacker->PlaySound(ECharacterSoundType::EST_KillSound);
				}
			}
		}
		if(AttackerController)
		{
			AttackerController->ShowHitCrosshairHandle();
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
			OnPlayerStateInitialized();
		}
		
	}
}

void APlayerCharacter::SetSpawnPoint()
{
	if(HasAuthority() && MyPlayerState->GetTeam() != ETeam::ET_NoTeam)
	{
		TArray<AActor*>PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass
			(this, ATeamPlayerStart::StaticClass(), PlayerStarts);

		TArray<ATeamPlayerStart*>TeamPlayerStarts;
		for(auto Start: PlayerStarts)
		{
			ATeamPlayerStart* TeamStart = Cast<ATeamPlayerStart>(Start);
			if(TeamStart && TeamStart->Team==MyPlayerState->GetTeam())
			{
				TeamPlayerStarts.Add(TeamStart);
			}
		}
		if(TeamPlayerStarts.Num() > 0)
		{
			ATeamPlayerStart* ChosenPlayerStart =
				TeamPlayerStarts[FMath::RandRange(0, TeamPlayerStarts.Num() - 1)];
			SetActorLocationAndRotation
				(ChosenPlayerStart->GetActorLocation(), ChosenPlayerStart->GetActorRotation());
		}
	}
}

void APlayerCharacter::OnPlayerStateInitialized()
{
	MyPlayerState->AddToScore(0.f);
	MyPlayerState->AddToDefeats(0);
	if(!IsLocallyControlled())
	{
		SetTeamColor(MyPlayerState->GetTeam());
	}
	SetSpawnPoint();
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
	if(UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		AnimInstance && ReloadMontage)
	{
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
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_ShotGun:
			SectionName = FName("ShotGun");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("SniperRifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("Rifle");
			break;
		default:
			break;
		}
		AnimInstance->Montage_Play(ReloadMontage);
		AnimInstance->Montage_JumpToSection(SectionName, ReloadMontage);
	}
}

void APlayerCharacter::PlayTossGrenadeMontage() const
{
	if(UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && TossGrenadeMontage)
	{
		AnimInstance->Montage_Play(TossGrenadeMontage);
	}
}

void APlayerCharacter::PlaySwapMontage() const
{
	if(UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && TossGrenadeMontage)
	{
		AnimInstance->Montage_Play(SwapMontage);
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

void APlayerCharacter::PlaySound(ECharacterSoundType SoundType)
{
	if(IsLocallyControlled())
	{
		HandlePlaySound(SoundType);
	}
	else
	{
		ClientPlaySound(SoundType);
	}
}

void APlayerCharacter::ClientPlaySound_Implementation(ECharacterSoundType SoundType)
{
	if(IsLocallyControlled())
	{
		HandlePlaySound(SoundType);
	}
}

void APlayerCharacter::HandlePlaySound(const ECharacterSoundType SoundType)
{
	if(bRecentlySound)return;
	USoundCue* SoundToPlay = nullptr;
	switch (SoundType)
	{
	case ECharacterSoundType::EST_JumpSound:
		SoundToPlay = JumpSound;
		break;
	case ECharacterSoundType::EST_BombSound:
		SoundToPlay = BombSound;
		break;
	case ECharacterSoundType::EST_EquipSound:
		SoundToPlay = EquipSound;
		break;
	case ECharacterSoundType::EST_KillSound:
		SoundToPlay = KillSound;
		break;
	case ECharacterSoundType::EST_HealSound:
		SoundToPlay = HealSound;
		break;
	case ECharacterSoundType::EST_ShieldSound:
		SoundToPlay = ShieldSound;
		break;
	case ECharacterSoundType::EST_SpeedSound:
		SoundToPlay = SpeedSound;
		break;
	default:
		break;
	}
	if(SoundToPlay != nullptr)
	{
		UGameplayStatics::PlaySound2D(this, SoundToPlay);
		bRecentlySound = true;
		GetWorldTimerManager().SetTimer(SoundTimer, this, &ThisClass::ResetRecentlySound, SoundCoolDown);
	}
}

void APlayerCharacter::AddRecoil(const FRotator& RecoilAmount, const float RecoverSpeed)
{
	// 计算目标后坐力（累加方式，让后坐力逐渐增加）
	if(MaxRecoilAmount != FRotator::ZeroRotator)
	{
		MaxRecoilAmount += RecoilAmount;
	}
	else
	{
		MaxRecoilAmount = RecoilOffset + RecoilAmount;
	}
	MaxRecoilAmount.Pitch = FMath::Clamp(MaxRecoilAmount.Pitch, 0.f, 15.f); // 限制最大后坐力
	MaxRecoilAmount.Yaw = FMath::Clamp(MaxRecoilAmount.Yaw, -5.f, 5.f);
	// 逐步叠加
	RecoilRecoverySpeed = RecoverSpeed; // 记录恢复速度

	bIsRecoiling = true; // 标记正在后坐
}


void APlayerCharacter::ResetRecentlySound()
{
	bRecentlySound = false;
}

bool APlayerCharacter::IsLocallyReloading()
{
	if(Combat == nullptr)return false;
	return Combat->bLocallyReloading;
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

void APlayerCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if(LastHealth - Health > 1.f)
	{
		PlayHitReactMontage();
	}
}

void APlayerCharacter::DropShield(float DeltaTime)
{
	if(!HasAuthority())return;
	if(Shield <= 0.f)return;
	const float AmountShieldToDrop = DeltaTime*ShieldDropEverySecond;
	Shield = FMath::Clamp(Shield - AmountShieldToDrop, 0.f, MaxShield);
	UpdateHUDShield();
}

void APlayerCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if(LastShield - Shield >= 1.f)
	{
		PlayHitReactMontage();
	}
}

void APlayerCharacter::UpdateHUDHealth()
{
	if(!IsLocallyControlled()) return;
	PlayerController = PlayerController == nullptr?Cast<AMyPlayerController>(Controller):PlayerController;
	if(PlayerController)
	{
		PlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void APlayerCharacter::UpdateHUDShield()
{
	if(!IsLocallyControlled()) return;
	PlayerController = PlayerController == nullptr?Cast<AMyPlayerController>(Controller):PlayerController;
	if(PlayerController)
	{
		PlayerController->SetHUDShield(Shield, MaxShield);
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
			inputComponent->BindAction(IA_Equip, ETriggerEvent::Started, this, &ThisClass::OnActionEquip);
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
			inputComponent->BindAction(IA_Reload, ETriggerEvent::Started, this, &ThisClass::OnActionReload);
		}

		if(IA_TossGrenade)
		{
			inputComponent->BindAction(IA_TossGrenade, ETriggerEvent::Started, this, &ThisClass::OnActionTossGrenade);
		}
		if(IA_SwapWeapons)
		{
			inputComponent->BindAction(IA_SwapWeapons, ETriggerEvent::Started, this, &ThisClass::OnActionSwapWeapons);
		}
		if(IA_Quit)
		{
			inputComponent->BindAction(IA_Quit, ETriggerEvent::Started, this, &ThisClass::OnActionQuit);
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

void APlayerCharacter::ServerLeaveGame_Implementation()
{
	MyPlayerState = MyPlayerState == nullptr ?
		GetPlayerState<AMyPlayerState>() : MyPlayerState;
	ShootGameMode = ShootGameMode == nullptr ?
		GetWorld()->GetAuthGameMode<AShootGameMode>() : ShootGameMode;
	if(ShootGameMode && MyPlayerState)
	{
		ShootGameMode->PlayerLeftGame(MyPlayerState);
	}
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

void APlayerCharacter::StartShieldRecovery()
{
	GetWorld()->GetTimerManager().SetTimer(ShieldRecoveryTimerHandle, this, &APlayerCharacter::RecoverShieldTick, GetWorld()->GetDeltaSeconds(), true);
}

void APlayerCharacter::RecoverShieldTick()
{
	if (Shield < MaxShield)
	{
		float ShieldIncreasePerTick = MaxShield / ((ElimDelay- 0.1f) / GetWorld()->GetDeltaSeconds());
		Shield += ShieldIncreasePerTick;

		Shield = FMath::Min(Shield, MaxShield);

		UpdateHUDShield();
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(ShieldRecoveryTimerHandle);
	}
}