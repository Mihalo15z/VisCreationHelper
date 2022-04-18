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
#include "Materials/MaterialInterface.h"
#include "Assets/FVCH_AssetFunctions.h"
#include "Materials/MaterialInstanceConstant.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "LandscapeProxy.h"
#include "LandscapeEdit.h"
#include "Misc/FileHelper.h"
//#include "LandscapeProxy.h"

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
	FString PathToHeightmaps = PathToImportData / ConfigObject->HeightmapsDir;
	auto HeightMaps(FVCH_PreparationDataFunctions::GetAllHeightmaps(PathToHeightmaps, ConfigObject->GetFinalResolution()));
	//FVCH_PreparationDataFunctions::RemoveCrackForHeightmaps(HeightMaps, *ConfigObject->NameMask, ConfigObject->GetFinalResolution());
	// load geo data 
	FString PathToLandConfig = PathToImportData / ConfigObject->MapsDir / ConfigObject->LandConfigName;
	auto LevelData(FVCH_PreparationDataFunctions::GeneratedImportDataTables(PathToLandConfig));
	// calculate levelSize(mb calc to GeneratedImportDataTables)
	double LevelSize = FCString::Atod(*ConfigObject->LevelSize);
	double LevelZOffset = FCString::Atod(*ConfigObject->ZOffset);
	FRotator LandRotation = FRotator::ZeroRotator;
	FVector  LandPosition = FVector(0.f, 0.f, LevelZOffset * 100.f);
	FVector LandScale(LevelSize / (ConfigObject->GetFinalResolution() - 1.0), LevelSize / (ConfigObject->GetFinalResolution() - 1.0), 100.f);

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
				FVector Offset(NumericOffset * LevelSize, LitterOffset * LevelSize, 0.f);
				auto Landscape = NewWorld->SpawnActor<ALandscape>(Offset, LandRotation);
				Landscape->SetActorScale3D(LandScale);
				Landscape->LandscapeMaterial = LandscapeBaseMaterial;
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
				
				//FVector Offset(NumericOffset * LevelSize, LitterOffset * LevelSize, 0.f);
				//if (Offset != FVector::ZeroVector)
				//{
				//	NewWorld->PersistentLevel->ApplyWorldOffset(Offset, false);

				//	for (AActor* Actor : NewWorld->PersistentLevel->Actors)
				//	{
				//		if (Actor != nullptr)
				//		{
				//			GEditor->BroadcastOnActorMoved(Actor);
				//		}
				//	}
				//}

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

	GWarn->EndSlowTask();
}

