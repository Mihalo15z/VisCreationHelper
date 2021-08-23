// Fill out your copyright notice in the Description page of Project Settings.


#include "PreparationData/FVCH_PreparationDataFunctions.h"
#include "HAL/FileManager.h"
#include "XmlParser.h"
#include "MercatorConvertor/GeoMercatorConvertor.h"
//#include "XmlFile.h"

DECLARE_LOG_CATEGORY_CLASS(VCH_PrepDataLog, Log, All);

FVCH_PreparationDataFunctions::FVCH_PreparationDataFunctions()
{
}

FVCH_PreparationDataFunctions::~FVCH_PreparationDataFunctions()
{
}

TArray<FString> FVCH_PreparationDataFunctions::GetLevelNames(FString Path)
{
	TArray<FString>  Result;
	
	if (IFileManager::Get().DirectoryExists(*Path))
	{
		FString Filter =TEXT(".map");
		FString Mask = Path +TEXT("/*") + Filter;
		IFileManager::Get().FindFiles(Result, *Mask, true, false);
		for (auto& name : Result)
		{
			name.Replace(*Filter, TEXT(""));
		}
	}

	
	return MoveTemp(Result);
}

LevelImportedDataMap FVCH_PreparationDataFunctions::GeneratedImportDataTables(FString Path)
{
	FXmlFile LandFile;
	if(!LandFile.LoadFile(Path))
	{
		UE_LOG(VCH_PrepDataLog, Warning, TEXT("Don't open file %s"), *Path);
		return {};
	}

	auto RootNoda = LandFile.GetRootNode();
	if (!RootNoda)
	{
		UE_LOG(VCH_PrepDataLog, Warning, TEXT("Bad RootNode for file %s"), *Path);
		return {};
	}

	LevelImportedDataMap Result;
	for (const auto GeoNoda : RootNoda->GetChildrenNodes())
	{
		FLevelImportData LevelData;
		LevelData.LevelName = GeoNoda->GetAttribute(TEXT("name"));

		LevelData.LatAndLon.X = FCString::Atod(*GeoNoda->GetAttribute(TEXT("top")));
		LevelData.LatAndLon.Y = FCString::Atod(*GeoNoda->GetAttribute(TEXT("left")));
		LevelData.EndLatAndLon.X = FCString::Atod(*GeoNoda->GetAttribute(TEXT("bottom")));
		LevelData.EndLatAndLon.Y = FCString::Atod(*GeoNoda->GetAttribute(TEXT("right")));

		LevelData.CoordsXY = FGeoMercatorConvertor::GetGeoForMercator(LevelData.LatAndLon);
		LevelData.EndCoordsXY = FGeoMercatorConvertor::GetGeoForMercator(LevelData.EndLatAndLon);

		LevelData.SizeXY = LevelData.EndCoordsXY - LevelData.CoordsXY;

		Result.Add(LevelData.LevelName, LevelData);
	}
	return Result;
}
