// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

template< class T>
class SComboBox;
/**
 * 
 */
class VISCREATIONHELPER_API SAssetsUI : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAssetsUI)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	void OnSelectedVariantSetChanged(TSharedPtr<FString> NewItem, ESelectInfo::Type SelectType);
	FText GetCurretnOptionText() const;

private:

	FReply OnSetTexturesForLandscapeMaterialsCleck();

	FString TetureParamNamen;

	TSharedPtr<SComboBox<TSharedPtr<FString> > > SwitchModeImportTComboBox;
	TArray<TSharedPtr<FString> > VariantSetOptions;
	FString CurrentTextureParamName;
};
