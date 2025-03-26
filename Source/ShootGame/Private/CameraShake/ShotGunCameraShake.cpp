// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraShake/ShotGunCameraShake.h"

UShotGunCameraShake::UShotGunCameraShake()
{
	OscillationDuration = 0.3f;  
	OscillationBlendInTime = 0.15f;
	OscillationBlendOutTime = 0.15f;

	// 旋转抖动
	RotOscillation.Pitch.Amplitude = 0.4f;
	RotOscillation.Pitch.Frequency = 20.f;

	RotOscillation.Yaw.Amplitude = 0.4f;
	RotOscillation.Yaw.Frequency = 20.f;

	RotOscillation.Roll.Amplitude = 0.4f;
	RotOscillation.Roll.Frequency = 20.f;
}
