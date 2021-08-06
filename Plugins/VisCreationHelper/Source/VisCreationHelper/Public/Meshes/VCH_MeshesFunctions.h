// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


class UMaterial;
class UStaticMesh;
class FXmlNode;
class FXmlFile;

	/**
	*  структура основных параметров меша
	*/
struct VISCREATIONHELPER_API FVCHMeshData
{
public:
	int32 MinCoolDistance;
	int32 MaxCoolDistance;
	FString Name;  // имя будет ключем для базы 
	FString FullPath;
	FString GamePath;
	FVector Bounds;
	bool bCastShadow;
	bool bEnableCollision;
	uint8 CollisionType;
	bool bHaveSockets;
private:
	mutable TSoftObjectPtr<UStaticMesh> pStaticMesh;

public:
	//  load and return staticMesh
	FVCHMeshData();
	UStaticMesh* GetMesh() const;
	TSoftObjectPtr<UStaticMesh> GetSoftObjectPtrForMesh() const;
private:
	//   make string  XmlNode
	FString GetTextData();

	/** read data and init params
	*	return NameMesh (FString)
	*/
	FString ReadData(const FXmlNode* data);

private:
	friend class FVCH_MeshesFunctions;

};

enum class VISCREATIONHELPER_API ETypeMeshLib : uint8
{
	TML_All = 0,
	TML_Trees = 1,
	TML_Grass = 2,
	TML_SocketMesh = 3,
};

/**
 * 
 */
class VISCREATIONHELPER_API FVCH_MeshesFunctions
{
public:
	static FVCH_MeshesFunctions* Get();

	~FVCH_MeshesFunctions();
private:
	FVCH_MeshesFunctions();
public:
	void MakeLibraryData(FString Path);
	void LoadData(ETypeMeshLib type = ETypeMeshLib::TML_All);
	void ClearData();
	static UStaticMesh* FindMesh(FString Name);

	const FVCHMeshData* GetStaticMeshData(FString Name) const;

	TArray<FString> GetAllNames() const;
private:

	void ParseData(FXmlFile& file);


private:
	TMap<FString, FVCHMeshData> Library;
	bool bLoad = false;

	static FVCH_MeshesFunctions* SMLibrari;
	// global data
	//static const FString PathOfData;
	//static const FString AllMeshData;
	//static const FString ForestMeshes;
	//static const FString SocketMesh;


public:
	static void CheckXml(FString Path);


};
