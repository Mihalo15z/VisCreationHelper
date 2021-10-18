// Fill out your copyright notice in the Description page of Project Settings.


#include "PreparationData/FVCH_PreparationDataFunctions.h"
#include "HAL/FileManager.h"
#include "XmlParser.h"
#include "MercatorConvertor/GeoMercatorConvertor.h"
#include "NameEncoder/FNameEncoder.h"
#include "Async/ParallelFor.h"
#include "Misc/FileHelper.h"
#include "IImageWrapper.h"
#include "Modules/ModuleManager.h"
#include "IImageWrapperModule.h"
//#include "LandscapeProxy.h"
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

void FVCH_PreparationDataFunctions::GetMinMaxForHeightmaps(const HeightmapDataMap & HeightMaps, uint16 & OutMin, uint16 & OutMax)
{
	struct TempMinMax
	{
		uint16 MinValue = 65'535;
		uint16 MaxValue = 0;
		//TempMinMax() :MinValue(65'535), MaxValue(0) {};
	};
	TArray<TempMinMax> MinMaxValues;
	MinMaxValues.AddDefaulted(HeightMaps.Num());
	TArray<FString> Keys;
	auto numH = HeightMaps.GetKeys(Keys);
	ParallelFor(HeightMaps.Num(), [&HeightMaps, &MinMaxValues, &Keys](int32 index)
	{
		for (auto Value : HeightMaps[Keys[index]])
		{
			if (Value < MinMaxValues[index].MinValue)
				MinMaxValues[index].MinValue = Value;
			if (Value > MinMaxValues[index].MaxValue)
				MinMaxValues[index].MaxValue = Value;
		}
		UE_LOG(VCH_PrepDataLog, Error, TEXT("Min = %i, Max = %i for %s "), MinMaxValues[index].MinValue, MinMaxValues[index].MaxValue,*Keys[index]);
	}/*, EParallelForFlags::ForceSingleThread*/);

	OutMin = 65'535;
	OutMax = 0;
	for (auto Value : MinMaxValues)
	{
		if (Value.MinValue < OutMin)
			OutMin = Value.MinValue;
		if (Value.MaxValue > OutMax)
			OutMax = Value.MaxValue;
	}
	UE_LOG(VCH_PrepDataLog, Warning, TEXT("Min = %i, Max = %i for Height "), OutMin, OutMax);
}

void FVCH_PreparationDataFunctions::CorrectHMapsRange(HeightmapDataMap & HeightMaps, uint16 InMin, uint16 InMax, uint32 StableRange)
{
	constexpr uint16 MaxValue = 65'535;
	constexpr uint16 MediumValue = 65'536 / 2 - 1;
	constexpr uint16 StableCoof = 65'536 / 4 - 1; 
	constexpr double MedV = 65'536 / 2;
	if (InMax > MediumValue)
	{
		UE_LOG(VCH_PrepDataLog, Warning, TEXT("Hright Stable "));
		return;
	}
	double CurrentRange = InMax - InMin;
	//  test this!!!
	double Coof = static_cast<double>(MediumValue) / static_cast<double>(InMax);
	TArray<FString> Keys;
	auto numH = HeightMaps.GetKeys(Keys);
	ParallelFor(HeightMaps.Num(), [&HeightMaps, &Keys, Coof, StableCoof, MedV](int32 index)
	{
		for (auto& Value : HeightMaps[Keys[index]])
		{
			Value = static_cast<uint16>(static_cast<double>(Value) * Coof + MedV);
			//Value = static_cast<uint16>(static_cast<double>(Value + 1) * Coof + MedV);
		}
	});
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
	});
	return bHasError;
}

//double FVCH_PreparationDataFunctions::CalculateLevelSize(LevelImportedDataMap MapsData)
//{
//	return 0.0;
//}

