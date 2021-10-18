// Fill out your copyright notice in the Description page of Project Settings.

#include "ForestManager/ForestManager.h"
#include <ctime>
#include "IImageWrapperModule.h"
#include "Engine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"



// Sets default values
AForestManager::AForestManager()
{
	SetTickGroup(TG_PostUpdateWork);
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Forest.Reserve(9); // обрабатывается 9 квадратов;
	InstanceStartCullDistance = 400000.f;
	InstanceEndCullDistance = 500000.f;
	density = 800.f;
	scaleMin = 0.7f;
	scaleMax = 1.5f;
	ScaleCof = 0.78125f;
	PlayerLocationCH = FVector::ZeroVector;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootCompon"));
	CriticlaDelayTime = 0.020f;
	
}

AForestManager::~AForestManager()
{
	//if (forestProcessor.IsValid())
	//{
	//	forestProcessor->Exit();
	//	for (auto chunc : Forest)
	//	{
	//		for (auto treeType : chunc->Forest)
	//		{
	//			if (IsValid(treeType))
	//			if (treeType)
	//				treeType->DestroyComponent();
	//		}
	//	}
	//}
	//if(Forest.Num())
	//	Forest.Empty();
	//int i = 0;
}

// Called when the game starts or when spawned
void AForestManager::BeginPlay()
{
	Super::BeginPlay();

	SetTickGroup(TG_PostUpdateWork);

	RootComponent->SetComponentTickEnabledAsync(true);
	if (ForestTypes.Num() > 0)
	{
		CalcForestInitData();
	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
	forestProcessor = TUniquePtr<ThreadProcessingForest>(new ThreadProcessingForest(scaleMin, scaleMax, density, ScaleCof/*, ImageWrapperModule*//*, SyncObject*/));
	for (int i = 0; i < 9; i++) // обрабатывается  только  9 квадратов;
	{
		UForestChunc* chunc(NewObject<UForestChunc>(this));
		chunc->name = "None";
		chunc->bActive=(false);
		chunc->bBuilt=(false);
		chunc->bDelete=(false);
		chunc->bNewLevel=(false);
		for (auto mesh : forestMeshes)
		{

			UHierarchicalInstancedStaticMeshComponent* HISMC(NewObject<UHierarchicalInstancedStaticMeshComponent>(this));
			//HISMC->SetComponentTickEnabledAsync(true);
			HISMC->SetRelativeLocation(FVector::ZeroVector);
			//HISMC->AttachTo(RootComponent);
			HISMC->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
			HISMC->SetStaticMesh(mesh);
			//HISMC->bAffectDynamicIndirectLighting = false;
			//HISMC->in
			//HISMC->bEnableDensityScaling = false;
			//HISMC->bRenderCustomDepth = true;
			//HISMC->bAllowCullDistanceVolume = true;
			//HISMC->bUseAsOccluder = true;
			HISMC->InstanceEndCullDistance = InstanceEndCullDistance;
			HISMC->InstanceStartCullDistance = InstanceStartCullDistance;
			HISMC->bIsAsyncBuilding = true;									//  для возможности добавления в другом потоке перевести флаг в +
			
			HISMC->SetSimulatePhysics(false);
			HISMC->bDisableCollision = true;
			HISMC->SetCollisionProfileName(TEXT("NoCollision"));
			HISMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					
			HISMC->bHasPerInstanceHitProxies = false;
			HISMC->SetCanEverAffectNavigation(false);
			HISMC->bAffectDistanceFieldLighting = false;
			HISMC->SetVisibility(false);
			HISMC->bAutoActivate = false;

			HISMC->Mobility = EComponentMobility::Static;
			HISMC->bCastDynamicShadow = true;
			HISMC->bCastStaticShadow = false;
			HISMC->bAlwaysCreatePhysicsState = false;
			HISMC->bApplyImpulseOnDamage = false;
			HISMC->bCastDistanceFieldIndirectShadow = false;
			HISMC->bCastVolumetricTranslucentShadow = false;
			HISMC->bReceivesDecals = false;
			HISMC->bSelectable = false;
			HISMC->bAffectDistanceFieldLighting = false;
			//HISMC->UseDynamicInstanceBuffer = true;
			//HISMC->SetCastShadow(false);
			HISMC->bAutoRebuildTreeOnInstanceChanges = false;
			//HISMC->bAttachedToStreamingManagerAsStatic = true;
			HISMC->bAutoRegister = false;
			//HISMC->bDisableCollision = true;
			HISMC->bNavigationRelevant = false;

			HISMC->SetShouldUpdatePhysicsVolume(false);
			//HISMC->bShouldUpdatePhysicsVolume = false;
			//HISMC->bHasCustomNavigableGeometry = false;
			HISMC->CreationMethod = EComponentCreationMethod::Native;
			//HISMC->bCanPhysicsStateCreated = false;
			//HISMC->RegisterComponentWithWorld(this->GetWorld());
			//HISMC->Activate(false);
			//HISMC->Activate(true);
			//HISMC->StaticMesh->SetFlags(RF_NeedPostLoad);

			chunc->Forest.Add(HISMC);
		}
		Forest.Add(chunc);
	}

	LocationSync = MakeShareable<FLocationSync>(new FLocationSync);
	if (forestMeshes.Num() != 0 && forestProcessor.IsValid())
	{
		if (!forestProcessor->Start(Forest, ForestTypes, LocationSync))
		{
			GEngine->AddOnScreenDebugMessage(0, 10, FColor::Red, FString("forestProcessor disable"), true);
		}
	}
	FString ConfigDataStr;
	FString pathToConfig(FPaths::ProjectConfigDir() + TEXT("ForestManager.config"));
	if (FFileHelper::LoadFileToString(ConfigDataStr, *pathToConfig))
	{
		CriticlaDelayTime = FCString::Atof(*ConfigDataStr); //  время между кадрами при котором можно обрабатывать новые квадраты леса

		if (CriticlaDelayTime < 0.005f)
		{
			CriticlaDelayTime = 0.020f;
		}
	}


}


void AForestManager::start()
{
	forestProcessor->Start(Forest, ForestTypes, LocationSync);
}

void AForestManager::CalcForestInitData()
{
	forestMeshes.Empty();
	for (auto& fType : ForestTypes)
	{
		for (auto& plant : fType.Plants)
		{
			auto mesh(plant.mesh.LoadSynchronous());
			if (mesh)
			{
				plant.index = forestMeshes.AddUnique(mesh);
			}
		}
		fType.GeneratedIndecesArray();
	}
}


void AForestManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (forestProcessor.IsValid())
	{
		forestProcessor->Stop();
		forestProcessor->WaitThread();
	}
			

	//Super::OnRemoveFromWorld();

}

