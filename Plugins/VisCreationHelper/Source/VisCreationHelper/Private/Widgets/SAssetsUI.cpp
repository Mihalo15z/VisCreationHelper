// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SAssetsUI.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SAssetsUI::Construct(const FArguments& InArgs)
{
	/*
	ChildSlot
	[
		// Populate the widget
	];
	*/
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply SAssetsUI::SetTexturesForLandscapeMaterials()
{
	return FReply::Handled();
}
