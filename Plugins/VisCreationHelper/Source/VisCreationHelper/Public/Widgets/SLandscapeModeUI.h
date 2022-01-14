// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class VISCREATIONHELPER_API SLandscapeModeUI : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLandscapeModeUI)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	FReply OnImportLandscapePrxyClick();
	FReply OnImportLandscapeClick();
	FReply OnBackupHeightmapsClick();
	FReply OnRenameLandscapesClick();
	FReply OnCreateLandscapeMaterialsClick();

};