void FVCH_PreparationDataFunctions::MakeXMlForMapFiles(FString Path, FString LandFileName)
{
	TArray<FString>  Result;

	if (IFileManager::Get().DirectoryExists(*Path))
	{
		FString Filter = TEXT(".map");
		FString Mask = Path + TEXT("/*") + Filter;
		IFileManager::Get().FindFiles(Result, *Mask, true, false);

		if (Result.Num() > 0)
		{
			FString XMl_Data(TEXT("<?xml version=\"1.0\" encoding=\"windows-1251\"?>\n<Land>"));
			auto RemoveNotValidDataLamda = [](TArray<FString>& Data)
			{
				for (auto& st : Data)
				{
					st.ReplaceInline(TEXT(" "), TEXT(""));
				}
			};

			for (const auto& FileName : Result)
			{
				TArray<FString> parsedLines, GeoLine, GeoLineRD;
				if (FFileHelper::LoadANSITextFileToStrings(*(Path + TEXT("/") + FileName), NULL, parsedLines))
				{
					if (parsedLines.Num() < 53)
					{
						UE_LOG(VCH_PrepDataLog, Warning, TEXT("Bad map file, path = %s"), *FileName);
						continue;
					}
					// see *.map file for GlobalMapper
					parsedLines[52].ParseIntoArray(GeoLine, TEXT(","), true);
					parsedLines[50].ParseIntoArray(GeoLineRD, TEXT(","), false);
					if (GeoLine.Num() < 4 || GeoLineRD.Num() < 4)
					{
						UE_LOG(VCH_PrepDataLog, Warning, TEXT("Bad map file, path = %s"), *FileName);
						continue;
					}

					RemoveNotValidDataLamda(GeoLine);
					RemoveNotValidDataLamda(GeoLineRD);

					XMl_Data.Append(FString::Printf(TEXT("\n\t<Sector name=\"%s\"\tleft=\"%s\"\ttop=\"%s\"\tright=\"%s\"\tbottom=\"%s\"/>"), *(FileName.Replace(*Filter, TEXT(""))), *GeoLine[2], *GeoLineRD[3], *GeoLineRD[2], *GeoLine[3]));
				}
			}

			XMl_Data.Append(FString::Printf(TEXT("\n</Land><!-- Total number of files %i -->"), Result.Num()));
			FFileHelper::SaveStringToFile(XMl_Data, *(Path + TEXT("/") + LandFileName));
		}
		else
		{
			UE_LOG(VCH_PrepDataLog, Warning, TEXT("Not Found *.map files, path = %s"), *Path);
		}
	}
}

TArray64<uint8> FVCH_PreparationDataFunctions::LoadImage(const FString& Path)
{
	TArray64<uint8> data;
	if (FFileHelper::LoadFileToArray(data, *Path, FILEREAD_Silent))
	{
		auto& ImageWrapperModule = FModuleManager::GetModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
		TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
		if (ImageWrapper->SetCompressed(data.GetData(), data.Num()) && (ImageWrapper->GetRaw(ERGBFormat::RGBA, 8, data)))
		{
			return data;
		}
		else
		{
			UE_LOG(VCH_PrepDataLog, Error, TEXT("%s"), *("bad file: " + Path));
		}
	}
	return data;
}

//void FVCH_PreparationDataFunctions::MakeXMLForForestGenerator(FString Path)
//{
//		FString Result;
//		Result.Append("<?xml version=\"1.0\" encoding = \"Windows-1252\"?>\n\t<Data>\n\t<Levels>\n");
//		for (auto level : GWorld->GetLevels())
//		{
//			GWorld->SetCurrentLevel(level);
//			//ALandscapeProxy* landscape(nullptr);
//			//landscape = nullptr; // GetLandscapeOfCurrentLevel();
//
//			//if(landscape)
//			//{
//			//	Result.Append("\t\t<info name=\"" + landscape->GetActorLabel() + "\" ");
//			//	FVector origen;
//			//	FVector bound;
//			//	landscape->GetActorBounds(true, origen, bound);
//			//	Result.Append(" origin=\"" + origen.ToString());
//			//	Result.Append("\" bound=\"" + bound.ToString());
//			//	Result.Append("\" location=\"" + landscape->GetActorLocation().ToString());
//			//	Result.Append("\" />\n");
//			//}
//		}
//		Result.Append("\t</Levels>\n</Data>");
//		//FString Path = FPaths::ProjectContentDir() + "backupXML/" + "InfoLevels.xml";
//		FFileHelper::SaveStringToFile(Result, *Path);
//	
//}


