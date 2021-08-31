// Fill out your copyright notice in the Description page of Project Settings.


#include "Landscape/FVCH_LandscapeFunctions.h"
#include "CoreGlobals.h"
#include "Misc/FeedbackContext.h"
#include "Settings/VCH_Settings.h"
#include "PreparationData/FVCH_PreparationDataFunctions.h"
#include "NameEncoder/FNameEncoder.h"
#include "FileHelpers.h"
#include "Landscape.h"
#include "LandscapeStreamingProxy.h"
#include "LandscapeInfo.h"

DECLARE_LOG_CATEGORY_CLASS(VCH_LandscapeLog, Log, All);


FVCH_LandscapeFunctions::FVCH_LandscapeFunctions()
{
}

FVCH_LandscapeFunctions::~FVCH_LandscapeFunctions()
{
}

void FVCH_LandscapeFunctions::ImpotrLandscapesToNewLevels(FString PathToImportData)
{
	GWarn->BeginSlowTask(FText::FromString(TEXT("Import Lamdscapes")), true);
	const FString SlashStr = TEXT("/");
	auto ConfigObject = GetDefault<UVCH_Settings>();

	// load base landscape material
	auto LandscapeBaseMaterial = ConfigObject->LandscapeBaseMaterial.LoadSynchronous();
	if (!LandscapeBaseMaterial)
	{
		UE_LOG(VCH_LandscapeLog, Error, TEXT("Bad path to LandscapeMaterial %s"), *ConfigObject->LandscapeBaseMaterial.ToString());
		GWarn->EndSlowTask();
		return;
	}
	// load Heightmaps (and Remover crack? mb this is different function (work oly *.raw files(open - edit - save);
	FString PathToHeightmaps = PathToImportData + SlashStr + ConfigObject->HeightmapsDir;
	auto HeightMaps(FVCH_PreparationDataFunctions::GetAllHeightmaps(PathToHeightmaps, ConfigObject->GetFinalResolution()));
	FVCH_PreparationDataFunctions::RemoveCrackForHeightmaps(HeightMaps, *ConfigObject->NameMask, ConfigObject->GetFinalResolution());
	// load geo data 
	FString PathToLandConfig = PathToImportData + SlashStr + ConfigObject->MapsDir + SlashStr + ConfigObject->LandConfigName;
	auto LevelData(FVCH_PreparationDataFunctions::GeneratedImportDataTables(PathToLandConfig));
	// calculate levelSize(mb calc to GeneratedImportDataTables
	double LevelSize = FCString::Atod(*ConfigObject->LevelSize);
	double LevelZOffset = FCString::Atod(*ConfigObject->ZOffset);
	FRotator LandRotation = FRotator::ZeroRotator;
	FVector  LandPosition = FVector(0.f, 0.f, LevelZOffset * 100.f);
	FVector LandScale(LevelSize / (ConfigObject->GetFinalResolution() - 1.0), LevelSize / (ConfigObject->GetFinalResolution() - 1.0), 100.f);
	//const auto& FirstLevelData = *LevelData.begin();
	//const auto& LastLevelData = *LevelData.end();
	//FirstLevelData.Value.CoordsXY

	
	
	//Test HeightMaps Size;

	// test for height maps data (debug)
	FNameEncoder Encoder(ConfigObject->NameMask);
	if (!Encoder.IsValid())
	{
		UE_LOG(VCH_LandscapeLog, Error, TEXT("Bad mask %s"), *ConfigObject->NameMask);
		GWarn->EndSlowTask();
		return;
	}

	int32 ZeroLitterIndex(-1), ZeroNumericIndex(-1);
	if (!Encoder.GetIndeces(ConfigObject->ZeroLevelName, ZeroLitterIndex, ZeroNumericIndex))
	{
		UE_LOG(VCH_LandscapeLog, Error, TEXT("Bad ZeroName %s"), *ConfigObject->ZeroLevelName);
		GWarn->EndSlowTask();
		return;
	}

	int32 CurrentLitterIndex, CurrentNumericIndex;
	int32 LitterOffset, NumericOffset;
	FString WorldRootPath = /*TEXT("Game/")*/FPaths::ProjectContentDir() + ConfigObject->PathToMaps + SlashStr;
	const int32 minX(0);
	const int32 minY(0);
	const int32 MaxX(ConfigObject->Resolution * ConfigObject->NumSubsections);
	const int32 MaxY(MaxX);
	const int32 NumSubsections(ConfigObject->NumSubsections);
	const int32 Resolution(ConfigObject->Resolution);
	FString HMapExtension = TEXT(".raw");
	for (const auto& HeightMap : HeightMaps)
	{
		if (Encoder.GetIndeces(HeightMap.Key, CurrentLitterIndex, CurrentNumericIndex))
		{
			LitterOffset = CurrentLitterIndex - ZeroLitterIndex;
			NumericOffset = CurrentNumericIndex - ZeroNumericIndex;
			
			FString MapFileName = WorldRootPath + HeightMap.Key + FPackageName::GetMapPackageExtension();
			// Create a new world - so we can 'borrow' its level
			UWorld* NewWorld = UWorld::CreateWorld(EWorldType::None, false);
			check(NewWorld);

			bool bSaved = FEditorFileUtils::SaveLevel(NewWorld->PersistentLevel, *MapFileName);
			if (bSaved)
			{
				auto Landscape = NewWorld->SpawnActor<ALandscape>(LandPosition, LandRotation);
				Landscape->SetActorScale3D(LandScale);
				//Landscape->EditorSetLandscapeMaterial(LandscapeBaseMaterial);
				Landscape->LandscapeMaterial = LandscapeBaseMaterial;
				//TArray<FLandscapeImportLayerInfo> LayerInfos;
				TMap<FGuid, TArray<uint16>> HeightmapDataPerLayers;
				HeightmapDataPerLayers.Add(FGuid(), HeightMap.Value);
				TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayer;
				MaterialLayerDataPerLayer.Add(FGuid(), TArray<FLandscapeImportLayerInfo>());
				auto Guid = FGuid::NewGuid();
				Landscape->Import(
					Guid
					, minX
					, minY
					, MaxX
					, MaxY
					, NumSubsections
					, Resolution
					, HeightmapDataPerLayers
					, *(PathToHeightmaps + SlashStr + HeightMap.Key + HMapExtension)
					, MaterialLayerDataPerLayer
					, ELandscapeImportAlphamapType::Layered);
				FVector Offset(NumericOffset * LevelSize, LitterOffset * LevelSize, 0.f);
				if (Offset != FVector::ZeroVector)
				{
					NewWorld->PersistentLevel->ApplyWorldOffset(Offset, false);

					for (AActor* Actor : NewWorld->PersistentLevel->Actors)
					{
						if (Actor != nullptr)
						{
							GEditor->BroadcastOnActorMoved(Actor);
						}
					}
				}

				FEditorFileUtils::SaveLevel(NewWorld->PersistentLevel, *MapFileName);
			}

			// Destroy the new world we created and collect the garbage
			NewWorld->DestroyWorld(false);
			CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
		}
		else
		{
			UE_LOG(VCH_LandscapeLog, Error, TEXT("Bad LevelName %s"), *HeightMap.Key);
		}
	}

		// BEGINN LOOP For HeightMaps
	//for(const auto& )
	//Update GWarn
	//make new level
	// save new level
	// load/import textures for landscape material;
	// create material instances
	// SetMaterialsParams(textures, offset, ...)
	// create Landscape
	// import landscape data
	// add CoordInfo in Tag for Landscape
	// set Level offset
	// save level
	// Destroy the new world we created and collect the garbage
	//NewWorld->DestroyWorld(false);
	//CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	// Move/Remove file *.map in import folder
		// END LOOP For HeightMaps





	//GWarn->StatusUpdate(TileIndex, ImportSettings.HeightmapFileList.Num(), FText::Format(LOCTEXT("ImportingLandscapeTiles", "Importing landscape tiles: {0}"), FText::FromString(TileName)));

	GWarn->EndSlowTask();
}