void FVCH_LandscapeFunctions::ImportLandscapeProxyToNewLevels(FString PathToImportData)
{
	GWarn->BeginSlowTask(FText::FromString(TEXT("Import LamdscapeStreamingProxy")), true);
	GWorld->PersistentLevel->ApplyWorldOffset(FVector(0.f), false);
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
	FString PathToHeightmaps = PathToImportData / ConfigObject->HeightmapsDir;
	auto HeightMaps(FVCH_PreparationDataFunctions::GetAllHeightmaps(PathToHeightmaps, ConfigObject->GetFinalResolution()));
	uint16 MinH(0), MaxH(0);

	if(!FVCH_PreparationDataFunctions::CheckHeightmaps(HeightMaps, ConfigObject->GetFinalResolution(), ConfigObject->NameMask))
		FVCH_PreparationDataFunctions::RemoveCrackForHeightmaps(HeightMaps, *ConfigObject->NameMask, ConfigObject->GetFinalResolution());
	// load geo data 
	FString PathToLandConfig = PathToImportData / ConfigObject->MapsDir / ConfigObject->LandConfigName;
	auto LevelData(FVCH_PreparationDataFunctions::GeneratedImportDataTables(PathToLandConfig));
	// calculate levelSize(mb calc to GeneratedImportDataTables
	
	const double LevelZOffset = FCString::Atod(*ConfigObject->ZOffset);
	const double GeoScale = ConfigObject->Scale.IsEmpty() ? 1.0 : FCString::Atod(*ConfigObject->Scale);
	const double LevelSize = FCString::Atod(*ConfigObject->LevelSize) * GeoScale * 100.0;
	const double DeltaHeight = FCString::Atod(*ConfigObject->DeltaHeight);
	//double HeightFactor = FCString::Atod(*ConfigObject->)
	FRotator LandRotation(FRotator::ZeroRotator);
	FVector  LandPosition(0.f, 0.f, LevelZOffset * 100.0);
	double ResolutionLand = ConfigObject->Resolution * 2.0;
	FVector LandScale(LevelSize / ResolutionLand, LevelSize / ResolutionLand, (DeltaHeight / 256.0) * 100.0);

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

	Landscape->SetActorLocation(LandPosition);
	Landscape->SetActorScale3D(LandScale);
	Landscape->SetActorRotation(LandRotation);
	// Setup layers list for importing
	TArray<FLandscapeImportLayerInfo> ImportLayers;

	Landscape->ComponentSizeQuads = 510;
	Landscape->NumSubsections = 2;
	Landscape->SubsectionSizeQuads = 255;
	Landscape->SetLandscapeGuid(LandscapeGuid);
	Landscape->CreateLandscapeInfo();

	int32 CountHMap(0);
	
	for (const auto& HeightMap : HeightMaps)
	{
		++CountHMap;
		GWarn->StatusUpdate(CountHMap, HeightMaps.Num(), FText::FromString(TEXT("Importing landscape tiles:") + HeightMap.Key));
		if (Encoder.GetIndeces(HeightMap.Key, CurrentLitterIndex, CurrentNumericIndex))
		{
			LitterOffset = CurrentLitterIndex - ZeroLitterIndex;
			NumericOffset = CurrentNumericIndex - ZeroNumericIndex;


			FString MapFileName = WorldRootPath + HeightMap.Key + FPackageName::GetMapPackageExtension();

			UE_LOG(VCH_LandscapeLog, Warning, TEXT(" map  name %s  offset L =%i, N = %i"), *HeightMap.Key, LitterOffset, NumericOffset);
			// Create a new world - so we can 'borrow' its level
			UWorld* NewWorld = UWorld::CreateWorld(EWorldType::None, false);
			check(NewWorld);

			bool bSaved = FEditorFileUtils::SaveLevel(NewWorld->PersistentLevel, *MapFileName);
			if (bSaved)
			{
				ALandscapeProxy* LandscapeProxy = NewWorld->SpawnActor<ALandscapeStreamingProxy>();
				LandscapeProxy->LandscapeMaterial = LandscapeBaseMaterial;
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
				FVector Offset(NumericOffset * LevelSize, LitterOffset * LevelSize, 0.f);
				LandscapeProxy->SetActorLocation(LandPosition + Offset);
				LandscapeProxy->SetActorScale3D(LandScale);
				LandscapeProxy->SetActorRotation(LandRotation);
				LandscapeProxy->SetActorLabel(HeightMap.Key + TEXT("LandscapeStreming"));

/*				FVector Offset(NumericOffset * LevelSize, LitterOffset * LevelSize, 0.f);
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
				}*/	

				if (LandscapeProxy)
				{
					FVector	DrawScale = LandscapeProxy->GetRootComponent()->GetRelativeScale3D();
					// to do : calc right offset
					FIntPoint QuadsSpaceOffset;
					QuadsSpaceOffset.X = FMath::RoundToInt(-Offset.X / DrawScale.X);
					QuadsSpaceOffset.Y = FMath::RoundToInt(-Offset.Y / DrawScale.Y);
					LandscapeProxy->SetAbsoluteSectionBase(QuadsSpaceOffset + LandscapeProxy->LandscapeSectionOffset /*+ QuadsSpaceOffset2*/);
					UE_LOG(VCH_LandscapeLog, Warning, TEXT("%s  section offset %s"), *(QuadsSpaceOffset + LandscapeProxy->LandscapeSectionOffset).ToString(), *LandscapeProxy->LandscapeSectionOffset.ToString());
					// UpdateLandscapeSectionsOffset(FIntPoint(Offset.X, Offset.Y), land); // section offset is 2D 
					bool bShowWarnings = false;
					ULandscapeInfo::RecreateLandscapeInfo(GWorld, bShowWarnings);
				}
				FString PathToLandscapeData(TEXT("/Game/") + ConfigObject->PathToLandscapeMatAndTextures);
				FString MatInstName = HeightMap.Key + TEXT("_Inst");
				FString PathToMInst = PathToLandscapeData + SlashStr + MatInstName /*+ TEXT(".") + MatInstName*/;
				// change paths  to /Game/ConfigObject->PathToLandscapeMatAndTextures/MapName
				auto MatInst = FVCH_AssetFunctions::CreateMaterialInstance(PathToMInst, Cast<UMaterialInterface>(LandscapeBaseMaterial));
				auto Texture = FVCH_AssetFunctions::ImportTextureForLandscape(HeightMap.Key, PathToImportData + SlashStr + ConfigObject->TexturesDir /*+ SlashStr + HeightMap.Key + TEXT(".png")*/, PathToLandscapeData);
				if (MatInst && Texture)
				{

					MatInst->SetTextureParameterValueEditorOnly(TEXT("BaseTexture"), Texture);
					MatInst->SetScalarParameterValueEditorOnly(TEXT("LitterOffset"), -LitterOffset);
					MatInst->SetScalarParameterValueEditorOnly(TEXT("NumericOffset"), -NumericOffset ); //- 1???
					MatInst->PostEditChange();
					MatInst->MarkPackageDirty();
					LandscapeProxy->LandscapeMaterial = MatInst;
					FPropertyChangedEvent PropertyChangedEvent(FindFieldChecked<FProperty>(LandscapeProxy->GetClass(), FName("LandscapeMaterial")));
					LandscapeProxy->PostEditChangeProperty(PropertyChangedEvent);
					LandscapeProxy->PostEditChange();
					LandscapeProxy->MarkPackageDirty();
				}
				FEditorFileUtils::SaveLevel(NewWorld->PersistentLevel, *MapFileName);
			}
			NewWorld->DestroyWorld(false);
			CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
		}
	}

	GWarn->EndSlowTask();
}

