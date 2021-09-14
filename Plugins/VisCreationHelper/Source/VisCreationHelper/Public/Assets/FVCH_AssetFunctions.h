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


	static UTexture* ImportTextureForLandscape(FString Name, FString Path, FString SavePath);
	static UMaterialInstanceConstant* CreateMaterialInstance(FString PathToImport, UMaterialInterface* ParentMaterial);
};
