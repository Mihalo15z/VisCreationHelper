// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SVCH_MainWidget.h"
#include "SlateOptMacros.h"
#include "Input/Reply.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "VCH_Test.h"
#include "Widgets/SFoliageModeWidget.h"

namespace
{
	const FString FoliageMode = TEXT("Foliage mode");
	const FString LandscapeMode = TEXT("Landscape Mode");
	const FString PreparationDataMode = TEXT("Preparation data mode");
	const FString TestMode = TEXT("Test Mode");

}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SVCH_MainWidget::Construct(const FArguments& InArgs)
{
	CurrentOptionsText = FText::FromString(TEXT("<<<"));
	
	ChildSlot
	[
		// Populate the widget
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Fill)
		.VAlign(EVerticalAlignment::VAlign_Top)
		.AutoHeight()
		[
			SAssignNew(SwitchModeComboBox, SComboBox<TSharedPtr<FString>>)
			.OptionsSource(&VariantSetOptions)
			.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
			{
				return SNew(STextBlock).Text(FText::FromString(*Item));
		})
			.Content()
			[
				SNew(STextBlock)
				.Text(this, &SVCH_MainWidget::GetCurrentOptionText)
			]
			.OnSelectionChanged(this, &SVCH_MainWidget::OnSelectedVariantSetChanged)

		]
		+ SVerticalBox::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Fill)
		.VAlign(EVerticalAlignment::VAlign_Fill)
		[
			SAssignNew(BorederForContent, SBorder)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.HAlign(EHorizontalAlignment::HAlign_Fill)
		]

	];

	BorederForContent->SetContent(SNew(SFoliageModeWidget));
	
	VariantSetOptions.Add(MakeShared<FString>(TEXT("Foliage mode")));
	VariantSetOptions.Add(MakeShared<FString>(TEXT("Preparation data mode")));
	VariantSetOptions.Add(MakeShared<FString>(TEXT("Landscape Mode")));
	VariantSetOptions.Add(MakeShared<FString>(TEXT("Test Mode")));
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply SVCH_MainWidget::OnClickTestButton()
{
	FVCH_Test TestObj;
	TestObj.GlobalTest();
	return FReply::Handled();
}

FReply SVCH_MainWidget::OnClickTestButton2()
{
	FVCH_Test TestObj;
	//TestObj.TestGetLevelNames();
	//TestObj.TestLevelsCoords();
	TestObj.TestClearAllIFA();
	return FReply::Handled();
}

void SVCH_MainWidget::OnSelectedVariantSetChanged(TSharedPtr<FString> NewItem, ESelectInfo::Type SelectType)
{
	CurrentOptionsText = FText::FromString(*NewItem);
	if (::LandscapeMode == *NewItem)
	{

	}
	else if (::FoliageMode == *NewItem)
	{
		BorederForContent->SetContent(SNew(SFoliageModeWidget));
	}
	else if (::TestMode == *NewItem)
	{
		BorederForContent->SetContent(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Left)
			.VAlign(EVerticalAlignment::VAlign_Top)
			.AutoHeight()
			[
				SNew(SButton).OnClicked(this, &SVCH_MainWidget::OnClickTestButton)
				.Text(FText::FromString(TEXT("Test Button")))
			]
		+ SVerticalBox::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Left)
			.VAlign(EVerticalAlignment::VAlign_Top)
			.AutoHeight()
			[
				SNew(SButton).OnClicked(this, &SVCH_MainWidget::OnClickTestButton2)
				.Text(FText::FromString(TEXT("Test Button 2 ")))
			]);
	}
	else if (::PreparationDataMode == *NewItem)
	{

	}
}

FText SVCH_MainWidget::GetCurrentOptionText() const
{
	return CurrentOptionsText;
}
