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
	static void PreparationImportData(FString Path);
	static HeightmapDataMap GetAllHeightmaps(FString Path);
	static TArray<FString> GetLevelNames(FString Path);
	static void CheckMapFiles(FString Path);
	static LevelImportedDataMap GeneratedImportDataTables(FString PathToLandXml);
	static void RemoveCrackForHeightmaps(const HeightmapDataMap& HeightMaps);
	static void SaveHeightMaps(const HeightmapDataMap& HeightMaps, FString PathToSave);
};
