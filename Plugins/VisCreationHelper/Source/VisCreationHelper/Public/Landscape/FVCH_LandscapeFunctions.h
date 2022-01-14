// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class VISCREATIONHELPER_API FVCH_LandscapeFunctions
{
public:
	FVCH_LandscapeFunctions();
	~FVCH_LandscapeFunctions();

	static void ImpotrLandscapesToNewLevels( FString PathToImportData);
	static void ImportLandscapeProxyToNewLevels(FString PathToImportData);
	static void BackupHeightmaps(FString PathToSave);
	static void UpdateLandscapeName();
	static void SetLandscapeMaterial();
};
