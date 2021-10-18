// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "StaticForestSettings.generated.h"

class UStaticMesh;

USTRUCT()
struct FForestTypeParams
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(Config, EditDefaultsOnly)
		TSoftObjectPtr<UStaticMesh> MeshPath;
	UPROPERTY(EditAnywhere)
		float MinScale = 1.f;
	UPROPERTY(EditAnywhere)
		float MaxScale = 1.f;
	UPROPERTY(EditAnywhere)
		int32 MinDrawDistance = 400000;
	UPROPERTY(EditAnywhere)
		int32 MaxDrawDistance = 500000;
	UPROPERTY(EditAnywhere)
		int32 ChanceInst = 1;
	UPROPERTY(EditAnywhere)
		bool bRandomRotation = false;
};
/**
 * 
 */

UENUM()
enum class EPlaceForestLevel : uint8
{
	LoacalLevelIFA = 0,
	GloabalIFA = 1
};


UCLASS(Config = VCH_Settings, defaultconfig, meta = (DisplayName = "Vis Creations Helper Settings|Forest"))
class VISCREATIONHELPER_API UStaticForestSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(Config, EditDefaultsOnly)
		float Density = 800.f;
	UPROPERTY(Config, EditDefaultsOnly)
		TArray<FForestTypeParams> ForestTypes;
	UPROPERTY(Config, EditDefaultsOnly)
		EPlaceForestLevel PlaceForestLevel = EPlaceForestLevel::LoacalLevelIFA;

};