void FVCH_LandscapeFunctions::BackupHeightmaps(FString PathToSave)
{
	auto ClearLandscapeLabelLambda = [](const FString& InName)
	{
		auto Result = InName.Replace(TEXT("LandscapeStreming"), TEXT(""));
		Result.ReplaceInline(TEXT("Landscape"), TEXT(""));
		return Result;
	};

	for (TActorIterator<ALandscapeProxy> Iter(GWorld, ALandscapeProxy::StaticClass()); Iter; ++Iter)
	{
		ALandscapeProxy* Landscape = *Iter;
		if (Landscape)
		{
			ULandscapeInfo* Info = nullptr;
			auto SectionOffset = Landscape->LandscapeSectionOffset;
			int32 LandscapeSectionSize = Landscape->NumSubsections  *  Landscape->SubsectionSizeQuads;
			Info = Landscape->GetLandscapeInfo();
			FString Name = ClearLandscapeLabelLambda(Landscape->GetActorLabel());

			FLandscapeEditDataInterface  EditorLandscapeInterface(Info);
			int32 MinX(MAX_int32);
			int32 MinY(MAX_int32);
			int32 MaxX(-MAX_int32);
			int32 MaxY(-MAX_int32);
			int32 Resolution = 0;

			if (!(Info->GetLandscapeExtent(MinX, MinY, MaxX, MaxY)))
			{
				continue;
			}

			TArray<uint16> HeightData;
			int32 SizeArr = (LandscapeSectionSize + 1) * (LandscapeSectionSize + 1);
			HeightData.AddZeroed(SizeArr);

			EditorLandscapeInterface.GetHeightDataFast(SectionOffset.X, SectionOffset.Y, SectionOffset.X + LandscapeSectionSize, SectionOffset.Y +LandscapeSectionSize, HeightData.GetData(), 0);
			TArray<uint8> SaveData;
			
			SaveData.AddZeroed(SizeArr * 2);
			FMemory::Memmove(SaveData.GetData(), HeightData.GetData(), SaveData.Num());

			// maybe convert name by maskFormat???
			FFileHelper::SaveArrayToFile(SaveData, *(PathToSave / Name + TEXT(".raw")));
		}
	}
}

