// Fill out your copyright notice in the Description page of Project Settings.


#include "ShotGunReloadAnimNotify.h"

#include "Components/CombatComponent.h"
#include "Player/PlayerCharacter.h"

void UShotGunReloadAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,  const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation);
	
	if(MeshComp && MeshComp->GetOwner())
	{
		if(APlayerCharacter* PlayerCharacter =
			Cast<APlayerCharacter>(MeshComp->GetOwner());
			PlayerCharacter && PlayerCharacter->GetCombat() && PlayerCharacter->HasAuthority())
		{
			// 记录触发时间（精确到毫秒）
			// FString Timestamp = FDateTime::Now().ToString(TEXT("%H:%M:%S.%s"));
			// UE_LOG(LogTemp, Log, TEXT("[%s] AnimNotify Triggered"), *Timestamp);

			// 记录调用堆栈（仅开发版本有效）
			// FString Callstack = FFrame::GetScriptCallstack(true);
			// UE_LOG(LogTemp, Log, TEXT("Callstack:\n%s"), *Callstack);
			PlayerCharacter->GetCombat()->ShotGunShellReload();
		}
	}

	
}
