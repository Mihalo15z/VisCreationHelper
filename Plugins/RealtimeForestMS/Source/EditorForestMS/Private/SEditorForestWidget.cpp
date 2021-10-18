// Fill out your copyright notice in the Description page of Project Settings.


#include "SEditorForestWidget.h"
#include "SlateOptMacros.h"
#include "FEditorForestFunctions.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SEditorForestWidget::Construct(const FArguments& InArgs)
{
	
	ChildSlot
	[
		// Populate the widget
		SNew(SVerticalBox) 
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT ("Please, Loading all levels")))
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("Generate Landscape XML")))
			.OnClicked(this, &SEditorForestWidget::OnGenerateLandscapeXMLClick)
		]

	];
	
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION


FReply SEditorForestWidget::OnGenerateLandscapeXMLClick()
{
	FEditorForestFunctions::GenerateLandscapeXML();
	return FReply::Handled();
}