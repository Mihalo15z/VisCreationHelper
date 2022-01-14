// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SLandscapeModeUI.h"
#include "SlateOptMacros.h"
#include "Settings/VCH_Settings.h"
#include <Landscape/FVCH_LandscapeFunctions.h>


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLandscapeModeUI::Construct(const FArguments& InArgs)
{
	auto MakeButton = [&](auto FunClick, const FString& InLable)
	{
		return SNew(SButton)
			.OnClicked(this, FunClick)
			.Text(FText::FromString(InLable));
	};
	
	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot().AutoHeight()
		[
			MakeButton(&SLandscapeModeUI::OnImportLandscapePrxyClick, TEXT("Import Landscape Prxy"))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakeButton(&SLandscapeModeUI::OnImportLandscapeClick, TEXT("Import Landscape"))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakeButton(&SLandscapeModeUI::OnBackupHeightmapsClick, TEXT("Backup Heightmaps"))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakeButton(&SLandscapeModeUI::OnRenameLandscapesClick, TEXT("Rename Landscapes"))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakeButton(&SLandscapeModeUI::OnCreateLandscapeMaterialsClick, TEXT("Create Landscape Materials"))
		]
	];
	
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply SLandscapeModeUI::OnImportLandscapePrxyClick()
{
	auto SettingsObject = GetDefault<UVCH_Settings>();
	check(SettingsObject);
	FString Path = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + SettingsObject->GlobalImportContentDir;
	FVCH_LandscapeFunctions::ImportLandscapeProxyToNewLevels(Path);

	return FReply::Handled();
}

FReply SLandscapeModeUI::OnImportLandscapeClick()
{
	auto SettingsObject = GetDefault<UVCH_Settings>();
	check(SettingsObject);
	FString Path = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + SettingsObject->GlobalImportContentDir;
	FVCH_LandscapeFunctions::ImpotrLandscapesToNewLevels(Path);
	return FReply::Handled();
}

FReply SLandscapeModeUI::OnBackupHeightmapsClick()
{
	auto SettingsObject = GetDefault<UVCH_Settings>();
	check(SettingsObject);
	FString PathToSave = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + SettingsObject->GlobalImportContentDir / SettingsObject->HeightmapsDir / TEXT("BackHeightmaps");
	FVCH_LandscapeFunctions::BackupHeightmaps(PathToSave);

	return FReply::Handled();
}

FReply SLandscapeModeUI::OnRenameLandscapesClick()
{
	FVCH_LandscapeFunctions::UpdateLandscapeName();
	return FReply::Handled();
}

FReply SLandscapeModeUI::OnCreateLandscapeMaterialsClick()
{
	FVCH_LandscapeFunctions::SetLandscapeMaterial();
	return FReply::Handled();
}

