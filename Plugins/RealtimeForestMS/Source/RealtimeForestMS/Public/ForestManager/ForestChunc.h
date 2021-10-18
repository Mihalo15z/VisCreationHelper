// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "UObject/NoExportTypes.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Containers/UnrealString.h"
#include "CoreMinimal.h"
#include "ForestChunc.generated.h"

/**
 * 
 */
UCLASS()
class REALTIMEFORESTMS_API UForestChunc : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FString name;
	UPROPERTY()
	TArray<UHierarchicalInstancedStaticMeshComponent* > Forest;

	bool bBuilt = false;
	bool bDelete = false;
	bool bActive = false;
	bool bNewLevel = false;
	bool isBuilding = false;
	
	int64 buildTime = 0;

	UForestChunc();
};

struct FLocationSync
{
public:
	FVector PLocation;
	//FCriticalSection* SyncObject;

	FLocationSync()
	{
		//SyncObject = new FCriticalSection();
		PLocation = FVector::ZeroVector;
	};

	~FLocationSync()
	{
		//delete SyncObject;
	};

	inline void set(FVector& newLocation)
	{
		//FScopeLock lock(SyncObject);
		PLocation = newLocation;
	};

	inline FVector get()
	{
		//FScopeLock lock(SyncObject);
		return PLocation;
	};

	//private:
	// добавить запрет на копирование.
	inline FLocationSync operator=(const FLocationSync cg)
	{
		return cg;
	};

};


USTRUCT(BlueprintType)
struct REALTIMEFORESTMS_API FPlant
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = Mesh)
		TSoftObjectPtr<class UStaticMesh> mesh;
	// соотношение к остольным деревьям. Сумирует все кофициенты и находит процент(5 берез, 3 ели, ...)
	UPROPERTY(EditAnywhere, meta = (ClampMin = 1, ClampMax = 255))
		int32 ratio;
	//  индекс в массиве Forest->Forest
	UPROPERTY()
		int32 index;
	FPlant() :ratio(1), index(-1) {};
};

USTRUCT(BlueprintType)
struct REALTIMEFORESTMS_API FForestType
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0, ClampMax = 254), Category = "Forest")
		int32 minRange;
	UPROPERTY(EditAnywhere, meta = (ClampMin = 1, ClampMax = 255), Category = "Forest")
		int32 maxRange;
	UPROPERTY(EditAnywhere, Category = "Forest")
		TArray<FPlant> Plants;

private:
	// так как общее количество не большое и спользуем массив индексов, так увеличим немного скорость работы, и обезапасим от некоректного ввода
	TArray<uint8> indices;

public:
	FForestType() :minRange(0), maxRange(255) {};
	// создаем массив индексов для процентного соотношения
	void GeneratedIndecesArray();
	FORCEINLINE int32 GetIndex(uint8 val)  const 
	{
		if (indices.Num() > 0)
		{
			return indices[val % indices.Num()];
		}
		return -1;
	}
};