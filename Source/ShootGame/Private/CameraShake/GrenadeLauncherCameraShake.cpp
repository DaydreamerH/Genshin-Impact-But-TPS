// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraShake/GrenadeLauncherCameraShake.h"

UGrenadeLauncherCameraShake::UGrenadeLauncherCameraShake()
{
	OscillationDuration = 0.3f;  
	OscillationBlendInTime = 0.1f;
	OscillationBlendOutTime = 0.05f;

	RotOscillation.Yaw.Amplitude = 0.5f;
	RotOscillation.Yaw.Frequency = 0.1f;

	RotOscillation.Roll.Amplitude = 0.6f;
	RotOscillation.Roll.Frequency = 60.f;
}
