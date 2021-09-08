// Fill out your copyright notice in the Description page of Project Settings.


#include "PreparationData/FVCH_PreparationDataFunctions.h"
#include "HAL/FileManager.h"
#include "XmlParser.h"
#include "MercatorConvertor/GeoMercatorConvertor.h"
#include "NameEncoder/FNameEncoder.h"
#include "Async/ParallelFor.h"
#include "Misc/FileHelper.h"
//#include "XmlFile.h"

DECLARE_LOG_CATEGORY_CLASS(VCH_PrepDataLog, Log, All);

FVCH_PreparationDataFunctions::FVCH_PreparationDataFunctions()
{
}

FVCH_PreparationDataFunctions::~FVCH_PreparationDataFunctions()
{
}

HeightmapDataMap FVCH_PreparationDataFunctions::GetAllHeightmaps(FString Path, int32 Resolution)
{
	HeightmapDataMap Result;
	TArray<FString>  HeightmapNames;

	if (IFileManager::Get().DirectoryExists(*Path))
	{
		FString Filter = TEXT(".raw");
		FString Mask = Path + TEXT("/*") + Filter;
		IFileManager::Get().FindFiles(HeightmapNames, *Mask, true, false);

		for (const auto& FileName : HeightmapNames)
		{
			auto Heightmap(LoadHeightmap(Path + TEXT("/") + FileName, Resolution));
			if (Heightmap.Num() > 0)
			{
				Result.Add(FileName.Replace(*Filter,TEXT("")), MoveTemp(Heightmap));
			}
		}
	}
	return Result;
}

