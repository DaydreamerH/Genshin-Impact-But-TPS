#include "ShootGame/Public/Player/PlayerCharacter.h"

#include "EnhancedInputSubsystemInterface.h"
#include "../Plugins/EnhancedInput/Source/EnhancedInput/Public/EnhancedInputSubsystems.h"
#include "../Plugins/EnhancedInput/Source/EnhancedInput/Public/EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CombatComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameMode/LobbyGameMode.h"
#include "GameMode/ShootGameMode.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 500.f;
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
	
	GetCharacterMovement()->JumpZVelocity = 500.0f;  // 调整这个值来控制跳跃高度

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
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
}

void APlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if(Combat)
	{
		Combat->Character = this;
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
	}
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	this->UpdateMPC();

	
}

void APlayerCharacter::OnActionMoveForward(const FInputActionValue& InputActionValue)
{
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
	float inputValue = InputActionValue.Get<float>();
	AddControllerPitchInput(inputValue);
}

void APlayerCharacter::OnActionLookRight(const FInputActionValue& InputActionValue)
{
	float inputValue = InputActionValue.Get<float>();
	AddControllerYawInput(inputValue);
}

void APlayerCharacter::OnActionJump(const FInputActionValue& InputActionValue)
{
	Jump();
}

void APlayerCharacter::OnActionEquip(const FInputActionValue& InputActionValue)
{
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
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void APlayerCharacter::OnActionAimReleased(const FInputActionValue& InputActionValue)
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void APlayerCharacter::ServerOnActionEquip_Implementation()
{
	if(Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void APlayerCharacter::UpdateMPC() const
{
	if(PlayerIndex!=-1)
	{
		USkeletalMeshComponent* mesh = GetMesh();
		if(mesh)
		{
			FVector location = mesh->GetComponentLocation();

			if(MPC_Position)
			{
				FLinearColor NewColor(location.X, location.Y, location.Z);
				if(UMaterialParameterCollectionInstance* mpcInst = GetWorld()->GetParameterCollectionInstance(MPC_Position))
				{
					FString ParameterNameString = FString::Printf(TEXT("Position_%d"), PlayerIndex);
					FName ParameterName = FName(*ParameterNameString);

					mpcInst->SetVectorParameterValue(ParameterName, NewColor);
				}
			}
		}
	}
}

void APlayerCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
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


