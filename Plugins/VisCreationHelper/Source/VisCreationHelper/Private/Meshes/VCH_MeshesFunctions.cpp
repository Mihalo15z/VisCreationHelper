// Fill out your copyright notice in the Description page of Project Settings.


#include "Meshes/VCH_MeshesFunctions.h"

FVCHMeshData::FVCHMeshData() : MinCoolDistance(3500), MaxCoolDistance(5000), bCastShadow(true), bEnableCollision(true), CollisionType(uint8(ECollisionEnabled::QueryAndPhysics))
{
}

UStaticMesh * FVCHMeshData::GetMesh() const
{
	return nullptr;
}
