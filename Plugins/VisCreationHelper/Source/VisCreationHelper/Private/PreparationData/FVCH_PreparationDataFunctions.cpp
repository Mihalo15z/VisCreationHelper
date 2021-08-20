// Fill out your copyright notice in the Description page of Project Settings.


#include "PreparationData/FVCH_PreparationDataFunctions.h"
#include "HAL/FileManager.h"
#include "XmlParser.h"
//#include "XmlFile.h"

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
		return {};
	}

	auto RootNoda = LandFile.GetRootNode();
	if (!RootNoda)
	{
		return {};
	}

	for (const auto GeoNoda : RootNoda->GetChildrenNodes())
	{
		// parce data to map
	}
	return {};
}
