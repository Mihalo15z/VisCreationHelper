// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SPreparationDataModeUI.h"
#include "SlateOptMacros.h"
#include "Settings/VCH_Settings.h"
#include "PreparationData/FVCH_PreparationDataFunctions.h"
#include "Widgets/Input/SSpinBox.h"


DECLARE_LOG_CATEGORY_CLASS(VCH_UILog, Log, All);

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SPreparationDataModeUI::Construct(const FArguments& InArgs)
{
	
	auto MakeButton = [&](auto FunClick, const FString& InLable )
	{
		return SNew(SButton)
			.OnClicked(this, FunClick) 
			.Text(FText::FromString(InLable));
	};

	auto SettingsObject = GetDefault<UVCH_Settings>();
	check(SettingsObject);

	CorrectHMapValue = SettingsObject->CorrectHMapValue;
	ChildSlot
	[
		SNew(SVerticalBox)
	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			MakeButton(&SPreparationDataModeUI::OnGenerateLevelsXMLClick, TEXT("Generate Levels XML"))
		]
	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			MakeButton(&SPreparationDataModeUI::OnGetMinMaxForHeightmapsClick, TEXT("Get Min/Max For Heightmaps"))
		]
	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot().FillWidth(0.5)
			[
				MakeButton(&SPreparationDataModeUI::OnCorrectHMapsRangeClick, TEXT("Correct HMaps Range"))
			]
			+ SHorizontalBox::Slot().FillWidth(0.5).Padding(FMargin(5.f,2.f))
			[
				SNew(SSpinBox<int32>)
				.MinSliderValue(1)
				.MaxSliderValue(32'768)
				.Value(SettingsObject->CorrectHMapValue)
				.OnValueChanged_Lambda([&](int32 Value) { CorrectHMapValue = Value; })
			]	
		]
	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			MakeButton(&SPreparationDataModeUI::OnRemoveCrackForHeightmapsClick, TEXT("Remove Crack For Heightmaps"))
		]
	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			MakeButton(&SPreparationDataModeUI::OnCheckHeightmapsClick, TEXT("Check Heightmaps"))
		]
	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			MakeButton(&SPreparationDataModeUI::OnMakeHMapForTileImportClick, TEXT("Make HMap For Tile Import"))
		]
	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			MakeButton(&SPreparationDataModeUI::OnCalculateGeneratedParamsClick, TEXT("Calculate Generated Params"))
		]
	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			MakeButton(&SPreparationDataModeUI::OnConverPngTo8RawClick, TEXT("Convert Png To 8Raw"))
		]
	];
	
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply SPreparationDataModeUI::OnGenerateLevelsXMLClick()
{
	// may be add GWarn for click button event
	//GWarn->BeginSlowTask(FText::FromString(TEXT("Generate Levels XML")), true);
	auto SettingsObject = GetDefault<UVCH_Settings>();
	check(SettingsObject);
	FVCH_PreparationDataFunctions::MakeXMlForMapFiles(FPaths::ProjectDir() + SettingsObject->GlobalImportContentDir / SettingsObject->MapsDir, TEXT("Land.xml"));
	return FReply::Handled(); 
}
FReply SPreparationDataModeUI::OnGetMinMaxForHeightmapsClick()
{
	auto SettingsObject = GetDefault<UVCH_Settings>();
	check(SettingsObject);
	FString PathToHeightmaps = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + SettingsObject->GlobalImportContentDir / SettingsObject->HeightmapsDir;
	auto HeightmapsMap = FVCH_PreparationDataFunctions::GetAllHeightmaps(PathToHeightmaps, SettingsObject->GetFinalResolution());

	uint16 MinH, MaxH;
	FVCH_PreparationDataFunctions::GetMinMaxForHeightmaps(HeightmapsMap, MinH, MaxH);

	return FReply::Handled();
}
FReply SPreparationDataModeUI::OnCorrectHMapsRangeClick()
{
	auto SettingsObject = GetDefault<UVCH_Settings>();
	check(SettingsObject);
	FString PathToHeightmaps = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + SettingsObject->GlobalImportContentDir / SettingsObject->HeightmapsDir;
	auto HeightmapsMap = FVCH_PreparationDataFunctions::GetAllHeightmaps(PathToHeightmaps, SettingsObject->GetFinalResolution());
	
	check(CorrectHMapValue > 0 && CorrectHMapValue < 32'768);
	FVCH_PreparationDataFunctions::CorrectHMapsRange(HeightmapsMap, CorrectHMapValue);
	
	if (FVCH_PreparationDataFunctions::CheckHeightmaps(HeightmapsMap, SettingsObject->GetFinalResolution(), SettingsObject->NameMask))
	{
		FVCH_PreparationDataFunctions::RemoveCrackForHeightmaps(HeightmapsMap, SettingsObject->NameMask, SettingsObject->GetFinalResolution());
	}
	FVCH_PreparationDataFunctions::SaveHeightMaps(HeightmapsMap, PathToHeightmaps + TEXT("/RightHeightmaps/"));

	return FReply::Handled();
}
FReply SPreparationDataModeUI::OnRemoveCrackForHeightmapsClick()
{
	auto SettingsObject = GetDefault<UVCH_Settings>();
	check(SettingsObject);
	FString PathToHeightmaps = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + SettingsObject->GlobalImportContentDir / SettingsObject->HeightmapsDir;
	auto HeightmapsMap = FVCH_PreparationDataFunctions::GetAllHeightmaps(PathToHeightmaps, SettingsObject->GetFinalResolution());

	if (FVCH_PreparationDataFunctions::CheckHeightmaps(HeightmapsMap, SettingsObject->GetFinalResolution(), SettingsObject->NameMask))
	{
		FVCH_PreparationDataFunctions::RemoveCrackForHeightmaps(HeightmapsMap, SettingsObject->NameMask, SettingsObject->GetFinalResolution());
		FVCH_PreparationDataFunctions::SaveHeightMaps(HeightmapsMap, PathToHeightmaps + TEXT("/RightHeightmaps/"));
	}

	return FReply::Handled();
}
FReply SPreparationDataModeUI::OnCheckHeightmapsClick()
{
	auto SettingsObject = GetDefault<UVCH_Settings>();
	check(SettingsObject);
	FString PathToHeightmaps = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + SettingsObject->GlobalImportContentDir / SettingsObject->HeightmapsDir;
	auto HeightmapsMap = FVCH_PreparationDataFunctions::GetAllHeightmaps(PathToHeightmaps, SettingsObject->GetFinalResolution());
	
	if (FVCH_PreparationDataFunctions::CheckHeightmaps(HeightmapsMap, SettingsObject->GetFinalResolution(), SettingsObject->NameMask))
	{
		UE_LOG(VCH_UILog, Error, TEXT("BAD Heightmaps"));
		// to do : Send in loge UI
	}
	else
	{
		UE_LOG(VCH_UILog, Warning, TEXT("GOOD Heightmaps"));
		// to do : Send in loge UI
	}
	return FReply::Handled();
}

FReply SPreparationDataModeUI::OnMakeHMapForTileImportClick()
{
	auto SettingsObject = GetDefault<UVCH_Settings>();
	check(SettingsObject);
	FString PathToHeightmaps = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + SettingsObject->GlobalImportContentDir / SettingsObject->HeightmapsDir;
	FVCH_PreparationDataFunctions::CopyAndRenameHMaps(PathToHeightmaps, TEXT("TileImport"), SettingsObject->NameMask);
	return FReply::Handled();
}

FReply SPreparationDataModeUI::OnCalculateGeneratedParamsClick()
{
	auto SettingsObject = GetDefault<UVCH_Settings>();
	check(SettingsObject);
	FIntPoint Offset;
	FVector Scale;
	FString PathToLandConfig = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) +  SettingsObject->GlobalImportContentDir / SettingsObject->MapsDir / SettingsObject->LandConfigName;
	FVCH_PreparationDataFunctions::GetOffsetAndScale(
		PathToLandConfig,
		SettingsObject->ZeroLevelName,
		SettingsObject->NameMask,
		FCString::Atod(*SettingsObject->Scale),
		FCString::Atod(*SettingsObject->DeltaHeight),
		FCString::Atod(*SettingsObject->LevelSize),
		SettingsObject->Resolution,
		Offset,
		Scale);
	return FReply::Handled();
}


FReply SPreparationDataModeUI::OnConverPngTo8RawClick()
{
	FString PathToData = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir())/TEXT("ConvertData");
	FVCH_PreparationDataFunctions::ConvertTextureToRaw8b(PathToData);
	return FReply::Handled();
}