TArray<uint16> FVCH_PreparationDataFunctions::LoadHeightmap(FString Path, int32 Resolution)
{
	TArray<uint16> Data;
	TArray<uint8> ImportData;
	const int32 SqResolution(Resolution * Resolution);
	if (Resolution > 0 &&
		FFileHelper::LoadFileToArray(ImportData, *Path, FILEREAD_Silent) &&
		ImportData.Num() == SqResolution * 2)
	{
		Data.AddUninitialized(SqResolution);
		//FMemory::Memcpy(Data.GetData(), ImportData.GetData(), ImportData.Num());
		FMemory::Memmove(Data.GetData(), ImportData.GetData(), ImportData.Num());
		return MoveTemp(Data);
	}
	else
	{
		UE_LOG(VCH_PrepDataLog, Warning, TEXT("Bad Heightmap %s"), *Path);
		check(false);
	}
		
	return TArray<uint16>();
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

void FVCH_PreparationDataFunctions::RemoveCrackForHeightmaps(HeightmapDataMap& HeightMaps, FString Mask, int32 Resolution)
{
	FNameEncoder Encoder(Mask);
	if (!Encoder.IsValid())
	{
		UE_LOG(VCH_PrepDataLog, Error, TEXT("Bad mask  %s"), *Mask);
		return;
	}
	TArray<FString> Keys;

	ParallelFor(HeightMaps.GetKeys(Keys), [&](int32 Index)
	{
		int32 LetterIndex(-1), NumericIndex(-1);
		if (!Encoder.GetIndeces(Keys[Index], LetterIndex, NumericIndex))
		{
			//UE_LOG(VCH_PrepDataLog, Error, TEXT("Bad Name for file %s"), *(Keys[Index]));
			return;
		}
		TArray<uint16>& RefToCurrentHeigmap = HeightMaps[Keys[Index]];
		constexpr uint16 u16_Two = 2;

		//Bottom neighbor
		auto BottomNeighbor = Encoder.GetName(LetterIndex + 1, NumericIndex);
		if (HeightMaps.Contains(BottomNeighbor))
		{
			TArray<uint16>& RefToBottomHeightmap = HeightMaps[BottomNeighbor];
			
			const int32 OffsetForCurrent = Resolution * (Resolution - 1);

			for (int i = 0; i < Resolution; ++i)
			{
				const auto IndexForCurrentH = OffsetForCurrent + i;
				const uint16 MidValue = (RefToCurrentHeigmap[IndexForCurrentH] + RefToBottomHeightmap[i]) / u16_Two;
				RefToCurrentHeigmap[IndexForCurrentH] = MidValue;
				RefToBottomHeightmap[i] = MidValue;
			}
		}

		// Right
		auto RightNeighbor = Encoder.GetName(LetterIndex, NumericIndex + 1);
		if (HeightMaps.Contains(RightNeighbor))
		{
			TArray<uint16>& RefToRightHeghtmap = HeightMaps[RightNeighbor];
			for (int i = 0; i < Resolution; ++i)
			{
				const auto IndexForCurrentH = Resolution * i + Resolution - 1;
				const auto IndexForRightH = Resolution * i;
				uint16 MidValue = (RefToCurrentHeigmap[IndexForCurrentH] + RefToRightHeghtmap[IndexForRightH]) / u16_Two;
				RefToCurrentHeigmap[IndexForCurrentH] = MidValue;
				RefToRightHeghtmap[IndexForRightH] = MidValue;
			}
		}
		// to do : maybe make lambda
	}/*, EParallelForFlags::ForceSingleThread*/);

	// Corners 
	constexpr uint16 u16_Four = 4;
	for (auto &Heightmap : HeightMaps)
	{
		int32 LetterIndex, NumericIndex;
		Encoder.GetIndeces(Heightmap.Key, LetterIndex, NumericIndex);

		FString RightName(Encoder.GetName(LetterIndex, NumericIndex + 1));
		FString BottomRightName(Encoder.GetName(LetterIndex + 1, NumericIndex + 1));
		FString BottomName(Encoder.GetName(LetterIndex + 1, NumericIndex));

		bool bContaintRightName(HeightMaps.Contains(RightName));
		bool bContaintBottomRightName(HeightMaps.Contains(BottomRightName));
		bool bContaintBottomName(HeightMaps.Contains(BottomName));

		uint32 SumHeight(Heightmap.Value[Resolution * Resolution - 1]);
		uint32 NumLevels(1);
		auto AddHeightLambda = [&SumHeight, &NumLevels](uint16 Height)
		{
			SumHeight += Height;
			++NumLevels;
			return true;
		};

		bContaintRightName			&& AddHeightLambda(HeightMaps[RightName][Resolution * (Resolution - 1)]);
		bContaintBottomRightName	&& AddHeightLambda(HeightMaps[BottomRightName][0]);
		bContaintBottomName			&& AddHeightLambda(HeightMaps[BottomName][Resolution - 1]);
		
		if (NumLevels == 1)
		{
			continue;
		}

		uint16 MidValue = SumHeight / NumLevels;

		auto SetHeightData = [MidValue](uint16& OutVal)
		{
			OutVal = MidValue;
			return true;
		};
		
		Heightmap.Value[Resolution * Resolution - 1] = MidValue;
		bContaintRightName			&& SetHeightData(HeightMaps[RightName][Resolution * (Resolution - 1)]);
		bContaintBottomRightName	&& SetHeightData(HeightMaps[BottomRightName][0]);
		bContaintBottomName			&& SetHeightData(HeightMaps[BottomName][Resolution - 1]);
	}
}

void FVCH_PreparationDataFunctions::SaveHeightMaps(const HeightmapDataMap & HeightMaps, FString PathToSave)
{
	for (const auto& HeightData : HeightMaps)
	{
		auto& Heightmap = HeightData.Value;
		uint64 DataSize = Heightmap.Num() * 2;
		TArrayView<uint8> SaveData((uint8*)(HeightData.Value.GetData()), DataSize);
		FFileHelper::SaveArrayToFile(SaveData, *(PathToSave + HeightData.Key + TEXT(".raw")));
	}
}

bool FVCH_PreparationDataFunctions::CheckHeightmaps(const HeightmapDataMap& HeightMaps, int32 Resolution, FString Mask)
{
	FNameEncoder Encoder(Mask);
	if (!Encoder.IsValid())
	{
		UE_LOG(VCH_PrepDataLog, Error, TEXT("Bad mask  %s"), *Mask);
		return false;
	}
	TArray<FString> Keys;

	TAtomic<bool> bHasError = false;
	ParallelFor(HeightMaps.GetKeys(Keys), [&](int32 Index)
	{
		int32 LetterIndex(-1), NumericIndex(-1);
		Encoder.GetIndeces(Keys[Index], LetterIndex, NumericIndex);
		const auto& RefToCurrentHeigmap = HeightMaps[Keys[Index]];
		constexpr uint16 u16_Two = 2;
		// not check first and end points
		const auto NumCheckPonints = Resolution /*- 1*/;

		//Bottom neighbor
		auto BottomNeighbor = Encoder.GetName(LetterIndex + 1, NumericIndex);
		if (HeightMaps.Contains(BottomNeighbor))
		{
			const auto& RefToBottomHeightmap = HeightMaps[BottomNeighbor];

			const int32 OffsetForCurrent = Resolution * (Resolution - 1);
			
			for (int i = 0; i < NumCheckPonints - 1; ++i)
			{
				const auto IndexForCurrentH = OffsetForCurrent + i;
				if (RefToCurrentHeigmap[IndexForCurrentH] != RefToBottomHeightmap[i])
				{
					//UE_LOG(VCH_PrepDataLog, Error, TEXT("Bad Value %s = %i, %s = %i , index = %i"), *BottomNeighbor, RefToBottomHeightmap[i], *(Keys[Index]), RefToCurrentHeigmap[IndexForCurrentH], i);
					bHasError = true;
					return;
				}
			}
		}

		// Right
		auto RightNeighbor = Encoder.GetName(LetterIndex, NumericIndex + 1);
		if (HeightMaps.Contains(RightNeighbor))
		{
			const auto& RefToRightHeghtmap = HeightMaps[RightNeighbor];
			for (int i = 0; i < NumCheckPonints; ++i)
			{
				const auto IndexForCurrentH = Resolution * i + Resolution - 1;
				const auto IndexForRightH = Resolution * i;
				if (RefToCurrentHeigmap[IndexForCurrentH] != RefToRightHeghtmap[IndexForRightH])
				{
					bHasError = true;
					return;
				}
			}
		}
	}, EParallelForFlags::ForceSingleThread);
	return bHasError;
}

