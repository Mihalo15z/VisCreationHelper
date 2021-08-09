// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "VCH_Settings.generated.h"

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
		int32 Resolution = 511;
	UPROPERTY(Config, EditDefaultsOnly, Category = "Landscape")
		int32 NumSections = 1;

	// Level
	UPROPERTY(Config, EditDefaultsOnly, Category = "Levels|Name")
		FString NameMask = TEXT("UG**##");
	// maybe add in private, need convert to double for getter
	UPROPERTY(Config, EditDefaultsOnly, Category = "Levels|Coords")
		/*double*/ FString ZeroLat;
	UPROPERTY(Config, EditDefaultsOnly, Category = "Levels|Coords")
		/*double*/ FString ZeroLon;
	UPROPERTY(Config, EditDefaultsOnly, Category = "Levels|Coords")
		/*double*/ FString Scale;
	UPROPERTY(Config, EditDefaultsOnly, Category = "Levels|Coords")
		/*double*/ FString ZOffset;



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


};
