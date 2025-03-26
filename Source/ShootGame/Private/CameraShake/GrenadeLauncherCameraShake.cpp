// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraShake/GrenadeLauncherCameraShake.h"

UGrenadeLauncherCameraShake::UGrenadeLauncherCameraShake()
{
	OscillationDuration = 0.3f;  
	OscillationBlendInTime = 0.1f;
	OscillationBlendOutTime = 0.1f;

	// 旋转抖动
	RotOscillation.Pitch.Amplitude = 0.5f;
	RotOscillation.Pitch.Frequency = 0.1f;

	RotOscillation.Yaw.Amplitude = 0.5f;
	RotOscillation.Yaw.Frequency = 0.1f;

	RotOscillation.Roll.Amplitude = 0.5f;
	RotOscillation.Roll.Frequency = 0.1f;
}
