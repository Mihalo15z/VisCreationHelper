// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SFoliageModeWidget.h"
#include "SlateOptMacros.h"
#include "Settings/VCH_Settings.h"
#include "Foliage/FVCH_FoliageFunctions.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SFoliageModeWidget::Construct(const FArguments& InArgs)
{
	
	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Left)
		.VAlign(EVerticalAlignment::VAlign_Top)
		.AutoHeight()
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("Generate Forest")))
			.OnClicked(this, &SFoliageModeWidget::OnPlaceForestClick)
		]
		+ SVerticalBox::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Left)
		.VAlign(EVerticalAlignment::VAlign_Top)
		.AutoHeight()
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("Clear All IFAs")))
			.OnClicked(this, &SFoliageModeWidget::OnClearAllIFAClick)
		]
		// Populate the widget
	];
	
}
FReply SFoliageModeWidget::OnPlaceForestClick()
{
	auto SettingsObject = GetDefault<UVCH_Settings>();
	FString PathToforestMask = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + TEXT("_ImportData/Forest");
	FString PathToHeightmaps = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + TEXT("_ImportData/") + SettingsObject->HeightmapsDir;
	FString PathToWaterMask = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + TEXT("_ImportData/") + SettingsObject->WaterMasksDir;
	FVCH_FoliageFunctions::GenerateForest(PathToHeightmaps, PathToforestMask, PathToWaterMask);
	return FReply::Handled();
}
FReply SFoliageModeWidget::OnClearAllIFAClick()
{
	FVCH_FoliageFunctions::ClearAllIFA();
	return FReply::Handled();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