// Called every frame
void AForestManager::Tick( float DeltaTime )
{	
	/*GEngine->bEnableOnScreenDebugMessages = true;
	GEngine->bEnableOnScreenDebugMessagesDisplay = true;
	GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, "FIgBam!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");*/
	//uint64 timett(clock());
	Super::Tick( DeltaTime );
	//return;
	LocationSync->set(PlayerLocationCH);
	//!!!!!!!!!!!!!!
	// если есть просадка фпс, то регистрацию компонентов не производим!
	if (DeltaTime > CriticlaDelayTime)
	{
		UE_LOG(LogTemp, Error, TEXT(" location = %s , fps = %f, time = "), *PlayerLocationCH.ToString(),  1 / DeltaTime, DeltaTime);
		return;
	}
	
	//FString  debugString("Debug\n\n\n");

	for (auto& chunk : Forest)
	{
		if (chunk->bActive && chunk->bDelete)
		{
			for (auto HISMC : chunk->Forest)
			{
				if (HISMC->IsRegistered())
				{
					HISMC->MarkForNeededEndOfFrameUpdate();
					HISMC->SetVisibility(false);
					HISMC->Deactivate();
					HISMC->UnregisterComponent();
					HISMC->MarkForNeededEndOfFrameUpdate();
				}
			}

			//chunk->bDelete = false;
			chunk->bActive=(false);
			// 			GEngine->AddOnScreenDebugMessage(-1, 4, FColor::Green, FString("Debug. Delete chunk - ") + chunk->name, false);
			// 			if(LocationSync->get() == FVector::ZeroVector)
			// 				GEngine->AddOnScreenDebugMessage(-1, 4, FColor::Red, FString("Debug. Location - 000") , false);
		}
	}

	//return;
	bool bOneActivation = false;  //  для одного на тик
	for (auto& chunk : Forest)
	{	
		/* активируем квадрат леса если он построен*/
		
		//  активирую один элемент Hismc  за тик
		if (chunk->bBuilt && !chunk->bActive /*&& !bOneActivation*/)
		{
			bOneActivation = true;
			
			//int64 StartTime(clock());
			for (int32 i = 0; i < chunk->Forest.Num(); i++)
			{
				if (!(chunk->Forest[i]->IsRegistered()))
				{
					//uint64 timett(clock());
					
					//chunk->Forest[i]->UnbuiltInstanceBounds.Init();
					//chunk->Forest[i]->ReleasePerInstanceRenderData();
					//chunk->Forest[i]->MarkRenderStateDirty();
					//chunk->Forest[i]->ReleasePerInstanceRenderData();
					//chunk->Forest[i]->InitPerInstanceRenderData();
					//chunk->Forest[i]->Modify(false);
					//chunk->Forest[i]->RegisterComponent();
					chunk->Forest[i]->MarkForNeededEndOfFrameRecreate();
					chunk->Forest[i]->RegisterComponentWithWorld(this->GetWorld());
					//chunk->Forest[i]->MarkRenderStateDirty();
					chunk->Forest[i]->SetVisibility(true);
					chunk->Forest[i]->MarkForNeededEndOfFrameRecreate();
					chunk->Forest[i]->MarkForNeededEndOfFrameUpdate();
					if (i == chunk->Forest.Num() - 1)
					{
						chunk->bBuilt = (false);
						chunk->bActive = (true);
					}
					//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Orange, FString::Printf(TEXT("time reg - %i"), clock() - timett), false);

					return;
					//break;
				}
			}

			/*if (bOneActivation)
			{
				return;
			}*/
			//chunk->buildTime = clock() - StartTime;
			//chunk->bBuilt = false;
			//chunk->bActive = true;
		}

		/*деактивриуе квадрат леса если он не находится зоне обработки*/
//		if (chunk->bActive && chunk->bDelete)
//		{
//			for (auto HISMC : chunk->Forest)
//			{
//				
//				HISMC->SetVisibility(false);
//				HISMC->Deactivate();
//				if(HISMC->IsRegistered())
//					HISMC->UnregisterComponent();
//			}
//
//
//
//
//			//chunk->bDelete = false;
//			chunk->bActive.AtomicSet(false);
//// 			GEngine->AddOnScreenDebugMessage(-1, 4, FColor::Green, FString("Debug. Delete chunk - ") + chunk->name, false);
//// 			if(LocationSync->get() == FVector::ZeroVector)
//// 				GEngine->AddOnScreenDebugMessage(-1, 4, FColor::Red, FString("Debug. Location - 000") , false);
//		}
		//int64 numInst = 0;
		//if (chunk->bActive)
		//{
		//	
		//	for (auto typeForest : chunk->Forest)
		//	{
		//		numInst += typeForest->NumBuiltInstances;
		//		
		//	}
		//	//GEngine->AddOnScreenDebugMessage(199, 4, FColor::Green, FString("Debug. Num instance - ") + FString::FromInt(numInst), false);
		//}

		/*debugString.Append(chunk->name + ", isActive = " + FString::FromInt(chunk->bActive) + ", bDelete = " + FString::FromInt(chunk->bDelete) +
			", bBuild = " + FString::FromInt(chunk->bBuilt) + ", BNewLevel = " + FString::FromInt(chunk->bNewLevel) + ", NumInstances =  " + FString::FromInt(numInst) + "  registredTime - " + FString::FromInt(chunk->buildTime) + "\n");
	*/
		//if (bOneActivation)
		//{
		//	bOneActivation = false;
		//	//GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Black, FString::Printf(TEXT("time reg - %i"), clock() - timett), false);
		//	break;
		//}
	}
	
	//GEngine->AddOnScreenDebugMessage(198, 4, FColor::Red, forestProcessor->getPosition().ToString(), false);
	//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Green, PlayerLocationCH.ToString() + "\n" + debugString, false);
	/*if (DeltaTime > 0.02)
		GEngine->AddOnScreenDebugMessage(-1, 4, FColor::Black, FString::Printf(TEXT("Delta = %f"), DeltaTime), false);*/
}


