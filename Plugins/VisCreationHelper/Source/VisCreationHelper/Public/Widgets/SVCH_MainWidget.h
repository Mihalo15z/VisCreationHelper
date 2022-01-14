// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SBorder;
//class SComboButton;
template< class T>
class SComboBox;
/**
 * 
 */
class VISCREATIONHELPER_API SVCH_MainWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SVCH_MainWidget)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	FReply OnClickTestButton();
	FReply OnClickTestButton2();

private: // comboBox functions and data
	void OnSelectedVariantSetChanged(TSharedPtr<FString> NewItem, ESelectInfo::Type SelectType);

	TArray<TSharedPtr<FString> > VariantSetOptions;
	FString CurrentOptionsStr;
	FText CurrentOptionsText;
	FText GetCurrentOptionText() const;
private:
	TSharedPtr<SBorder> BorederForContent;
	TSharedPtr<SComboBox<TSharedPtr<FString> > > SwitchModeComboBox;

	friend struct FVCH_UIRegistrator;
};