// load level => find landscapeStreamingProxy => PROCESS(InFunction) => Save
void IteratedLevelsForLandscape(const TFunction< void(const FAssetData& AssetData, ALandscapeStreamingProxy* Landscape)>& InFunction, bool bSave = true)
{
	auto ConfigObject = GetDefault<UVCH_Settings>();
	check(ConfigObject);

	FString Path = TEXT("/Game/") + ConfigObject->MapsDir;
	auto Assets = FVCH_AssetFunctions::GetAssetsByPath(Path);
	for (const auto& LevelData : Assets)
	{
		if (LevelData.GetClass() == (UWorld::StaticClass()))
		{
			CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
			FEditorFileUtils::LoadMap((LevelData.ObjectPath.ToString()));

			check(GWorld);
			for (TActorIterator<ALandscapeStreamingProxy> Iter(GWorld); Iter; ++Iter)
			{
				auto* Landscape = *Iter;
				if (Landscape)
				{
					InFunction(LevelData, Landscape);
					if (bSave)
					{
						Landscape->MarkPackageDirty();
						Landscape->PostEditChange();
						FEditorFileUtils::SaveCurrentLevel();
					}
					break;
				}
			}
		}
	}
}

void FVCH_LandscapeFunctions::UpdateLandscapeName()
{
	FString PrefixName = TEXT("LSP_");
	auto MakeNewNameForLandscape = [&PrefixName](const FAssetData& AssetData, ALandscapeStreamingProxy* Landscape)
	{
		Landscape->SetActorLabel(PrefixName + AssetData.AssetName.ToString());
	};

	IteratedLevelsForLandscape(MakeNewNameForLandscape, true);
}

void FVCH_LandscapeFunctions::SetLandscapeMaterial()
{
	auto ConfigObject = GetDefault<UVCH_Settings>();
	check(ConfigObject);

	FNameEncoder Encoder(ConfigObject->NameMask);

	auto MaterialSoftPath = ConfigObject->LandscapeBaseMaterial;
	FString PathToMaterialsInst = TEXT("/Game/") + ConfigObject->PathToLandscapeMatAndTextures + TEXT("/Materials");

	auto SetLandMaterial = [&Encoder, &MaterialSoftPath, &PathToMaterialsInst](const FAssetData& AssetData, ALandscapeStreamingProxy* Landscape)
	{
		FString LevelDataName = Encoder.FromXYName(AssetData.AssetName.ToString());
		int32 LetterIndex, NumericIndex;
		Encoder.GetIndeces(LevelDataName, LetterIndex, NumericIndex);
		FString MatInstName = LevelDataName + TEXT("_Inst");
		FString PathToMInst = PathToMaterialsInst / MatInstName;
		// change paths  to /Game/ConfigObject->PathToLandscapeMatAndTextures/MapName
		auto MatInst = FVCH_AssetFunctions::CreateMaterialInstance(PathToMInst, Cast<UMaterialInterface>(MaterialSoftPath.LoadSynchronous()));
		if (MatInst)
		{
			MatInst->SetScalarParameterValueEditorOnly(TEXT("LitterOffset"), -LetterIndex);
			MatInst->SetScalarParameterValueEditorOnly(TEXT("NumericOffset"), -NumericIndex);
			MatInst->PostEditChange();
			MatInst->MarkPackageDirty();
			FEditorFileUtils::PromptForCheckoutAndSave({ MatInst->GetPackage() }, true, false);
			Landscape->LandscapeMaterial = MatInst;
			FPropertyChangedEvent PropertyChangedEvent(FindFieldChecked<FProperty>(Landscape->GetClass(), FName("LandscapeMaterial")));
			Landscape->PostEditChangeProperty(PropertyChangedEvent);
		}
	};

	IteratedLevelsForLandscape(SetLandMaterial, true);
}

void FVCH_LandscapeFunctions::RenameOpenLandscapes()
{
	check(GWorld);

	FString PrefixName = TEXT("LSP_");

	for (TActorIterator<ALandscapeStreamingProxy> Iter(GWorld); Iter; ++Iter)
	{
		auto* Landscape = *Iter;
		if (Landscape)
		{
			auto LevelPacage = Landscape->GetLevel()->GetPackage();
			
			FString LevelName = LevelPacage->GetName();
			//FString PathName = LevelPacage->GetPathName();
			FString AssetName = FPaths::GetCleanFilename(LevelName);
			Landscape->SetActorLabel(PrefixName + AssetName);
		}
	}
}

