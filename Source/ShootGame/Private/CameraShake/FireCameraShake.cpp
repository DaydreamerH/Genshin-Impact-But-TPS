// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraShake/FireCameraShake.h"

UFireCameraShake::UFireCameraShake()
{
	OscillationDuration = 0.2f;  
	OscillationBlendInTime = 0.1f;
	OscillationBlendOutTime = 0.05f;

	RotOscillation.Yaw.Amplitude = 0.2f;
	RotOscillation.Yaw.Frequency = 0.5f;

	RotOscillation.Roll.Amplitude = 0.5f;
	RotOscillation.Roll.Frequency = 60.f;
}