// Fill out your copyright notice in the Description page of Project Settings.


#include "VCH_Test.h"
#include "MercatorConvertor/GeoMercatorConvertor.h"
#include "PreparationData/FVCH_PreparationDataFunctions.h"
#include "NameEncoder/FNameEncoder.h"
#include "Settings/VCH_Settings.h"
#include "Landscape/FVCH_LandscapeFunctions.h"
#include "Assets/FVCH_AssetFunctions.h"
#include "Async/ParallelFor.h"
#include "Foliage/FVCH_FoliageFunctions.h"
#include "PerfomanceStats/VCHTimerCounter.h"


DECLARE_LOG_CATEGORY_CLASS(VCH_TestLog, Log, All);


FVCH_Test::FVCH_Test()
{
}

FVCH_Test::~FVCH_Test()
{
}

FString FVCH_Test::GlobalTest()
{
	UE_LOG(VCH_TestLog, Log, TEXT("Start Global Test"));

	//TestGetLevelNames();
	//TestLevelsCoords();
	//TestWorkForHeightmaps();

	TestEncoderName();

	//TestImportLandscape();
	//TestExportHeigmaps();

	//TestImportTextureForLandscape();

	//TestForestGeneration();
	return FString();
}

FString FVCH_Test::TestGetLevelNames()
{
	UE_LOG(VCH_TestLog, Log, TEXT("Start TestGetLevelNames"));
	FVCH_PreparationDataFunctions::MakeXMlForMapFiles(FPaths::ProjectDir() + TEXT("_ImportData/Maps"), TEXT("Land.xml"));
	//auto Result = FVCH_PreparationDataFunctions::GetLevelNames(FPaths::ProjectDir() + TEXT("_ImpotrData/Maps"));

	return {};
}

