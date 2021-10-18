// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "EditorForestMSStyle.h"

class FEditorForestMSCommands : public TCommands<FEditorForestMSCommands>
{
public:

	FEditorForestMSCommands()
		: TCommands<FEditorForestMSCommands>(TEXT("EditorForestMS"), NSLOCTEXT("Contexts", "EditorForestMS", "EditorForestMS Plugin"), NAME_None, FEditorForestMSStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
