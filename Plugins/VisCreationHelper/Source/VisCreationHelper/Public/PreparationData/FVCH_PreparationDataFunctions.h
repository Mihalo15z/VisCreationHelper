// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DoubleVect2.h"


/**
 * 
 */

struct FLevelImportData
{
public:
	//FLevelImportData& operator=(FLevelImportData&&) = default;
	FLevelImportData& operator=(const FLevelImportData&) = default;

	// TO DO
	//FString ToString() const;
	//FString ToCSVString() const;
	//static FLevelImportData FromString(FString StrData);
	//static FLevelImportData FromCSV(FString SrtData);
	//static FLevelImportData FromStringArray(const TArray<FString> & StrArrData);

public:

	FDoubleVect2 LatAndLon;
	FDoubleVect2 EndLatAndLon;
	FDoubleVect2 CoordsXY;
	FDoubleVect2 EndCoordsXY;
	FDoubleVect2 SizeXY;
	int32 ImportStatus = 0;
	FString LevelName;

};

using LevelImportedDataMap = TMap<FString, FLevelImportData>;
using HeightmapDataMap = TMap <FString, TArray<uint16> >;

class VISCREATIONHELPER_API FVCH_PreparationDataFunctions
{
public:
	FVCH_PreparationDataFunctions();
	~FVCH_PreparationDataFunctions();
	// make all preparations
	//static void PreparationImportData(FString Path);
	static HeightmapDataMap GetAllHeightmaps(const FString& Path, int32 Resolution);
	static TArray<uint16> LoadHeightmap(const FString& Path, int32 Resolution);
	[[nodiscard]] static TArray<FString> GetLevelNames(const FString& Path);
	//static void CheckMapFiles(FString Path);
	static LevelImportedDataMap GeneratedImportDataTables(const FString& PathToLandXml);
	static void RemoveCrackForHeightmaps(HeightmapDataMap& HeightMaps, const FString& Mask, int32 Resolution);
	static void GetMinMaxForHeightmaps(const HeightmapDataMap& HeightMaps, uint16& OutMin, uint16& OutMax);
	static void CorrectHMapsRange(HeightmapDataMap& HeightMaps, uint32 StableRange = 65'535);
	static void SaveHeightMaps(const HeightmapDataMap& HeightMaps, const FString& PathToSave);
	static bool CheckHeightmaps(const HeightmapDataMap& HeightMaps, int32 Resolution, const FString& Mask);
	//static double CalculateLevelSize(LevelImportedDataMap MapsData);
	static void MakeXMlForMapFiles(const FString& Path, const FString& LandFileName);
	//static void MakeXMLForForestGenerator(FString Path);
	static TArray64<uint8> LoadImage(const FString& Path);
	static void CopyAndRenameHMaps(const FString& HMapPath, const FString& SavePath, const FString& Mask);
	static void GetOffsetAndScale(const FString & PathToLandXML, const FString & ZeroLevelName, const FString & Mask, const double& GeoScale, const double& HeightFactor, const double& LandSize, int32 Resolutinon, FIntPoint & OutOffset, FVector & OutScale);
	static void CheckHasDataInChenel(const TArray64<uint8>& Data, bool bR, bool bG, bool bB, bool bA, uint8 MinVal, int64& NumR, int64& NumG, int64& NumB, int64& numA);
	static void SortWaterAndForestTextures(const FString& InDataPath, const FString& OutWaterPath, const FString& OutForestPath);

	static void ConvertTextureToRaw8b(const FString& path);
};
