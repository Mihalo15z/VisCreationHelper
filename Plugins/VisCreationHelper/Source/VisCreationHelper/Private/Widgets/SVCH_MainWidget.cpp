// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SVCH_MainWidget.h"
#include "SlateOptMacros.h"
#include "Input/Reply.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "VCH_Test.h"
#include "Widgets/SFoliageModeWidget.h"
#include <Widgets/SLandscapeModeUI.h>
#include <Widgets/SPreparationDataModeUI.h>
#include "Widgets/SAssetsUI.h"





namespace
{
	class FBaseUIConstructor
	{
	public:
		virtual TSharedRef<SCompoundWidget> MakeUI() = 0;
		virtual ~FBaseUIConstructor() {};
	};

	template<class T>
	class TUIConstructor : public FBaseUIConstructor
	{
	public:
		virtual  TSharedRef<SCompoundWidget> MakeUI() override
		{
			return SNew(T);
		}
		virtual ~TUIConstructor<T>() {};
	};

	template <class T>
	TUniquePtr<FBaseUIConstructor> MakeUniqueUIConstructor()
	{
		return MakeUnique<TUIConstructor<T> >();
	}


	const FString FoliageMode = TEXT("Foliage mode");
	const FString LandscapeMode = TEXT("Landscape Mode");
	const FString PreparationDataMode = TEXT("Preparation data mode");
	const FString TestMode = TEXT("Test Mode");
	const FString AssetMode = TEXT("Asset Mode");


	TMap <FString, TUniquePtr<FBaseUIConstructor> > UIClasses{};

}

struct FVCH_UIRegistrator
{
private:
	SVCH_MainWidget* MainWidget = nullptr;
public:
	FVCH_UIRegistrator(SVCH_MainWidget* InMainWidget) :MainWidget(InMainWidget) 
	{
		check(MainWidget);
	}
	template <class T>
	void RegistrationUI(const FString& InUI_Name)
	{
		MainWidget->VariantSetOptions.Add(MakeShared<FString>(InUI_Name));
		UIClasses.Add(InUI_Name, (::MakeUniqueUIConstructor<T>()));
	}
};



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
				SNew(SBorder)
				//.ColorAndOpacity(FLinearColor(0.5f, 0.5f, 7.f, 1.f))
				.ColorAndOpacity(FLinearColor(0.1f, 0.1f, 0.7f, 1.f))
				.BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.7f, 1.f))
				.ForegroundColor(FLinearColor(0.1f, 0.1f, 0.7f, 1.f))
				[
					SNew(STextBlock)
					.Text(this, &SVCH_MainWidget::GetCurrentOptionText)
				]
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
			.Padding(FMargin(2.f, 20.f, 2.f, 2.f))
		]

	];


	BorederForContent->SetContent(SNew(SFoliageModeWidget));
	
	FVCH_UIRegistrator UI_Registrator(this);
	UI_Registrator.RegistrationUI<SFoliageModeWidget>(::FoliageMode);
	UI_Registrator.RegistrationUI<SLandscapeModeUI>(::LandscapeMode);
	UI_Registrator.RegistrationUI<SPreparationDataModeUI>(::PreparationDataMode);
	UI_Registrator.RegistrationUI<SAssetsUI>(::AssetMode);

	VariantSetOptions.Add(MakeShared<FString>(TEXT("Test Mode")));

	
	SwitchModeComboBox->SetSelectedItem(VariantSetOptions[0]);

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

	if (::TestMode == *NewItem)
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
		return;
	}

	if (UIClasses.Contains(*NewItem))
	{
		BorederForContent->SetContent(UIClasses[*NewItem]->MakeUI());
	}
}

FText SVCH_MainWidget::GetCurrentOptionText() const
{
	return CurrentOptionsText;
}
