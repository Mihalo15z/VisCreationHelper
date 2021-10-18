// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Containers/Array.h"


/**
 * 
 */
class REALTIMEFORESTMS_API FForestData
{
public:
	FForestData();
	~FForestData();


	TArray<uint16> heightMap;
	TArray<uint8> forestMask;
	int32 hMapResolution = 0;
	int32 fMaskResolution = 0;
	float hMapPixelSize = 0;
	float fMaskPixelSize = 0;
	int32 psevdoNumInstance = 0;
	
};
