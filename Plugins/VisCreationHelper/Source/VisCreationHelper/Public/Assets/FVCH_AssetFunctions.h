// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */

class UMaterialInstanceConstant;
class UMaterialInterface;
class UTexture;

class VISCREATIONHELPER_API FVCH_AssetFunctions
{
public:
	FVCH_AssetFunctions();
	~FVCH_AssetFunctions();


	static UTexture* ImportTextureForLandscape(const FString& Name, const FString& Path, const FString& SavePath);
	static void ImportTextures(const FString& Path, const FString& SavePath, const FString& SuffixStr = TEXT(""), int32 NumSaveTextures = 10);
	static void ApplyTextureParams(UTexture* Texture);
	static UMaterialInstanceConstant* CreateMaterialInstance(const FString& PathToImport, UMaterialInterface* ParentMaterial);
	static TArray<FAssetData> GetAssetsByPath(const FString& InPath);
	static void SetTexturesForLandMaterials(const FString& PathToMats, const FString& ParametrName, const FString& SuffixStr, const FString& ImportPath);
};
