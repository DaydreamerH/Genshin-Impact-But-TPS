#pragma once

UENUM(BlueprintType)
enum class ESoundType : uint8
{
	EST_HealSound UMETA(DisplayName="Heal Sound"),
	EST_ShieldSound UMETA(DisplayName="Shield Sound"),
	EST_SpeedSound UMETA(DisplayName="Speed Sound"),
	EST_JumpSound UMETA(DisplayName="Jump Sound"),
	EST_EquipSound UMETA(DisplayName="Equip Sound"),
	EST_BombSound UMETA(DisplayName="Bomb Sound"),
	EST_KillSound UMETA(DisplayName="Kill Sound"),
	EST_MAX UMETA(DisplayName = "Default")
};