#pragma once
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
//#include "Array.h"
//#include "UnrealString.h"
struct FForestStruct
{
public:
	FString name;
	TArray<UHierarchicalInstancedStaticMeshComponent* > Forest;
	bool bBuilt = false;
	bool bDelete = false;
	bool bActive = false;

};

