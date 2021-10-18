// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "ForestStruct.h"
#include "ThreadProcessingForest.h"
#include "ForestChunc.h"
#include "ForestManager.generated.h"


UCLASS()
class REALTIMEFORESTMS_API AForestManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AForestManager();
	~AForestManager();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	UPROPERTY(EditAnywhere, Category = "Forest")
		int32 InstanceEndCullDistance;
	UPROPERTY(EditAnywhere, Category = "Forest")
		int32 InstanceStartCullDistance;
	// средняя дистанция между деревьями
	UPROPERTY(EditAnywhere, Category = "Forest")
		float density;
	UPROPERTY(EditAnywhere, Category = "Forest")
		float scaleMin;
	UPROPERTY(EditAnywhere, Category = "Forest")
		float scaleMax;
	// массив мешей леса
	UPROPERTY(EditAnywhere, Category = "Forest")
		TArray<UStaticMesh*> forestMeshes;
	UPROPERTY(BlueprintReadWrite, Category = "Forest")
		FVector PlayerLocationCH;
	UPROPERTY()
		TArray<UForestChunc*> Forest;
	UPROPERTY(EditAnywhere, Category = "Forest")
		float ScaleCof;
	UPROPERTY(EditAnywhere, Category = "Forest")
		TArray<FForestType> ForestTypes;
	UPROPERTY(EditAnywhere, Category = "Forest")
		float CriticlaDelayTime = 0.50f;
private:
		
		TUniquePtr<ThreadProcessingForest> forestProcessor;
		int64 STime = 0;
		//FCriticalSection SyncObject;

		//float CriticlaDelayTime = 0.50f; //   максимальная время между кадрами при котором можжно обновлять новые квадраты леса

public:

	TSharedPtr<FLocationSync> LocationSync;

	UFUNCTION()
	void start();
private:
	void CalcForestInitData();

	
	
};
