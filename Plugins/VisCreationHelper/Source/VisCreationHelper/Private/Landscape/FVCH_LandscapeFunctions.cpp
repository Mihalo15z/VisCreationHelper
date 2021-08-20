// Fill out your copyright notice in the Description page of Project Settings.


#include "Landscape/FVCH_LandscapeFunctions.h"
#include "CoreGlobals.h"
#include "Misc/FeedbackContext.h"

FVCH_LandscapeFunctions::FVCH_LandscapeFunctions()
{
}

FVCH_LandscapeFunctions::~FVCH_LandscapeFunctions()
{
}

void FVCH_LandscapeFunctions::ImpotrLandscapesToNewLevels(FString PathToImportData)
{
	GWarn->BeginSlowTask(FText::FromString(TEXT("Import Lamdscapes")), true);
	// make path data(to import content and from)
	
	// load Heightmaps (and Remover crack? mb this is different function (work oly *.raw files(open - edit - save);
	//Test HeightMaps Size;

	// load base landscape material

		// BEGINN LOOP For HeightMaps
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

	//GWarn->StatusUpdate(TileIndex, ImportSettings.HeightmapFileList.Num(), FText::Format(LOCTEXT("ImportingLandscapeTiles", "Importing landscape tiles: {0}"), FText::FromString(TileName)));

	GWarn->EndSlowTask();
}

void FVCH_LandscapeFunctions::BackupHeightmaps(FString PathToSave)
{
}

