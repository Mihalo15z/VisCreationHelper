// Fill out your copyright notice in the Description page of Project Settings.


#include "Assets/FVCH_AssetFunctions.h"
#include "AssetToolsModule.h"
#include "AssetRegistryModule.h"
#include "IAssetTools.h"
#include "Engine/Texture.h"
#include "HAL/FileManager.h"
#include "Modules/ModuleManager.h"
#include "Engine/Texture2D.h"
#include "MaterialUtilities.h"
#include "Materials/MaterialInstanceConstant.h"

DECLARE_LOG_CATEGORY_CLASS(VCH_AssetsLog, Log, All);

FVCH_AssetFunctions::FVCH_AssetFunctions()
{
}

FVCH_AssetFunctions::~FVCH_AssetFunctions()
{
}

UTexture* FVCH_AssetFunctions::ImportTextureForLandscape(FString Name, FString Path, FString SavePath)
{
	UTexture* ImportedTexture = nullptr;
	FSoftObjectPath SoftPathToTexture(SavePath + TEXT("/") + Name + TEXT(".") + Name);
	ImportedTexture =Cast<UTexture>(SoftPathToTexture.TryLoad());
	if (ImportedTexture == nullptr)																						// если такой текстуры не существует, создаем новую
	{
		FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FString> TextureFileNames;
		TextureFileNames.Add(Path);
		TArray<UObject*> ImportedAssets = AssetToolsModule.Get().ImportAssets(TextureFileNames, SavePath);
		if (ImportedAssets.Num() == 0)
		{
			UE_LOG(VCH_AssetsLog, Warning, TEXT("Bad Import Texture %s"), *Path);
			return nullptr;
		}
		ImportedTexture = (ImportedAssets.Num() > 0) ? Cast<UTexture>(ImportedAssets[0]) : nullptr;
		auto Texture2d = Cast<UTexture2D>(ImportedTexture);
		//if (Texture2d)
		//{
		//	// to do :  add texture settings
		//}
	}
	return ImportedTexture;
}

UMaterialInstanceConstant* FVCH_AssetFunctions::CreateMaterialInstance(FString PathToImport, UMaterialInterface* ParentMaterial)
{
	FSoftObjectPath MatInstPath(PathToImport);
	auto MatInst = Cast<UMaterialInstanceConstant>(MatInstPath.TryLoad());
	if (MatInst)
	{
		return MatInst;
	}
	else
	{
		//MatInst = FMaterialUtilities::CreateInstancedMaterial(ParentMaterial, nullptr, PathToImport, RF_Public | RF_Standalone);

		UPackage* MaterialOuter(nullptr);
		{
			//MaterialOuter = CreatePackage(NULL, *(PathToImport));
			MaterialOuter = CreatePackage(*(PathToImport));
			MaterialOuter->FullyLoad();
			MaterialOuter->Modify();
		}
		const FString AssetBaseName = FPackageName::GetShortName(PathToImport);
		MatInst = NewObject<UMaterialInstanceConstant>(MaterialOuter, FName(*AssetBaseName), RF_Public | RF_Standalone);
		checkf(MatInst, TEXT("Failed to create instanced material"));
		MatInst->Parent = ParentMaterial;
	}
	return MatInst;
}
