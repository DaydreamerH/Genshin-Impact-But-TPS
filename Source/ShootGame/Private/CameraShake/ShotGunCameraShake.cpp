// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraShake/ShotGunCameraShake.h"

UShotGunCameraShake::UShotGunCameraShake()
{
	OscillationDuration = 0.3f;  
	OscillationBlendInTime = 0.15f;
	OscillationBlendOutTime = 0.05f;

	RotOscillation.Yaw.Amplitude = 0.4f;
	RotOscillation.Yaw.Frequency = 1.f;

	RotOscillation.Roll.Amplitude = 0.6f;
	RotOscillation.Roll.Frequency = 30.f;
}