/*
UEditorLoadingAndSavingUtils
UEditorLevelLibrary
FWorldTileCollectionModel
LevelsBBox.ShiftBy(FVector(InDelta, 0))



	if (ImportDialog->ShouldImport() && ImportDialog->GetImportSettings().HeightmapFileList.Num())
	{
		const FTiledLandscapeImportSettings& ImportSettings = ImportDialog->GetImportSettings();

		// Default path for imported landscape tiles
		// Use tile prefix as a folder name under world root
		FString WorldRootPath = FPackageName::LongPackageNameToFilename(GetWorld()->WorldComposition->GetWorldRoot());
		// Extract tile prefix
		FString FolderName = FPaths::GetBaseFilename(ImportSettings.HeightmapFileList[0]);
		int32 PrefixEnd = FolderName.Find(TEXT("_x"), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		FolderName.LeftInline(PrefixEnd, false);
		WorldRootPath+= FolderName;
		WorldRootPath+= TEXT("/");

		GWarn->BeginSlowTask(LOCTEXT("ImportingLandscapeTilesBegin", "Importing landscape tiles"), true);

		// Create main landscape actor in persistent level, it will be empty (no components in it)
		// All landscape tiles will go into it's own sub-levels
		FGuid LandscapeGuid = FGuid::NewGuid();
		{
			ALandscape* Landscape = GetWorld()->SpawnActor<ALandscape>();
			Landscape->SetActorTransform(FTransform(FQuat::Identity, FVector::ZeroVector, ImportSettings.Scale3D));

			// Setup layers list for importing
			TArray<FLandscapeImportLayerInfo> ImportLayers;
			SetupLandscapeImportLayers(ImportSettings, GetWorld()->GetOutermost()->GetName(), INDEX_NONE, ImportLayers);

			// Set landscape configuration
			Landscape->bCanHaveLayersContent = false;
			Landscape->LandscapeMaterial	= ImportSettings.LandscapeMaterial.Get();
			Landscape->ComponentSizeQuads	= ImportSettings.QuadsPerSection*ImportSettings.SectionsPerComponent;
			Landscape->NumSubsections		= ImportSettings.SectionsPerComponent;
			Landscape->SubsectionSizeQuads	= ImportSettings.QuadsPerSection;
			Landscape->SetLandscapeGuid(LandscapeGuid);
			for (const auto& ImportLayerInfo : ImportLayers)
			{
				Landscape->EditorLayerSettings.Add(FLandscapeEditorLayerSettings(ImportLayerInfo.LayerInfo));
			}
			Landscape->CreateLandscapeInfo();
		}

		// Import tiles
		int32 TileIndex = 0;
		for (const FString& Filename : ImportSettings.HeightmapFileList)
		{
			check(LandscapeGuid.IsValid());

			FString TileName = FPaths::GetBaseFilename(Filename);
			FVector TileScale = ImportSettings.Scale3D;

			GWarn->StatusUpdate(TileIndex, ImportSettings.HeightmapFileList.Num(), FText::Format(LOCTEXT("ImportingLandscapeTiles", "Importing landscape tiles: {0}"), FText::FromString(TileName)));

			FWorldTileModel::FLandscapeImportSettings TileImportSettings = {};
			TileImportSettings.LandscapeGuid		= LandscapeGuid;
			TileImportSettings.LandscapeMaterial	= ImportSettings.LandscapeMaterial.Get();
			TileImportSettings.ComponentSizeQuads	= ImportSettings.QuadsPerSection*ImportSettings.SectionsPerComponent;
			TileImportSettings.QuadsPerSection		= ImportSettings.QuadsPerSection;
			TileImportSettings.SectionsPerComponent = ImportSettings.SectionsPerComponent;
			TileImportSettings.SizeX				= ImportSettings.SizeX;
			TileImportSettings.SizeY				= ImportSettings.SizeX;
			TileImportSettings.HeightmapFilename	= Filename;
			TileImportSettings.LandscapeTransform.SetScale3D(TileScale);

			// Setup layers list for importing
			SetupLandscapeImportLayers(ImportSettings, GetWorld()->GetOutermost()->GetName(), TileIndex, TileImportSettings.ImportLayers);
			TileImportSettings.ImportLayerType = ELandscapeImportAlphamapType::Additive;

			if (ReadHeightmapFile(TileImportSettings.HeightData, Filename, TileImportSettings.SizeX, TileImportSettings.SizeY))
			{
				FString MapFileName = WorldRootPath + TileName + FPackageName::GetMapPackageExtension();
				// Create a new world - so we can 'borrow' its level
				UWorld* NewWorld = UWorld::CreateWorld(EWorldType::None, false);
				check(NewWorld);

				bool bSaved = FEditorFileUtils::SaveLevel(NewWorld->PersistentLevel, *MapFileName);
				if (bSaved)
				{
					// update levels list so we can find a new level in our world model
					PopulateLevelsList();
					TSharedPtr<FWorldTileModel> NewTileModel = StaticCastSharedPtr<FWorldTileModel>(FindLevelModel(NewWorld->GetOutermost()->GetFName()));
					// Hide level, so we do not depend on a current world origin
					NewTileModel->SetVisible(false);

					// Create landscape proxy in a new level
					ALandscapeProxy* NewLandscape = NewTileModel->ImportLandscapeTile(TileImportSettings);

					if (NewLandscape)
					{
						// Set bounds of a tile
						NewTileModel->TileDetails->Bounds = NewLandscape->GetComponentsBoundingBox();

						// Calculate this tile offset from world origin
						FIntRect NewLandscapeRect = NewLandscape->GetBoundingRect();
						float WidthX = NewLandscapeRect.Width()*TileScale.X;
						float WidthY = NewLandscapeRect.Height()*TileScale.Y;
						FIntPoint TileCoordinates = ImportSettings.TileCoordinates[TileIndex] + ImportSettings.TilesCoordinatesOffset;
						FIntPoint TileOffset = FIntPoint(TileCoordinates.X*WidthX, TileCoordinates.Y*WidthY);
						if (ImportSettings.bFlipYAxis)
						{
							TileOffset.Y = -(TileOffset.Y + WidthY);
						}

						// Place level tile at correct position in the world
						NewTileModel->SetLevelPosition(FIntVector(TileOffset.X, TileOffset.Y, 0));

						// Save level with a landscape
						FEditorFileUtils::SaveLevel(NewWorld->PersistentLevel, *MapFileName);
					}

					// Destroy the new world we created and collect the garbage
					NewWorld->DestroyWorld(false);
					CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
				}
			}

			TileIndex++;
		}

		GWarn->EndSlowTask();
*/