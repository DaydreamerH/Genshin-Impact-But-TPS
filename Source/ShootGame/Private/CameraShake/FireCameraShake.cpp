// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraShake/FireCameraShake.h"

UFireCameraShake::UFireCameraShake()
{
	OscillationDuration = 0.2f;  
	OscillationBlendInTime = 0.1f;
	OscillationBlendOutTime = 0.1f;

	// 旋转抖动
	RotOscillation.Pitch.Amplitude = 0.2f;
	RotOscillation.Pitch.Frequency = 0.1f;

	RotOscillation.Yaw.Amplitude = 0.2f;
	RotOscillation.Yaw.Frequency = 0.1f;

	RotOscillation.Roll.Amplitude = 0.2f;
	RotOscillation.Roll.Frequency = 0.1f;
}

void UFireCameraShake::SetCameraShakeParams(FUCameraShakeParams& CameraShakeParams)
{
	OscillationDuration = CameraShakeParams.OscillationDuration;
	OscillationBlendInTime = CameraShakeParams.OscillationBlendInTime;
	OscillationBlendOutTime = CameraShakeParams.OscillationBlendOutTime;
	
	RotOscillation.Pitch.Amplitude = CameraShakeParams.RotOscillationPitchAmplitude;
	RotOscillation.Pitch.Frequency = CameraShakeParams.RotOscillationPitchFrequency;
	
	RotOscillation.Yaw.Amplitude = CameraShakeParams.RotOscillationYawAmplitude;
	RotOscillation.Yaw.Frequency = CameraShakeParams.RotOscillationYawFrequency;
	
	RotOscillation.Roll.Amplitude = CameraShakeParams.RotOscillationRollAmplitude;
	RotOscillation.Roll.Frequency = CameraShakeParams.RotOscillationRollFrequency;
}
