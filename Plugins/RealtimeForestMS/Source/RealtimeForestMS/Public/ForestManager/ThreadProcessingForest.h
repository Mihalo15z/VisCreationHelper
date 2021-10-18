// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
//#include "ForestStruct.h"
#include "ForestChunc.h"
#include "Runtime/Core/Public/HAL/ThreadingBase.h"
/**
 * 
 */
class IImageWrapperModule;

enum class  MaskChannel : int32
{
	R = 0,
	G = 1,
	B = 2,
	A = 3
};

class FLevelData
{
public:
	FString name;
	FVector location;
	FVector origin;
	FVector bound;
};

class FProcessingData
{
public:
	FLevelData* levelData;
    UForestChunc* chunc;
	//TArray<FMatrix> InstanceTransforms;
};

struct FForestBuilder
{
	FMatrix XForm;
	FBox MeshBox;
	FStaticMeshInstanceData InstanceBuffer;
	TArray<FClusterNode> ClusterTree;
	int32 OutOcclusionLayerNum;
};


class REALTIMEFORESTMS_API ThreadProcessingForest : FRunnable
{
public:
	ThreadProcessingForest(const float _minScale, const float _maxScale, const float _density, const float scaleCof/*, IImageWrapperModule& _ImageWrapperModule*/ /*, FCriticalSection &_SyncObject*/);
	~ThreadProcessingForest();
	void WaitThread()
	{
		while (!isFinishTakt)
		{
			FPlatformProcess::Sleep(0.2);
		}
	};
private:
	//FRunnable function
	
	virtual bool Init() override;
	virtual uint32 Run() override;
	bool IsActive() const;
	
	//virtual void Exit() override;

	// FRunnable variables
	bool isFinishTakt = true;
	bool Stopping = false;
	TUniquePtr<FRunnableThread> Thread;	
	//FCriticalSection* SyncObject;
public:
	bool Start(TArray<UForestChunc*>& _Forest, TArray<FForestType>& forestType, TSharedPtr<FLocationSync> _LocationSync);
	//virtual void Exit() override;
	virtual void Stop() override;
private:
	//bool startThread(TArray<FForestStruct> Forest) { return true; }

	TArray<UForestChunc*>* Forest;
	/*const */TArray<FForestType>* ForestType;
	TArray<uint8> randomData;
	const float minScale;
	const float maxScale;
	const float density;
	const float scaleCof;
	TArray<FLevelData> levelData;
	TQueue<FProcessingData> placeQueue;
	TArray<FString> NamesMask;
	TArray<FString> NamesHMAp;
	IImageWrapperModule& ImageWrapperModule;
	
	

public:
	FVector PlayerPosition ;


private:
	void PlaceForest();
	void PlaceForestParallel(); // not work , update this!!!
	void DeleteForest();
	void checkAllLevels();
	void checkActiveLevel();
	void checkPlayerPoaition(); // 
	void getLevelData(FString& path, TArray<FLevelData>& levelData);
	const TArray<uint16> getHeightMapData(const FString& path);
	const TArray64<uint8> getForestMaskData(const FString& path);
	// перенсем применение результата в основной поток;
	//FORCEINLINE void setResult(UHierarchicalInstancedStaticMeshComponent* Hismc, TArray<FClusterNode>& ClusterNode, TArray<FClusterNode>& ClusterTree,
	//	TArray<int32>& SortedInstances,TArray<int32>& InstanceReorderTable, int32& OcclusionLayerNumNodes)
	//{
	//	Hismc->NumBuiltInstances = Hismc->SortedInstances.Num();
	//	Exchange(ClusterTree, ClusterNode);
	//	Exchange(Hismc->SortedInstances, SortedInstances);
	//	Exchange(Hismc->InstanceReorderTable, InstanceReorderTable);
	//	Hismc->OcclusionLayerNumNodes = OcclusionLayerNumNodes;
	//	Hismc->BuiltInstanceBounds = (ClusterTree.Num() > 0 ? FBox(ClusterTree[0].BoundMin, ClusterTree[0].BoundMax) : FBox(EForceInit::ForceInitToZero));
	//	Hismc->NumBuiltInstances = Hismc->SortedInstances.Num();
	//	//Hismc->RemoveInstances(
	//	//Hismc->RemovedInstances.Empty();
	//	Hismc->UnbuiltInstanceBounds.Init();
	//	//Hismc->
	//}

	void DebugsetDelete();
	//void UncompressPNGData(const ERGBFormat::Type InFormat, const int32 InBitDepth, TArray<uint8>& CompressedData, TArray<uint8>& RawData);
public:
	TSharedPtr<FLocationSync> LocationSync;

public:
	//debug function
	FORCEINLINE FVector getPosition() const
	{
		//if(PlayerPosition)
			return PlayerPosition;
	}

private:
	static void threadFunction(ThreadProcessingForest* obj)
	{
		obj->Init();
		obj->Run();
	};

private:
	static constexpr float BoundScaleCoff = 2.6f;

};
