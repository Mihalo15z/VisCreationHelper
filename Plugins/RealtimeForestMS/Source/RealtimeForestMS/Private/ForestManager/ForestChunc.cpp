// Fill out your copyright notice in the Description page of Project Settings.


#include "ForestManager/ForestChunc.h"


UForestChunc::UForestChunc()
{
	name = "None";
	bBuilt = bDelete = bActive = bNewLevel = false;
};


void FForestType::GeneratedIndecesArray()
{
	for (const auto& plant : Plants)
	{
		int32 sIndex(indices.Num());
		indices.AddDefaulted(plant.ratio);
		for (; sIndex < indices.Num(); ++sIndex)
		{
			indices[sIndex] = plant.index;
		}
	}
}