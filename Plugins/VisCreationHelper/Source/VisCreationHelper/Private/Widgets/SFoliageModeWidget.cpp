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
		.AutoHeight()
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("Generate Forest")))
			.OnClicked(this, &SFoliageModeWidget::OnPlaceForestClick)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("Clear All IFAs")))
			.OnClicked(this, &SFoliageModeWidget::OnClearAllIFAClick)
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION


FReply SFoliageModeWidget::OnPlaceForestClick()
{
	auto SettingsObject = GetDefault<UVCH_Settings>();
	check(SettingsObject);
	FString ImportPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + SettingsObject->GlobalImportContentDir;
	FString PathToforestMask = ImportPath / SettingsObject->FoliageMasksDir;
	FString PathToHeightmaps = ImportPath / SettingsObject->HeightmapsDir;
	FString PathToWaterMask = ImportPath / SettingsObject->WaterMasksDir;
	FVCH_FoliageFunctions::GenerateForest(PathToHeightmaps, PathToforestMask, PathToWaterMask);
	return FReply::Handled();
}
FReply SFoliageModeWidget::OnClearAllIFAClick()
{
	FVCH_FoliageFunctions::ClearAllIFA();
	return FReply::Handled();
}