void FVCH_LandscapeFunctions::ImpoertLandscapeProxyToNewLevels(FString PathToImportData)
{
	GWarn->BeginSlowTask(FText::FromString(TEXT("Import LamdscapeStreamingProxy")), true);

	const FString SlashStr = TEXT("/");
	auto ConfigObject = GetDefault<UVCH_Settings>();

	// load base landscape material
	auto LandscapeBaseMaterial = ConfigObject->LandscapeBaseMaterial.LoadSynchronous();
	if (!LandscapeBaseMaterial)
	{
		UE_LOG(VCH_LandscapeLog, Error, TEXT("Bad path to LandscapeMaterial %s"), *ConfigObject->LandscapeBaseMaterial.ToString());
		GWarn->EndSlowTask();
		return;
	}
	// load Heightmaps (and Remover crack? mb this is different function (work oly *.raw files(open - edit - save);
	FString PathToHeightmaps = PathToImportData + SlashStr + ConfigObject->HeightmapsDir;
	auto HeightMaps(FVCH_PreparationDataFunctions::GetAllHeightmaps(PathToHeightmaps, ConfigObject->GetFinalResolution()));
	FVCH_PreparationDataFunctions::RemoveCrackForHeightmaps(HeightMaps, *ConfigObject->NameMask, ConfigObject->GetFinalResolution());
	// load geo data 
	FString PathToLandConfig = PathToImportData + SlashStr + ConfigObject->MapsDir + SlashStr + ConfigObject->LandConfigName;
	auto LevelData(FVCH_PreparationDataFunctions::GeneratedImportDataTables(PathToLandConfig));
	// calculate levelSize(mb calc to GeneratedImportDataTables
	double LevelSize = FCString::Atod(*ConfigObject->LevelSize);
	double LevelZOffset = FCString::Atod(*ConfigObject->ZOffset);
	FRotator LandRotation(FRotator::ZeroRotator);
	FVector  LandPosition(0.f, 0.f, LevelZOffset * 100.f);
	FVector LandScale(LevelSize / (ConfigObject->GetFinalResolution() - 1.0), LevelSize / (ConfigObject->GetFinalResolution() - 1.0), 100.f);
	//const auto& FirstLevelData = *LevelData.begin();
	//const auto& LastLevelData = *LevelData.end();
	//FirstLevelData.Value.CoordsXY



	//Test HeightMaps Size;

	// test for height maps data (debug)
	FNameEncoder Encoder(ConfigObject->NameMask);
	if (!Encoder.IsValid())
	{
		UE_LOG(VCH_LandscapeLog, Error, TEXT("Bad mask %s"), *ConfigObject->NameMask);
		GWarn->EndSlowTask();
		return;
	}

	int32 ZeroLitterIndex(-1), ZeroNumericIndex(-1);
	if (!Encoder.GetIndeces(ConfigObject->ZeroLevelName, ZeroLitterIndex, ZeroNumericIndex))
	{
		UE_LOG(VCH_LandscapeLog, Error, TEXT("Bad ZeroName %s"), *ConfigObject->ZeroLevelName);
		GWarn->EndSlowTask();
		return;
	}

	int32 CurrentLitterIndex, CurrentNumericIndex;
	int32 LitterOffset, NumericOffset;
	FString WorldRootPath = /*TEXT("Game/")*/FPaths::ProjectContentDir() + ConfigObject->PathToMaps + SlashStr;
	const int32 minX(0);
	const int32 minY(0);
	const int32 MaxX(ConfigObject->Resolution * ConfigObject->NumSubsections);
	const int32 MaxY(MaxX);
	const int32 NumSubsections(ConfigObject->NumSubsections);
	const int32 Resolution(ConfigObject->Resolution);
	FString HMapExtension = TEXT(".raw");

	FGuid LandscapeGuid = FGuid::NewGuid();
	ALandscape* Landscape = GWorld->SpawnActor<ALandscape>();
	// Landscape->SetActorTransform(FTransform(FQuat::Identity, FVector::ZeroVector, FVector(100, 100, 100)));
	Landscape->SetActorLocation(LandPosition);
	Landscape->SetActorScale3D(LandScale);
	Landscape->SetActorRotation(LandRotation);
	// Setup layers list for importing
	TArray<FLandscapeImportLayerInfo> ImportLayers;
	//SetupLandscapeImportLayers(ImportSettings, GWorld->GetOutermost()->GetName(), INDEX_NONE, ImportLayers);
	//Landscape->Import()
	// Set landscape configuration
	//Landscape->LandscapeMaterial = ImportSettings.LandscapeMaterial.Get();
	//Landscape->NumSubsections
	Landscape->ComponentSizeQuads = 510;
	Landscape->NumSubsections = 2;
	Landscape->SubsectionSizeQuads = 255;
	Landscape->SetLandscapeGuid(LandscapeGuid);
	//for (const auto& ImportLayerInfo : ImportLayers)
	//{
	//	Landscape->EditorLayerSettings.Add(FLandscapeEditorLayerSettings(ImportLayerInfo.LayerInfo));
	//}
	Landscape->CreateLandscapeInfo();
	
	for (const auto& HeightMap : HeightMaps)
	{
		if (Encoder.GetIndeces(HeightMap.Key, CurrentLitterIndex, CurrentNumericIndex))
		{
			LitterOffset = CurrentLitterIndex - ZeroLitterIndex;
			NumericOffset = CurrentNumericIndex - ZeroNumericIndex;

			FString MapFileName = WorldRootPath + HeightMap.Key + FPackageName::GetMapPackageExtension();
			// Create a new world - so we can 'borrow' its level
			UWorld* NewWorld = UWorld::CreateWorld(EWorldType::None, false);
			check(NewWorld);

			bool bSaved = FEditorFileUtils::SaveLevel(NewWorld->PersistentLevel, *MapFileName);
			if (bSaved)
			{
				ALandscapeProxy* LandscapeProxy = NewWorld->SpawnActor<ALandscapeStreamingProxy>();
				//Landscape->EditorSetLandscapeMaterial(LandscapeBaseMaterial);
				LandscapeProxy->LandscapeMaterial = LandscapeBaseMaterial;
				//TArray<FLandscapeImportLayerInfo> LayerInfos;
				TMap<FGuid, TArray<uint16>> HeightmapDataPerLayers;
				HeightmapDataPerLayers.Add(FGuid(), HeightMap.Value);
				TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayer;
				MaterialLayerDataPerLayer.Add(FGuid(), TArray<FLandscapeImportLayerInfo>());
				auto Guid = FGuid::NewGuid();
				LandscapeProxy->Import(
					Guid
					, minX
					, minY
					, MaxX
					, MaxY
					, NumSubsections
					, Resolution
					, HeightmapDataPerLayers
					, *(PathToHeightmaps + SlashStr + HeightMap.Key + HMapExtension)
					, MaterialLayerDataPerLayer
					, ELandscapeImportAlphamapType::Layered);
				LandscapeProxy->GetSharedProperties(Landscape);
				//if (LandscapeProxy)
				
				LandscapeProxy->SetActorLocation(LandPosition);
				LandscapeProxy->SetActorScale3D(LandScale);
				LandscapeProxy->SetActorRotation(LandRotation);
				LandscapeProxy->SetActorLabel(HeightMap.Key + TEXT("LandscapeStreming"));
				//FString NameToFind;
				//int32 LetterIndex, NumericIndex;
				//Encoder.GetIndeces(i, LetterIndex, NumericIndex);
				//int32 XCof = config.startX - NumericIndex;
				//int32 YCof = config.startY - LetterIndex;

				FVector Offset(NumericOffset * LevelSize, LitterOffset * LevelSize, 0.f);
				if (Offset != FVector::ZeroVector)
				{
					NewWorld->PersistentLevel->ApplyWorldOffset(Offset, false);

					for (AActor* Actor : NewWorld->PersistentLevel->Actors)
					{
						if (Actor != nullptr)
						{
							GEditor->BroadcastOnActorMoved(Actor);
						}
					}
				}	

				if (LandscapeProxy)
				{
					FVector	DrawScale = LandscapeProxy->GetRootComponent()->GetRelativeScale3D();
					FIntPoint QuadsSpaceOffset;
					FIntPoint QuadsSpaceOffset2(510, 510);
					QuadsSpaceOffset.X = FMath::RoundToInt(Offset.X / DrawScale.X);
					QuadsSpaceOffset.Y = FMath::RoundToInt(Offset.Y / DrawScale.Y);
					LandscapeProxy->SetAbsoluteSectionBase(QuadsSpaceOffset + LandscapeProxy->LandscapeSectionOffset + QuadsSpaceOffset2);
					UE_LOG(VCH_LandscapeLog, Warning, TEXT("%s  section offset %s"), *(QuadsSpaceOffset + LandscapeProxy->LandscapeSectionOffset).ToString(), *LandscapeProxy->LandscapeSectionOffset.ToString());
					// UpdateLandscapeSectionsOffset(FIntPoint(Offset.X, Offset.Y), land); // section offset is 2D 
					bool bShowWarnings = false;
					ULandscapeInfo::RecreateLandscapeInfo(GWorld, bShowWarnings);
				}
				FEditorFileUtils::SaveLevel(NewWorld->PersistentLevel, *MapFileName);
				//if (BaseMaterial)
				//{
				//	ApplyMaterial(i, LandscapeProxy);
				//}
			}
			NewWorld->DestroyWorld(false);
			CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
		}
	}
	//GWarn->StatusUpdate(TileIndex, ImportSettings.HeightmapFileList.Num(), FText::Format(LOCTEXT("ImportingLandscapeTiles", "Importing landscape tiles: {0}"), FText::FromString(TileName)));

	GWarn->EndSlowTask();
}

