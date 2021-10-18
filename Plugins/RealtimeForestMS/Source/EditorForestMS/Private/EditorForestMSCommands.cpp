// Copyright Epic Games, Inc. All Rights Reserved.

#include "EditorForestMSCommands.h"

#define LOCTEXT_NAMESPACE "FEditorForestMSModule"

void FEditorForestMSCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "EditorForestMS", "Editor Forest", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
