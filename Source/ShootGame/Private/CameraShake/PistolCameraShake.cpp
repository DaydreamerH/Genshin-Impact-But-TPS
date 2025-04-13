// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraShake/PistolCameraShake.h"

UPistolCameraShake::UPistolCameraShake()
{
	OscillationDuration = 0.1f;  
	OscillationBlendInTime = 0.05f;
	OscillationBlendOutTime = 0.05f;

	RotOscillation.Yaw.Amplitude = 0.2f;
	RotOscillation.Yaw.Frequency = 0.1f;

	RotOscillation.Roll.Amplitude = 2.f;
	RotOscillation.Roll.Frequency = 30.f;
}
