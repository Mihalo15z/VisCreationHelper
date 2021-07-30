// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "VisCreationHelperStyle.h"

class FVisCreationHelperCommands : public TCommands<FVisCreationHelperCommands>
{
public:

	FVisCreationHelperCommands()
		: TCommands<FVisCreationHelperCommands>(TEXT("VisCreationHelper"), NSLOCTEXT("Contexts", "VisCreationHelper", "VisCreationHelper Plugin"), NAME_None, FVisCreationHelperStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
