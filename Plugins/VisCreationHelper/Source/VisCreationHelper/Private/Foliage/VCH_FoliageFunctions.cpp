// Fill out your copyright notice in the Description page of Project Settings.


#include "Foliage/VCH_FoliageFunctions.h"
#include "Meshes/VCH_MeshesFunctions.h"


InstanceParams::InstanceParams(const FVCHMeshData & InMeshData, const TArray<FTransform>& InTransforms):Tarasforms(InTransforms)
{
	MaxDrawDistance = InMeshData.MaxCoolDistance;
	MinDrawDistance = InMeshData.MinCoolDistance;
	CollisionProfileName = FName(TEXT("BlockAllDynamic"));
	//ECollisionEnabled CollisionEnabled =/* (ECollisionEnabled::QueryOnly)*/;
	CollisionEnabledType = InMeshData.CollisionType;
	bCastDynamicShadow = InMeshData.bCastShadow/*= true*/;
	bCastStaticShadow = false;
	bEnableDensityScaling = false;
	bAffectDistanceFieldLighting = false;

	StaticMesh = InMeshData.GetMesh();
}

void VCH_FoliageFunctions::AddInstancesForIFA(const InstanceParams & InParams, int32 & OutBossShot, int32 & OutNum, int32 & OutNumDelete, bool bAttachToComponent, bool bUseCalcLocation, bool bOnlyLandscape, ULevel * level, float UnifiedScale)
{
}

FVector VCH_FoliageFunctions::CalculateGroundPosition(const FVector & InLocation, AActor ** OutActor)
{
	return FVector();
}

FVector VCH_FoliageFunctions::CalculateGroundPosition(const FVector & InLocation, UActorComponent ** OutComponent)
{
	return FVector();
}