FString FVCH_Test::TestLevelsCoords()
{
	// load and calculated LevelData
	auto ImportData = FVCH_PreparationDataFunctions::GeneratedImportDataTables(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + TEXT("_ImportData/Maps/Land.xml"));
	
	// size Test
	if (ImportData.Num() == 0)
	{
		UE_LOG(VCH_TestLog, Log, TEXT("Empty Import Data"));
		return {};
	}
	const auto CheckSize = ImportData.begin()->Value.SizeXY;
	const auto ZeroPoint = ImportData.begin()->Value.CoordsXY;
	constexpr double Epsilon = 0.1;

	int32 MaxNumberIndex(-1);
	int32 MaxLitterIndex(-1);
	auto SettingsObject = GetDefault<UVCH_Settings>();
	check(SettingsObject);
	FNameEncoder Encoder(SettingsObject->NameMask);

	auto SetMaxIndex = [](const int32 CurrentIndex, int32& MaxIndex)
	{
		if (CurrentIndex > MaxIndex)
		{
			MaxIndex = CurrentIndex;
			return true;
		}
		return false;
	};

	FDoubleVect2 SumLevelsSize(0.0, 0.0);
	int32 NumX(0), NumY(0);
	//  maybe make one lambda [](double& CurrentSize, double Value, int32& CurrentNum) 
	auto SumLevelSizeXLambda = [&SumLevelsSize, &NumX](const FDoubleVect2& Value) mutable
	{
		SumLevelsSize.X += Value.X;
		++NumX;
		return true;
	};

	auto SumLevelSizeYLambda = [&SumLevelsSize, &NumY](const FDoubleVect2& Value) mutable
	{
		SumLevelsSize.Y += Value.Y;
		++NumY;
		return true;
	};


	for (const auto& LevelData : ImportData)
	{
		int32 TemNumberIndex, TempLitterIndex;
		Encoder.GetIndeces(LevelData.Key, TempLitterIndex, TemNumberIndex);

		SetMaxIndex(TemNumberIndex, MaxNumberIndex) && SumLevelSizeXLambda(LevelData.Value.SizeXY);
		SetMaxIndex(TempLitterIndex, MaxLitterIndex) && SumLevelSizeYLambda(LevelData.Value.SizeXY);;

		
		if (!LevelData.Value.SizeXY.EqualTo(CheckSize, Epsilon))
		{
			auto  difCoords = LevelData.Value.CoordsXY - CheckSize * FDoubleVect2(TemNumberIndex - 1, TempLitterIndex) - ZeroPoint;
			UE_LOG(VCH_TestLog, Log, TEXT(" Dif for 0 (%s) Check Size %s, Current Size %s, Name = %s"), *difCoords.ToString(), *CheckSize.ToString(), *LevelData.Value.SizeXY.ToString(), *LevelData.Key);
		}
	}

	// Horizontal Test
	UE_LOG(VCH_TestLog, Warning, TEXT("Horizontal  and Vertical Test"));
	for (int32 i = 0; i < MaxLitterIndex; ++i)
	{
		for (int32 j = 1; j < MaxNumberIndex; ++j)
		{
			auto LevelName00(Encoder.GetName(i, j));
			auto LevelName01(Encoder.GetName(i + 1, j));
			auto LevelName10(Encoder.GetName(i, j + 1));
			auto LevelName11(Encoder.GetName(i + 1, j + 1));
			if (!(ImportData.Contains(LevelName00) && ImportData.Contains(LevelName01) && ImportData.Contains(LevelName10) && ImportData.Contains(LevelName11)))
			{
				UE_LOG(VCH_TestLog, Warning, TEXT("Not valid names"));
				continue;
			}

			if (!ImportData[LevelName00].EndLatAndLon.EqualTo(ImportData[LevelName11].LatAndLon))
			{
				UE_LOG(VCH_TestLog, Log, TEXT("Bad LatLon %s"), *LevelName00, *(ImportData[LevelName00].EndLatAndLon.ToString()));
			}
		}
		
	}

	FDoubleVect2 Num2D(NumX, NumY);
	FDoubleVect2 LevelSizeBySum = SumLevelsSize / Num2D;
	UE_LOG(VCH_TestLog, Warning, TEXT("World Size by Sum %s"), *SumLevelsSize.ToString());
	UE_LOG(VCH_TestLog, Display, TEXT("Level Size by Sum %s"), *LevelSizeBySum.ToString());
	auto WorldSize = (ImportData[Encoder.GetName(MaxLitterIndex, MaxNumberIndex)].EndCoordsXY - ImportData.begin()->Value.CoordsXY);
	UE_LOG(VCH_TestLog, Warning, TEXT("World Size %s"), *WorldSize.ToString());

	UE_LOG(VCH_TestLog, Display, TEXT("Level Size Geo %s"), *(WorldSize/ Num2D).ToString());

	return FString();
}

FString FVCH_Test::TestWorkForHeightmaps()
{
	auto SettingsObject = GetDefault<UVCH_Settings>();
	check(SettingsObject);
	FString PathToHeightmaps = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + TEXT("_ImportData/") + SettingsObject->HeightmapsDir;
	auto HeightmapsMap = FVCH_PreparationDataFunctions::GetAllHeightmaps(PathToHeightmaps, SettingsObject->GetFinalResolution());

	uint16 MinH, MaxH;
	FVCH_PreparationDataFunctions::GetMinMaxForHeightmaps(HeightmapsMap, MinH, MaxH);
	FVCH_PreparationDataFunctions::CorrectHMapsRange(HeightmapsMap, 1352);

	if (FVCH_PreparationDataFunctions::CheckHeightmaps(HeightmapsMap, SettingsObject->GetFinalResolution(), SettingsObject->NameMask))
	{
		UE_LOG(VCH_TestLog, Warning, TEXT("Start Heightmaps have errors"));
	}
	FVCH_PreparationDataFunctions::RemoveCrackForHeightmaps(HeightmapsMap,SettingsObject->NameMask, SettingsObject->GetFinalResolution());
	if (FVCH_PreparationDataFunctions::CheckHeightmaps(HeightmapsMap, SettingsObject->GetFinalResolution(), SettingsObject->NameMask))
	{
		UE_LOG(VCH_TestLog, Error, TEXT("End Heightmaps have errors"));
		return {};
	}
	FVCH_PreparationDataFunctions::SaveHeightMaps(HeightmapsMap, PathToHeightmaps + TEXT("/RightHeightmaps/"));
	return FString();
}