void FVCH_LandscapeFunctions::ApplyMaterialsForOpenLandscapes()
{
	check(GWorld);

	auto ConfigObject = GetDefault<UVCH_Settings>();
	check(ConfigObject);

	FNameEncoder Encoder(ConfigObject->NameMask);

	auto MaterialSoftPath = ConfigObject->LandscapeBaseMaterial;
	FString PathToMaterialsInst = TEXT("/Game/") + ConfigObject->PathToLandscapeMatAndTextures + TEXT("/Materials");

	for (TActorIterator<ALandscapeStreamingProxy> Iter(GWorld); Iter; ++Iter)
	{
		auto* Landscape = *Iter;
		if (Landscape)
		{
			//auto LevelPacage = Landscape->GetLevel()->GetPackage();
			//FString LevelName = LevelPacage->GetName();
			FString LevelName = Landscape->GetActorLabel();
			FString LevelDataName = Encoder.FromXYName(LevelName);
			int32 LetterIndex, NumericIndex;
			Encoder.GetIndeces(LevelDataName, LetterIndex, NumericIndex);
			FString MatInstName = LevelDataName + TEXT("_Inst");
			FString PathToMInst = PathToMaterialsInst / MatInstName;
			// change paths  to /Game/ConfigObject->PathToLandscapeMatAndTextures/MapName
			auto MatInst = FVCH_AssetFunctions::CreateMaterialInstance(PathToMInst, Cast<UMaterialInterface>(MaterialSoftPath.LoadSynchronous()));
			if (MatInst)
			{
				MatInst->SetScalarParameterValueEditorOnly(TEXT("LitterOffset"), -LetterIndex);
				MatInst->SetScalarParameterValueEditorOnly(TEXT("NumericOffset"), -NumericIndex);
				MatInst->PostEditChange();
				MatInst->MarkPackageDirty();
				FEditorFileUtils::PromptForCheckoutAndSave({ MatInst->GetPackage() }, true, false);
				Landscape->LandscapeMaterial = MatInst;
				FPropertyChangedEvent PropertyChangedEvent(FindFieldChecked<FProperty>(Landscape->GetClass(), FName("LandscapeMaterial")));
				Landscape->PostEditChangeProperty(PropertyChangedEvent);
			}
			//InFunction(LevelData, Landscape);
			Landscape->MarkPackageDirty();
			if (bool bSave = true)
			{
				Landscape->MarkPackageDirty();
				Landscape->PostEditChange();
				FEditorFileUtils::SaveLevel(Landscape->GetLevel());
			}
		}
	}
}

void FVCH_LandscapeFunctions::GenerateLandscapeXML()
{
	auto ConfigObject = GetDefault<UVCH_Settings>();
	check(ConfigObject);

	FNameEncoder Encoder(ConfigObject->NameMask);

	FString Result;
	Result.Append(TEXT("<?xml version=\"1.0\" encoding = \"Windows-1252\"?>\n\t<Data>\n\t<Levels>\n"));

	for (TActorIterator<ALandscapeProxy> LandIter(GWorld, ALandscapeProxy::StaticClass()); LandIter; ++LandIter)
	{
		ALandscapeProxy* landscape = *LandIter;
		if (landscape)
		{
			// to do: check land name 
			auto LandName = Encoder.FromXYName(landscape->GetActorLabel());
			Result.Append(TEXT("\t\t<info name=\"") + LandName + TEXT("\" "));
			FVector origen;
			FVector bound;
			landscape->GetActorBounds(true, origen, bound);
			Result.Append(TEXT("\torigin=\"") + origen.ToString());
			Result.Append(TEXT("\"\tbound=\"") + bound.ToString());
			Result.Append(TEXT("\"\tlocation=\"") + landscape->GetActorLocation().ToString());
			Result.Append(TEXT("\" />\n"));
		}
	}
	Result.Append("\t</Levels>\n</Data>");
	FString Path = FPaths::ProjectConfigDir() + TEXT("InfoLevels.xml");
	FFileHelper::SaveStringToFile(Result, *Path);

}

