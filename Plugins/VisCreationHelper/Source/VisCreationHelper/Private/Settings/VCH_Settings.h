// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "VCH_Settings.generated.h"


UENUM()
enum class ELandscapeImportMode : uint8
{
	Landscape,
	LandscapeStreamingProxy
};

/**
 * 
 */
UCLASS(Config = VCH_Settings, defaultconfig, meta = (DisplayName = "Vis Creations Helper Settings"))
class UVCH_Settings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	//Landscape
	UPROPERTY(Config, EditDefaultsOnly, Category = "Landscape")
		TSoftObjectPtr<UMaterial> LandscapeBaseMaterial;
	UPROPERTY(Config, EditDefaultsOnly, Category = "Landscape")
		int32 Resolution = 255; // make  enum
	UPROPERTY(Config, EditDefaultsOnly, Category = "Landscape")
		int32 NumSubsections = 2; //  make enum
	UPROPERTY(Config, EditDefaultsOnly, Category = "Landscape")
		ELandscapeImportMode ImportMode = ELandscapeImportMode::Landscape;
	UPROPERTY(Config, EditDefaultsOnly, Category = "Landscape")
		int32 CorrectHMapValue = 1352;
	UPROPERTY(Config, EditDefaultsOnly, Category = "Landscape")
		FString LandscapePrefixName = TEXT("LSP");
	/*
	(const FGuid& InGuid,
	int32 InMinX =  0,
	int32 InMinY = 0,
	int32 InMaxX = 510,
	int32 InMaxY = 510,
	int32 InNumSubsections = 2,
	int32 InSubsectionSizeQuads = 255,
	const TMap<FGuid, TArray<uint16>>& InImportHeightData,
	const TCHAR* const InHeightmapFileName,
	const TMap<FGuid, TArray<FLandscapeImportLayerInfo>>& InImportMaterialLayerInfos,
	ELandscapeImportAlphamapType InImportMaterialLayerType,
	const TArray<struct FLandscapeLayer>* InImportLayers = nullptr);

	*/


	// Level
	UPROPERTY(Config, EditDefaultsOnly, Category = "Levels|Name")
		FString NameMask = TEXT("UG@@##");
	// maybe add in private, need convert to double for getter
	UPROPERTY(Config, EditDefaultsOnly, Category = "Levels|Coords")
		/*double*/ FString ZeroLat;
	UPROPERTY(Config, EditDefaultsOnly, Category = "Levels|Coords")
		/*double*/ FString ZeroLon;
	UPROPERTY(Config, EditDefaultsOnly, Category = "Levels|Coords")
		/*double*/ FString Scale;
	UPROPERTY(Config, EditDefaultsOnly, Category = "Levels|Coords")
		/*double*/ FString ZOffset;
	UPROPERTY(Config, EditDefaultsOnly, Category = "Levels|Coords")
		/*double*/ FString DeltaHeight;
	UPROPERTY(Config, EditDefaultsOnly, Category = "Levels|Coords")
		FString ZeroLevelName;
	UPROPERTY(Config, EditDefaultsOnly, Category = "Levels|Coords")
		/*double*/ FString LevelSize = TEXT("0.0");



	// path
	UPROPERTY(Config, EditDefaultsOnly, Category = "Paths")
		FString PathToMaps;
	UPROPERTY(Config, EditDefaultsOnly, Category = "Paths")
		FString PathToMeshes;
	UPROPERTY(Config, EditDefaultsOnly, Category = "Paths")
		FString PathToLandscapeMatAndTextures;
	UPROPERTY(Config, EditDefaultsOnly, Category = "Paths")
		FString PathToGameBlueprints;

	UPROPERTY(Config, EditDefaultsOnly, Category = "Paths|Sufix")
		FString SufixWaterMask = TEXT("_W");
	UPROPERTY(Config, EditDefaultsOnly, Category = "Paths|Sufix")
		FString SufixBaseTexture = TEXT("_B");

	//Import Data Structure
	UPROPERTY(Config, EditDefaultsOnly, Category = "Import|Paths")
		FString MapsDir = TEXT("Maps");
	UPROPERTY(Config, EditDefaultsOnly, Category = "Import|Paths")
		FString HeightmapsDir = TEXT("Heightmaps");
	UPROPERTY(Config, EditDefaultsOnly, Category = "Import|Paths")
		FString FoliageMasksDir = TEXT("FoliageMasks");
	UPROPERTY(Config, EditDefaultsOnly, Category = "Import|Paths")
		FString MeshesDir = TEXT("Meshes");
	UPROPERTY(Config, EditDefaultsOnly, Category = "Import|Paths")
		FString RoadsDir = TEXT("Roads");
	UPROPERTY(Config, EditDefaultsOnly, Category = "Import|Paths")
		FString TexturesDir = TEXT("Textures");
	UPROPERTY(Config, EditDefaultsOnly, Category = "Import|Paths")
		FString WaterMasksDir = TEXT("WaterMasks");
	UPROPERTY(Config, EditDefaultsOnly, Category = "Import|Config")
		FString GeoConfigFieName = TEXT("GeoData.txt");
	UPROPERTY(Config, EditDefaultsOnly, Category = "Import|Config")
		FString LandConfigName = TEXT("land.xml");
	UPROPERTY(Config, EditDefaultsOnly, Category = "Import|Paths")
		FString GlobalImportContentDir = TEXT("_ImportData");

	//UPROPERTY(Config, EditDefaultsOnly, Category = "Import|Textures")
	//	FString TParamName_BaseMask = TEXT("BaseTexture");
	//UPROPERTY(Config, EditDefaultsOnly, Category = "Import|Textures")
	//	FString TParamName_WaterMas = TEXT("Water");
	UPROPERTY(Config, EditDefaultsOnly, Category = "Import|Textures")
		TArray<FString> TextureParamNames = { TEXT("BaseTexture"), TEXT("Water") };
	int32 GetFinalResolution()const
	{
		return Resolution * 2 + 1;
	}
};


USTRUCT()
struct FConfigDataConstructor
{
	GENERATED_USTRUCT_BODY()
public:

};