void FVCH_LandscapeFunctions::BackupHeightmaps(FString PathToSave)
{
}

/*
void SWorldCreator::MyCreateWorld()
 {
	 TArray<FString> HeightmapsPath;
	 FString HMPath = WorkDirectory + "\\Heightmaps";



	 FString GeoPath = WorkDirectory + "/GeoData.txt";
	 FConfigReader config(GeoPath);
	 MaskName = config.MaskName;

	// FTransform LandTransform;
	 FVector LandLocation(0.f, 0.f, config.height * 100);
	 FRotator LandRotation(0.f, 0.f, 0.f);
	 double scaleFactor = (config.sizeLandscape / static_cast<double>(config.resolution - 1)) ;
	 double heightFact = static_cast<double>(config.height)* 0.390625;
	 FVector LandScale(scaleFactor, scaleFactor, heightFact);

	 int32 count = 0;
	 FText ProgressMessage;
	 ErrorsLog.Empty();
	 if (GetHeightmaps(HeightmapsPath, HMPath))
	 {
		 int32 total = HeightmapsPath.Num();
		 BaseMaterial = LoadBaseMaterial();
		 if (!BaseMaterial)
		 {
			 FText NewError = FText::FromString("Base material haven't loaded");
			 ErrorsLog += NewError.ToString();
			 ErrorsLog += "\n";
		 }
		 HeightmapsData = FillHeightmapsData(HeightmapsPath, config.resolution);
		 AlignHeightsToNeighbors(config.resolution);

		 const  FMyTiledLandscapeImportSettings ImportSettings;

		 FGuid LandscapeGuid = FGuid::NewGuid();

		 ALandscape* Landscape = GWorld->SpawnActor<ALandscape>();
		 // Landscape->SetActorTransform(FTransform(FQuat::Identity, FVector::ZeroVector, FVector(100, 100, 100)));
		 Landscape->SetActorLocation(LandLocation);
		 Landscape->SetActorScale3D(LandScale);
		 Landscape->SetActorRotation(LandRotation);
		 // Setup layers list for importing
		 TArray<FLandscapeImportLayerInfo> ImportLayers;
		 SetupLandscapeImportLayers(ImportSettings, GWorld->GetOutermost()->GetName(), INDEX_NONE, ImportLayers);
		 //Landscape->Import()
		 // Set landscape configuration
		 //Landscape->LandscapeMaterial = ImportSettings.LandscapeMaterial.Get();
		 //Landscape->NumSubsections
		 Landscape->ComponentSizeQuads = 510/;
		 Landscape->NumSubsections = 2;
		 Landscape->SubsectionSizeQuads = 255;
		 Landscape->SetLandscapeGuid(LandscapeGuid);
		 for (const auto& ImportLayerInfo : ImportLayers)
		 {
			 Landscape->EditorLayerSettings.Add(FLandscapeEditorLayerSettings(ImportLayerInfo.LayerInfo));
		 }
		 Landscape->CreateLandscapeInfo();

		 // FVector TileScale;
		 EncoderName encoder(MaskName);
		 for (const auto& i : HeightmapsPath)
		 {

			 FString HM_FileName(i);
			 int index = 0, length = 0;
			 HM_FileName.FindLastChar(*"\\", index);
			 length = HM_FileName.Len();
			 HM_FileName = HM_FileName.Mid(index + 1, length - index - 5);
			 FString path = "/Game/Maps/" + HM_FileName.Mid(0, HM_FileName.Len() - 2) + "/" + HM_FileName + "/" + HM_FileName + ".umap";
			 FString DefPath = "/Game/Maps/" + HM_FileName.Mid(0, HM_FileName.Len() - 2) + "/" + HM_FileName + "/" + HM_FileName;
			 FString TileName = DefPath;

			 FString WorldRootPath = FPackageName::LongPackageNameToFilename(GWorld->WorldComposition->GetWorldRoot());
			 FString MapFileName = TileName + FPackageName::GetMapPackageExtension();


			 UWorld* MyWorld = MyCreateNewWorld(path);
			 if (!MyWorld)
			 {
				 UE_LOG(LogTemp, Error, TEXT("Falid Creation level  for %s"), *i)
					 continue;
			 }
			 ALandscapeProxy* LandscapeProxy = CreateNewLandscapeProxy(i, 1021, MyWorld, LandscapeGuid);
			 if (LandscapeProxy)
			 {
				 LandscapeProxy->SetActorLocation(LandLocation);
				 LandscapeProxy->SetActorScale3D(LandScale);
				 LandscapeProxy->SetActorRotation(LandRotation);
				 LandscapeProxy->SetActorLabel(HM_FileName + TEXT("Lsmdscsape"));
				 FString NameToFind;
				 int32 LetterIndex, NumericIndex;
				 encoder.getIndeces(i, LetterIndex, NumericIndex);
				 int32 XCof = config.startX - NumericIndex;
				 int32 YCof = config.startY - LetterIndex;

				 SetLevelPosition(FIntVector(-XCof * config.sizeLandscape, -YCof * config.sizeLandscape, 0), MyWorld->PersistentLevel);
				 LandscapeProxy->GetSharedProperties(Landscape);
				 if (BaseMaterial)
				 {
					 ApplyMaterial(i, LandscapeProxy);
				 }
			 }
			 else
			 {
				 UE_LOG(LogTemp, Error, TEXT("Falid Creation Landscape  for %s"), *i);
			 }

			 // FEditorFileUtils::SaveMap(MyWorld, path);
			 MyWorld->PreSaveRoot(*path);
			 FEditorFileUtils::SaveLevel(MyWorld->PersistentLevel);
			 //const bool bPromptUserToSave(false);
			 //const bool bSaveMapPackages(true);
			 //const bool bSaveContentPackages(true);
			 //const bool bFastSave(false);
			 //const bool bNotifyNoPackagesSaved(false);
			 //const bool bCanBeDeclined(false);
			 //FEditorFileUtils::SaveDirtyPackages(bPromptUserToSave, bSaveMapPackages, bSaveContentPackages, bFastSave, bNotifyNoPackagesSaved, bCanBeDeclined);
			 MyWorld->DestroyWorld(false);
			 CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);

		 }


		 // LandscapeProxy->SetActorRotation(FRotator(0, 180, 0));
		 // LandscapeProxy->SetActorLocation(FVector::ZeroVector);
		 // LandscapeProxy->SetActorScale3D(FVector(100.f, 100.f, 100.f));
		 return;


	 }
 }
*/