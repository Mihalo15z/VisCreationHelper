// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class VISCREATIONHELPER_API SPreparationDataModeUI : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPreparationDataModeUI)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
private:

	FReply OnGenerateLevelsXMLClick();
	FReply OnGetMinMaxForHeightmapsClick();
	FReply OnCorrectHMapsRangeClick();
	FReply OnRemoveCrackForHeightmapsClick();
	FReply OnCheckHeightmapsClick();
	FReply OnMakeHMapForTileImportClick();
	FReply OnCalculateGeneratedParamsClick();

	FReply OnConverPngTo8RawClick();
protected:
	int32 CorrectHMapValue;

};
