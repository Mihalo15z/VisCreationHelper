// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UStaticMesh;
class UActorComponent;
class AActor;
struct FVCHMeshData;

/**
 * 
 */

 //struct for instance data
struct VISCREATIONHELPER_API InstanceParams
{
	int32 MaxDrawDistance;
	int32 MinDrawDistance;
	FName CollisionProfileName;
	uint8 CollisionEnabledType;
	bool bCastDynamicShadow /*= true*/;
	bool bCastStaticShadow /*= false*/;
	bool bEnableDensityScaling /*= false*/;
	bool bAffectDistanceFieldLighting /*= true/false*/;

	UStaticMesh* StaticMesh;
	const TArray<FTransform>& Tarasforms;

	InstanceParams(UStaticMesh* InStaticMesh, const TArray<FTransform>& InTarasforms)
		: MaxDrawDistance(5000.f), MinDrawDistance(4000.f), CollisionProfileName(FName(TEXT("NoCollision"))), CollisionEnabledType(0),
		bCastDynamicShadow(false), bCastStaticShadow(false), bEnableDensityScaling(false), bAffectDistanceFieldLighting(false), StaticMesh(InStaticMesh), Tarasforms(InTarasforms)
	{};

	InstanceParams(const FVCHMeshData& InMeshData, const TArray<FTransform>& InTransforms);
	//InstanceParams(const FVCHMeshData* InMeshData, const TArray<FTransform>& Intransforms);
};

class VISCREATIONHELPER_API VCH_FoliageFunctions
{
public:

	static void  AddInstancesForIFA(const InstanceParams& InParams, int32& OutBossShot, int32& OutNum, int32& OutNumDelete, bool bAttachToComponent = true, bool bUseCalcLocation = true, bool bOnlyLandscape = false, ULevel* level = nullptr, float UnifiedScale = 0.f);
	static FVector CalculateGroundPosition(const FVector& InLocation, AActor** OutActor);
	static FVector CalculateGroundPosition(const FVector& InLocation, UActorComponent** OutComponent);
	// work only loading levels.
	static void RemoveAllInstancesInTheWorld(/* params -include instances, extrude instances, ...*/);
	void SetCollision(bool bEnable);

};
