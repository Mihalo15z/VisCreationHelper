// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SAssetsUI.h"
#include "SlateOptMacros.h"
#include "Assets/FVCH_AssetFunctions.h"
#include "Settings/VCH_Settings.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SAssetsUI::Construct(const FArguments& InArgs)
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
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			MakeButton(&SAssetsUI::OnSetTexturesForLandscapeMaterialsCleck, TEXT("Set Textures For Landscape Materials"))
		]	
	];
		
	
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply SAssetsUI::OnSetTexturesForLandscapeMaterialsCleck()
{
	// TO DO : Add ComboBox for other type textures
	auto SettingsObject = GetDefault<UVCH_Settings>();
	check(SettingsObject);
	FString ImportPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + SettingsObject->GlobalImportContentDir / SettingsObject->TexturesDir;
	FString PathToMaterials = TEXT("/Game/") + SettingsObject->PathToLandscapeMatAndTextures;
	FVCH_AssetFunctions::SetTexturesForLandMaterials(PathToMaterials, TEXT("BaseTexture"), ImportPath);
	return FReply::Handled();
}
