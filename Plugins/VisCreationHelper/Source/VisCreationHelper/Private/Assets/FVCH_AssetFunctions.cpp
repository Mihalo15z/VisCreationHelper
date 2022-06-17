// Fill out your copyright notice in the Description page of Project Settings.


#include "Assets/FVCH_AssetFunctions.h"
#include "AssetToolsModule.h"
#include "AssetRegistryModule.h"
#include "IAssetTools.h"
#include "Engine/Texture.h"
#include "HAL/FileManager.h"
#include "FileHelpers.h"
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

UTexture* FVCH_AssetFunctions::ImportTextureForLandscape(const FString& Name, const FString& Path, const FString& SavePath)
{
	UTexture* ImportedTexture = nullptr;
	FSoftObjectPath SoftPathToTexture(SavePath + TEXT("/") + Name + TEXT(".") + Name);
	ImportedTexture = Cast<UTexture>(SoftPathToTexture.TryLoad());
	if (ImportedTexture == nullptr)																						
	{
		FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FString> TextureFileNames;
		
		TextureFileNames.Add(Path / Name + TEXT(".png"));
		TArray<UObject*> ImportedAssets = AssetToolsModule.Get().ImportAssets(TextureFileNames, SavePath);
		if (ImportedAssets.Num() == 0)
		{
			UE_LOG(VCH_AssetsLog, Warning, TEXT("Bad Import Texture %s"), *Path);
			return nullptr;
		}
		ImportedTexture = (ImportedAssets.Num() > 0) ? Cast<UTexture>(ImportedAssets[0]) : nullptr;
		ApplyTextureParams(ImportedTexture);
	}
	return ImportedTexture;
}

void FVCH_AssetFunctions::ImportTextures(const FString& Path, const FString& SavePath,  const FString& SuffixStr, int32 NumSaveTextures)
{
	//TArray<FString>  TextureFiles;
	//TArray<

	//if (IFileManager::Get().DirectoryExists(*Path))
	//{
	//	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
	//	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	//	FString Filter = TEXT(".png");
	//	FString Mask = Path + TEXT("/*") + Filter;
	//	IFileManager::Get().FindFiles(TextureFiles, *Mask, true, false);

	//	TArray<FString> TextureFileNames;

	//	for (const auto& FileName : TextureFiles)
	//	{
	//		UTexture* ImportedTexture = nullptr;
	//		FString TextureAssetName = FileName.Replace(TEXT(".pmg"), TEXT("")) + SuffixStr;
	//		FSoftObjectPath SoftPathToTexture(SavePath + TEXT("/") + TextureAssetName + TEXT(".") + TextureAssetName);
	//		ImportedTexture = Cast<UTexture>(SoftPathToTexture.TryLoad());
	//		// if not found this texture, create new Texture
	//		if (ImportedTexture == nullptr)
	//		{
	//			TextureFileNames.Add(Path / FileName + TEXT(".png"));
	//		}


	//	}
	//	
	//	TArray<UObject*> ImportedAssets = AssetToolsModule.Get().ImportAssets(TextureFileNames, SavePath);
	//	if (ImportedAssets.Num() == 0)
	//	{
	//		UE_LOG(VCH_AssetsLog, Warning, TEXT("Bad Import Texture %s"), *Path);

	//	}
	//	ImportedTexture = (ImportedAssets.Num() > 0) ? Cast<UTexture>(ImportedAssets[0]) : nullptr;
	//	ApplyTextureParams(ImportedTexture);
	//	FEditorFileUtils::PromptForCheckoutAndSave({ ImportedTexture->GetPackage() }, false, false);
	//}

	
}

void FVCH_AssetFunctions::ApplyTextureParams(UTexture * Texture)
{
	auto Texture2d = Cast<UTexture2D>(Texture);
	if (Texture2d)
	{
		//Texture2d->bNoTiling = true;
		//Texture2d->
		// to do :  add texture settings
	}
}

UMaterialInstanceConstant* FVCH_AssetFunctions::CreateMaterialInstance(const FString& PathToImport, UMaterialInterface* ParentMaterial)
{
	FSoftObjectPath MatInstPath(PathToImport);
	auto MatInst = Cast<UMaterialInstanceConstant>(MatInstPath.TryLoad());
	if (MatInst)
	{
		return MatInst;
	}
	else
	{
		UPackage* MaterialOuter(nullptr);
		{
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

TArray<FAssetData> FVCH_AssetFunctions::GetAssetsByPath(const FString & InPath)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> Assets;
	AssetRegistry.GetAssetsByPath(FName(*InPath), Assets, true);
	return MoveTemp(Assets);
}

void FVCH_AssetFunctions::SetTexturesForLandMaterials(const FString& PathToMats, const FString& ParametrName, const FString& ImportPath)
{
	if (IFileManager::Get().DirectoryExists(*ImportPath))
	{
		TArray<FString > Files;
		FString Filter = TEXT(".png");
		FString Mask = ImportPath + TEXT("/*") + Filter;
		IFileManager::Get().FindFiles(Files, *Mask, true, false);

		FString PathToTextures = PathToMats + TEXT("/Textures");
		for(auto TextureName :Files)
		{
			TextureName.ReplaceInline(*Filter, TEXT(""));
			auto MaterialName = TextureName + TEXT("_Inst");
			FSoftObjectPath MatInstPath(PathToMats / TEXT("Materials") / MaterialName + TEXT(".") + MaterialName);
			auto MatInst = Cast<UMaterialInstanceConstant>(MatInstPath.TryLoad());

			if (MatInst)
			{
				UTexture* Texture = ImportTextureForLandscape(TextureName, ImportPath, PathToTextures / ParametrName);
				if (Texture)
				{
					MatInst->SetTextureParameterValueEditorOnly(FName(ParametrName), Texture);
					MatInst->PostEditChange();
					MatInst->MarkPackageDirty();
					FEditorFileUtils::PromptForCheckoutAndSave({ MatInst->GetPackage(), Texture->GetPackage() }, true, false);
					
					Texture->BeginDestroy();
					CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
				}
				MatInst->BeginDestroy();
			}
		}
	}
}
