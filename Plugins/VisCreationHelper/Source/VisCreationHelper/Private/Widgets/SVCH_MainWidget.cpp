// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SVCH_MainWidget.h"
#include "SlateOptMacros.h"
#include "Input/Reply.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "VCH_Test.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SVCH_MainWidget::Construct(const FArguments& InArgs)
{
	
	ChildSlot
	[
		// Populate the widget
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			SNew(SButton).OnClicked(this, &SVCH_MainWidget::OnClickTestButton)
			.Text(FText::FromString(TEXT("Test Button")))
		]
		+ SVerticalBox::Slot()
		[
			SNew(SButton).OnClicked(this, &SVCH_MainWidget::OnClickTestButton2)
			.Text(FText::FromString(TEXT("Test Button 2 ")))
		]

	];
	
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
	TestObj.TestGetLevelNames();
	TestObj.TestLevelsCoords();
	return FReply::Handled();
}
