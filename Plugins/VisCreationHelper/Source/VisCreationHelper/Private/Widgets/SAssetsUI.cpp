// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SAssetsUI.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/SBoxPanel.h"
#include "Assets/FVCH_AssetFunctions.h"
#include "Settings/VCH_Settings.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SAssetsUI::Construct(const FArguments& InArgs)
{
	auto SettingsObject = GetDefault<UVCH_Settings>();
	check(SettingsObject);
	for (auto ParamName : SettingsObject->TextureParamNames)
	{
		VariantSetOptions.Add(MakeShared<FString>(ParamName));
	}
	//VariantSetOptions.Add(MakeShared<FString>(FString(SettingsObject->TParamName_BaseMask)));
	//VariantSetOptions.Add(MakeShared<FString>(FString(SettingsObject->TParamName_WaterMas)));

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
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			//.AutoWidth()
			[
				MakeButton(&SAssetsUI::OnSetTexturesForLandscapeMaterialsCleck, TEXT("Set Textures For Landscape Materials"))
			]
			+SHorizontalBox::Slot()
			[
				SAssignNew(SwitchModeImportTComboBox, SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&VariantSetOptions)
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
				{
					return SNew(STextBlock).Text(FText::FromString(*Item));
				})
				
				.Content()
				[
					SNew(STextBlock).Text(this , &SAssetsUI::GetCurretnOptionText)
				]
				.OnSelectionChanged(this, &SAssetsUI::OnSelectedVariantSetChanged)
				.InitiallySelectedItem(VariantSetOptions[0])
			]
			
		]	
	];
		
	CurrentTextureParamName = *(VariantSetOptions[0]);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply SAssetsUI::OnSetTexturesForLandscapeMaterialsCleck()
{
	// TO DO : Add ComboBox for other type textures
	auto SettingsObject = GetDefault<UVCH_Settings>();
	check(SettingsObject && !CurrentTextureParamName.IsEmpty());

	FString ImportPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + SettingsObject->GlobalImportContentDir / SettingsObject->TexturesDir;
	FString PathToMaterials = TEXT("/Game/") + SettingsObject->PathToLandscapeMatAndTextures;
	FVCH_AssetFunctions::SetTexturesForLandMaterials(PathToMaterials, TEXT("BaseTexture"), ImportPath);
	return FReply::Handled();
}

void SAssetsUI::OnSelectedVariantSetChanged(TSharedPtr<FString> NewItem, ESelectInfo::Type SelectType)
{
	CurrentTextureParamName = *NewItem;
}

FText SAssetsUI::GetCurretnOptionText() const
{
	return FText::FromString(CurrentTextureParamName);
}
