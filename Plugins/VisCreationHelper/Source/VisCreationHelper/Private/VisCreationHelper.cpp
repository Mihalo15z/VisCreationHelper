// Copyright Epic Games, Inc. All Rights Reserved.

#include "VisCreationHelper.h"
#include "VisCreationHelperStyle.h"
#include "VisCreationHelperCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include <Widgets/SVCH_MainWidget.h>
#include "Widgets/Docking/SDockTab.h"

static const FName VisCreationHelperTabName("VisCreationHelper");

#define LOCTEXT_NAMESPACE "FVisCreationHelperModule"

void FVisCreationHelperModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FVisCreationHelperStyle::Initialize();
	FVisCreationHelperStyle::ReloadTextures();

	FVisCreationHelperCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FVisCreationHelperCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FVisCreationHelperModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FVisCreationHelperModule::RegisterMenus));


	struct Local
	{
		static TSharedRef<SDockTab> SpawnWorldCreatorTab(const FSpawnTabArgs& SpawnTabArgs)
		{
			const TSharedRef<SDockTab> MajorTab =
				SNew(SDockTab)
				.TabRole(ETabRole::MajorTab);

			MajorTab->SetContent(SNew(SVCH_MainWidget));

			return MajorTab;
		}
	};



	// Register a tab spawner so that our tab can be automatically restored from layout files
	FGlobalTabmanager::Get()->RegisterTabSpawner(VisCreationHelperTabName, FOnSpawnTab::CreateStatic(&Local::SpawnWorldCreatorTab))
		.SetDisplayName(LOCTEXT("VCHTabTitle", "VCH"))
		.SetTooltipText(LOCTEXT("VCHTooltipText", "Open the VCH Tab"));
}

void FVisCreationHelperModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FVisCreationHelperStyle::Shutdown();

	FVisCreationHelperCommands::Unregister();
	FGlobalTabmanager::Get()->UnregisterTabSpawner(VisCreationHelperTabName);
}

void FVisCreationHelperModule::PluginButtonClicked()
{
	// Put your "OnButtonClicked" stuff here
	FText DialogText = FText::Format(
							LOCTEXT("PluginButtonDialogText", "Add code to {0} in {1} to override this button's actions"),
							FText::FromString(TEXT("FVisCreationHelperModule::PluginButtonClicked()")),
							FText::FromString(TEXT("VisCreationHelper.cpp"))
					   );
	//FMessageDialog::Open(EAppMsgType::Ok, DialogText);
	FGlobalTabmanager::Get()->InvokeTab(VisCreationHelperTabName);
}

void FVisCreationHelperModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FVisCreationHelperCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FVisCreationHelperCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVisCreationHelperModule, VisCreationHelper)