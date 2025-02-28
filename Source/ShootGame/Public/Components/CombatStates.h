#pragma once

UENUM(BlueprintType)
enum class ECombatState: uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_Cooling UMETA(DisplayName = "Cooling"),
	ECS_MAX UMETA(DisplayName = "DefaultMax"),
	ECS_Reloading UMETA(DisplayName = "Reloading")
};