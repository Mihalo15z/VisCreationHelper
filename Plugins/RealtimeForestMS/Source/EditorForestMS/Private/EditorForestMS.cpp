// Copyright Epic Games, Inc. All Rights Reserved.

#include "EditorForestMS.h"
#include "EditorForestMSStyle.h"
#include "EditorForestMSCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "SEditorForestWidget.h"
#include "Framework/Docking/TabManager.h"

static const FName EditorForestMSTabName("EditorForestMS");

#define LOCTEXT_NAMESPACE "FEditorForestMSModule"

void FEditorForestMSModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FEditorForestMSStyle::Initialize();
	FEditorForestMSStyle::ReloadTextures();

	FEditorForestMSCommands::Register();
	 
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FEditorForestMSCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FEditorForestMSModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FEditorForestMSModule::RegisterMenus));


	struct Local
	{
		static TSharedRef<SDockTab> SpawnWorldCreatorTab(const FSpawnTabArgs& SpawnTabArgs)
		{
			const TSharedRef<SDockTab> MajorTab =
				SNew(SDockTab)
				.TabRole(ETabRole::MajorTab);

			MajorTab->SetContent(SNew(SEditorForestWidget));

			return MajorTab;
		}
	};



	// Register a tab spawner so that our tab can be automatically restored from layout files
	FGlobalTabmanager::Get()->RegisterTabSpawner(EditorForestMSTabName, FOnSpawnTab::CreateStatic(&Local::SpawnWorldCreatorTab))
		.SetDisplayName(LOCTEXT("VCHTabTitle", "VCH"))
		.SetTooltipText(LOCTEXT("VCHTooltipText", "Open the VCH Tab"));
}

void FEditorForestMSModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FEditorForestMSStyle::Shutdown();

	FEditorForestMSCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterTabSpawner(EditorForestMSTabName);
}

void FEditorForestMSModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(EditorForestMSTabName);
}

void FEditorForestMSModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FEditorForestMSCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Plugins");
			//auto& newSection = Section.AddMenuSeparator("Plugins");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FEditorForestMSCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEditorForestMSModule, EditorForestMS)