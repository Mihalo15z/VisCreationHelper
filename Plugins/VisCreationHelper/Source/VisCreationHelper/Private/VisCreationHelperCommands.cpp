// Copyright Epic Games, Inc. All Rights Reserved.

#include "VisCreationHelperCommands.h"

#define LOCTEXT_NAMESPACE "FVisCreationHelperModule"

void FVisCreationHelperCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "VisCreationHelper", "Execute VisCreationHelper action", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
