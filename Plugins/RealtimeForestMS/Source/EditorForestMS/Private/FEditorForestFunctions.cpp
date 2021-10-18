// Fill out your copyright notice in the Description page of Project Settings.


#include "FEditorForestFunctions.h"
#include "LandscapeProxy.h"
#include "Misc/FileHelper.h"
#include "Engine/World.h"
#include "EngineUtils.h"


FEditorForestFunctions::FEditorForestFunctions()
{
}

FEditorForestFunctions::~FEditorForestFunctions()
{
}

void FEditorForestFunctions::GenerateLandscapeXML()
{
	FString Result;
	Result.Append(TEXT("<?xml version=\"1.0\" encoding = \"Windows-1252\"?>\n\t<Data>\n\t<Levels>\n"));

	auto ClearLandscapeLabelLambda = [](const FString& InName)
	{
		auto Result = InName.Replace(TEXT("LandscapeStreming"), TEXT(""));
		Result.ReplaceInline(TEXT("Landscape"), TEXT(""));
		return Result;
	};

	for (TActorIterator<ALandscapeProxy> LandIter(GWorld, ALandscapeProxy::StaticClass()); LandIter; ++LandIter)
	{
		ALandscapeProxy* landscape = *LandIter;
		if(landscape)
		{
			// to do: check land name 
			auto LandName = ClearLandscapeLabelLambda(landscape->GetActorLabel());
			Result.Append(TEXT("\t\t<info name=\"") + LandName + TEXT("\" "));
			FVector origen;
			FVector bound;
			landscape->GetActorBounds(true, origen, bound);
			Result.Append(TEXT("\torigin=\"") + origen.ToString());
			Result.Append(TEXT("\"\tbound=\"" )+ bound.ToString());
			Result.Append(TEXT("\"\tlocation=\"") + landscape->GetActorLocation().ToString());
			Result.Append(TEXT("\" />\n"));
		}
	}
	Result.Append("\t</Levels>\n</Data>");
	FString Path = FPaths::ProjectConfigDir() + TEXT("InfoLevels.xml");
	FFileHelper::SaveStringToFile(Result, *Path);

}