FString FVCH_Test::TestEncoderName()
{
	FNameEncoder Encoder(TEXT("Ad@@##"), '@', '#');
	FString NameTestMask = Encoder.GetName(43, 64);
	FString NameXYTest = Encoder.ToXYName(NameTestMask);
	FString NameFromXY = Encoder.FromXYName(NameXYTest);
	int32 X = -1, Y = -1;
	Encoder.GetIndeces(NameTestMask, X, Y);
	UE_LOG(VCH_TestLog, Log, TEXT("Name  = %s,  X = %i, Y = %i"), *NameTestMask, X, Y);
	UE_LOG(VCH_TestLog, Log, TEXT("NameXY  = %s,  NameFromXY = %s"), *NameXYTest, *NameFromXY);
	UE_LOG(VCH_TestLog, Log, TEXT("Name  = %s,  X = %i, Y = %i"), *Encoder.GetName(0, 0), 0, 0);
	NameTestMask = TEXT("AdFG34");
	Encoder.GetIndeces(NameTestMask, X, Y);
	UE_LOG(VCH_TestLog, Log, TEXT("Name  = %s,  X = %i, Y = %i"), *NameTestMask, X, Y);
	return FString();
}

FString FVCH_Test::TestImportLandscape()
{
	FString Path = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + TEXT("_ImportData");
	//FVCH_LandscapeFunctions::ImpotrLandscapesToNewLevels(Path);
	FVCH_LandscapeFunctions::ImportLandscapeProxyToNewLevels(Path);

	return FString();
}

FString FVCH_Test::TestExportHeigmaps()
{
	auto SettingsObject = GetDefault<UVCH_Settings>();
	FString PathToSave = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + TEXT("_ImportData/") + SettingsObject->HeightmapsDir / TEXT("BackHeightmaps");
	FVCH_LandscapeFunctions::BackupHeightmaps(PathToSave);
	return FString();
}

FString FVCH_Test::TestImportTextureForLandscape()
{
	auto SettingsObject = GetDefault<UVCH_Settings>();
	FString TextureName("BYAV23");
	FString PathToSave("/Game/Textures");
	FString PathToTexture = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + TEXT("_ImportData/") + SettingsObject->TexturesDir;

	auto Texture = FVCH_AssetFunctions::ImportTextureForLandscape(TextureName, PathToTexture, PathToSave);
	if (Texture != nullptr)
	{
		UE_LOG(VCH_TestLog, Log, TEXT("Load texture - Good"));
	}
	else
	{
		UE_LOG(VCH_TestLog, Warning, TEXT("Load Texture - Bad"));
	}
	return FString();
}

FString FVCH_Test::TestForestGeneration()
{
	auto SettingsObject = GetDefault<UVCH_Settings>();
	FString PathToforestMask = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + TEXT("_ImportData/Forest");
	FString PathToHeightmaps = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + TEXT("_ImportData/") + SettingsObject->HeightmapsDir;
	FString PathToWaterMask = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + TEXT("_ImportData/") + SettingsObject->WaterMasksDir;
	FVCH_FoliageFunctions::GenerateForest(PathToHeightmaps, PathToforestMask, PathToWaterMask);
	return FString();
}

FString FVCH_Test::TestClearAllIFA()
{
	FVCH_FoliageFunctions::ClearAllIFA();
	return FString();
}

