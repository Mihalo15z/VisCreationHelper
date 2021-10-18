// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VisCreationHelper.h"

/**
 * 
 */
using LevelImportedDataMap = TMap<FString, FLevelImportData>;
using HeightmapDataMap = TMap <FString, TArray<uint16> >;

class VISCREATIONHELPER_API FVCH_PreparationDataFunctions
{
public:
	FVCH_PreparationDataFunctions();
	~FVCH_PreparationDataFunctions();
	// make all preparations
	//static void PreparationImportData(FString Path);
	static HeightmapDataMap GetAllHeightmaps(FString Path, int32 Resolution);
	static TArray<uint16> LoadHeightmap(FString Path, int32 Resolution);
	static TArray<FString> GetLevelNames(FString Path);
	//static void CheckMapFiles(FString Path);
	static LevelImportedDataMap GeneratedImportDataTables(FString PathToLandXml);
	static void RemoveCrackForHeightmaps(HeightmapDataMap& HeightMaps, FString Mask, int32 Resolution);
	static void GetMinMaxForHeightmaps(const HeightmapDataMap& HeightMaps, uint16& OutMin, uint16& OutMax);
	static void CorrectHMapsRange(HeightmapDataMap& HeightMaps, uint16 InMin, uint16 InMax, uint32 StableRange = 65'536);
	static void SaveHeightMaps(const HeightmapDataMap& HeightMaps, FString PathToSave);
	static bool CheckHeightmaps(const HeightmapDataMap& HeightMaps, int32 Resolution, FString Mask);
	//static double CalculateLevelSize(LevelImportedDataMap MapsData);
	static void MakeXMlForMapFiles(FString Path, FString LandFileName);
	//static void MakeXMLForForestGenerator(FString Path);
	static TArray64<uint8> LoadImage(const FString& Path);

};
