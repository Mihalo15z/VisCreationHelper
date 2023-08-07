// Fill out your copyright notice in the Description page of Project Settings.


#include "Foliage/FVCH_FoliageFunctions.h"
#include "Meshes/VCH_MeshesFunctions.h"
#include "InstancedFoliageActor.h"
#include "LandscapeProxy.h"
#include "Misc/FileHelper.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "PreparationData/FVCH_PreparationDataFunctions.h"
#include "Foliage/StaticForestSettings.h"
#include "FoliageType_InstancedStaticMesh.h"
#include "InstancedFoliage.h"
#include "FoliageEditUtility.h"


DECLARE_LOG_CATEGORY_CLASS(VCH_FoliageLog, Log, All);


struct FForestData
{
	const FForestTypeParams& Params;
	//TArray<FTransform> InstancesTrasfom;
	TArray<FFoliageInstance> Instances;

	FForestData() = delete;
	FForestData(const FForestTypeParams& InParams): Params(InParams)
	{
	}
};

using FForestDataArray = TArray<FForestData>;

void AddForestInstancesToIFA(const FForestDataArray& ForestData, AInstancedFoliageActor* IFA);

InstanceParams::InstanceParams(const FVCHMeshData & InMeshData, const TArray<FTransform>& InTransforms):Tarasforms(InTransforms)
{
	MaxDrawDistance = InMeshData.MaxCoolDistance;
	MinDrawDistance = InMeshData.MinCoolDistance;
	CollisionProfileName = FName(TEXT("BlockAllDynamic"));
	CollisionEnabledType = InMeshData.CollisionType;
	bCastDynamicShadow = InMeshData.bCastShadow/*= true*/;
	bCastStaticShadow = false;
	bEnableDensityScaling = false;
	bAffectDistanceFieldLighting = false;

	StaticMesh = InMeshData.GetMesh();
}

void FVCH_FoliageFunctions::AddInstancesForIFA(const InstanceParams & InParams, int32 & OutBossShot, int32 & OutNum, int32 & OutNumDelete, bool bAttachToComponent, bool bUseCalcLocation, bool bOnlyLandscape, ULevel * level, float UnifiedScale)
{
	for (auto* Level : GWorld->GetLevels())
	{
		AInstancedFoliageActor* IFA = AInstancedFoliageActor::GetInstancedFoliageActorForLevel(Level);
	}
	
}

FVector FVCH_FoliageFunctions::CalculateGroundPosition(const FVector & InLocation, AActor ** OutActor)
{
	return FVector();
}

FVector FVCH_FoliageFunctions::CalculateGroundPosition(const FVector & InLocation, UActorComponent ** OutComponent)
{
	
	return FVector();
}

void FVCH_FoliageFunctions::GenerateForest(const FString& HeightmapsPath, const FString& ForestMapsPath, const FString& WaterMaskPath)
{
	UWorld* WorldPtr = GWorld;
	check(WorldPtr);

	auto HeightMaps = FVCH_PreparationDataFunctions::GetAllHeightmaps(HeightmapsPath, 511);
	FVector EndOffset(0.f, 0.f, -2'000'000.f);

	auto ClearLandscapeLabelLambda = [](const FString& InName)
	{
		auto Result = InName.Replace(TEXT("LandscapeStreming"), TEXT(""));
		Result.ReplaceInline(TEXT("Landscape"), TEXT(""));
		return Result;
	};
	auto ForestSettings = GetDefault<UStaticForestSettings>();
	check(ForestSettings);

	// make forest data
	FForestDataArray ForestData;
	TArray<int32> ForestIndexes;
	{
		int32 index(0);
		for (const auto& ForestType : ForestSettings->ForestTypes)
		{
			ForestData.Add(FForestData(ForestType));
			for (int32 i = 0; i < ForestType.ChanceInst; ++i)
			{
				ForestIndexes.Add(index);
			}

			++index;
		}
	}
	if (ForestData.Num() == 0 || ForestIndexes.Num() == 0)
	{
		UE_LOG(VCH_FoliageLog, Warning, TEXT("Empty forest config"));
		return;
	}

	const int32 MaxIndexForestData = ForestIndexes.Num() - 1;

	for (TActorIterator<ALandscapeProxy> Iter(WorldPtr, ALandscapeProxy::StaticClass()); Iter; ++Iter)
	{
		if (ForestSettings->PlaceForestLevel == EPlaceForestLevel::LoacalLevelIFA)
		{
			// clear instances
			for (auto& Data : ForestData)
			{
				//Data.InstancesTrasfom.Empty();
				Data.Instances.Empty();
			}
		}

		ALandscapeProxy* Landscape = *Iter;
		if (Landscape)
		{
			UE_LOG(VCH_FoliageLog, Display, TEXT("%s - %s"), *Landscape->GetActorLabel(), *Landscape->GetActorLocation().ToString())
			// to do: check land name 
			auto LandName = ClearLandscapeLabelLambda(Landscape->GetActorLabel());
			if (LandName.IsEmpty())
			{
				UE_LOG(VCH_FoliageLog, Warning, TEXT("Bad landscape label - %s"), *Landscape->GetActorLabel());
				continue;
			}
			if (!HeightMaps.Contains(LandName))
			{
				continue;
			}
			auto& HeightMap = HeightMaps[LandName];
			// load forest mask
			auto PathToMask = ForestMapsPath / LandName + TEXT(".png");
			auto PathToWaterMask = WaterMaskPath / LandName + TEXT(".png");
			auto ImageData = FVCH_PreparationDataFunctions::LoadImage(PathToMask);
			auto WaterData = FVCH_PreparationDataFunctions::LoadImage(PathToWaterMask);
			if (ImageData.Num() == 0)
			{
				UE_LOG(VCH_FoliageLog, Warning, TEXT("Bad forest mask - %s"), *PathToMask);
				continue;
			}

			// to do: add work for different size textures;
			bool bCheckWater = ImageData.Num() == WaterData.Num();

			constexpr int32 NumChanels = 4;
			int32 ForestMaskResolution = static_cast<int32>(FMath::Sqrt(ImageData.Num() / NumChanels));
			int32 MaxIndexForestMask = ForestMaskResolution - 1;

			// calc size params
			FVector Origin;
			FVector Bounds;
			Landscape->GetActorBounds(true, Origin, Bounds);
			int32 PsevdoNumInstancesInLine = static_cast<int32>((Bounds.X * 2.f) / ForestSettings->Density);
			float ForestMaskPixelSize = (Bounds.X * 2.f) / static_cast<float>(ForestMaskResolution - 1);
			float HeightMapPixelSize = (Bounds.X * 2.f) / static_cast<float>(510);
			int32 MissCounter(0);
			for (int32 i = 0; i < PsevdoNumInstancesInLine; i++)
			{
				
				for (int32 j = 0; j < PsevdoNumInstancesInLine; j++)
				{
					if (FMath::FRand() < 0.4f)
					{
						continue;
					}
					float x(i * ForestSettings->Density);
					float y(j * ForestSettings->Density);

					float DeltaX = FMath::RandRange(0.f, ForestSettings->Density);
					float DeltaY = FMath::RandRange(0.f, ForestSettings->Density);

					int32 pixelFX(static_cast<int32>(x / ForestMaskPixelSize));
					int32 pixelFY(static_cast<int32>(y / ForestMaskPixelSize));

					constexpr int32 GChannelOffset = 1;
					int32  IndexInForestMask = (pixelFX + pixelFY * ForestMaskResolution) * NumChanels + GChannelOffset;

					if (pixelFX > (MaxIndexForestMask)
						|| pixelFY > (MaxIndexForestMask)
						|| (IndexInForestMask > ImageData.Num() - 1)
						|| (ImageData[IndexInForestMask] < 64))
					{
						continue;
					}
					else if (bCheckWater && WaterData[IndexInForestMask + 1] > 10)
					{
						continue;
					}


					// check HMap limit
					float X = x + DeltaX;
					float Y = y + DeltaY;
					check(!(X > Bounds.X * 2.f || Y > Bounds.X * 2.f));

					uint32 pixelHX(uint32((x + DeltaX) / HeightMapPixelSize));
					uint32 pixelHY(uint32((y + DeltaY) / HeightMapPixelSize));

					constexpr int32 ResolutinH = 511;
					constexpr float NormalizeValue = 256.f / (65'536 / 2 - 1);
					constexpr uint16 OffsetH = 65'536 / 2;
					constexpr float scaleCof = 528.125f;

					FVector A(
						(float)pixelHX * HeightMapPixelSize,
						(float)pixelHY * HeightMapPixelSize,
						(float)(HeightMap[pixelHX + pixelHY * ResolutinH] - OffsetH) * (scaleCof) * NormalizeValue);   // * 1437.890   scale Z
					FVector C(
						(float)(pixelHX + 1) * HeightMapPixelSize,
						(float)(pixelHY + 1) * HeightMapPixelSize,
						(float)(HeightMap[pixelHX + 1 + (pixelHY + 1) * ResolutinH] - OffsetH) * (scaleCof) * NormalizeValue);
					FVector B;
					if (((A.X * C.Y - C.X * A.Y) + (A.Y - C.Y) * X) / (A.X - C.X) > Y)
					{
						B = FVector(
							(float)(pixelHX)* HeightMapPixelSize,
							(float)(pixelHY + 1) * HeightMapPixelSize,
							(float)(HeightMap[pixelHX + (pixelHY + 1) * ResolutinH] - OffsetH) * (scaleCof) * NormalizeValue);
					}
					else
					{
						B = FVector(
							(float)(pixelHX + 1) * HeightMapPixelSize,
							(float)(pixelHY)* HeightMapPixelSize,
							(float)(HeightMap[pixelHX + 1 + pixelHY * ResolutinH] - OffsetH)  * (scaleCof) * NormalizeValue);
					}
					
					FPlane plane(A, B, C);
					const FVector Point1(X, Y, 10000.f);
					const FVector Point2(X, Y, 0.f);
					FVector treeLocation(FMath::LinePlaneIntersection(Point1, Point2, plane));

					int32 ForestDataIndex = ForestIndexes[FMath::RandRange(0, MaxIndexForestData)];
					auto& SelecForestData = ForestData[ForestDataIndex];
					//Inst.BaseComponent = Cast<UActorComponent>(Landscape->LandscapeComponents[0]);
					FFoliageInstance Inst;
					Inst.Location = treeLocation + Landscape->GetActorLocation();
					float ScaleInst = FMath::RandRange(SelecForestData.Params.MinScale, SelecForestData.Params.MaxScale);
					Inst.DrawScale3D = FVector(ScaleInst);

					if (SelecForestData.Params.bRandomRotation)
					{
						Inst.Rotation = FRotator(0.f, FMath::FRandRange(0.f, 180.f), 0.f);
					}
					SelecForestData.Instances.Add(Inst);
				}
			}

			if (ForestSettings->PlaceForestLevel == EPlaceForestLevel::LoacalLevelIFA)
			{
				auto Level = Landscape->GetLevel();
				check(Level);
				UE_LOG(VCH_FoliageLog, Display, TEXT("Place Forest to %s, miss = %i"), *Level->GetName(), MissCounter);
				auto IFA = AInstancedFoliageActor::GetInstancedFoliageActorForLevel(Level, true);
				AddForestInstancesToIFA(ForestData, IFA);
			}
		}

	}

	if (ForestSettings->PlaceForestLevel == EPlaceForestLevel::GloabalIFA)
	{
		AddForestInstancesToIFA(ForestData, AInstancedFoliageActor::GetInstancedFoliageActorForLevel(GWorld->PersistentLevel, true));
	}
}

void FVCH_FoliageFunctions::ClearAllIFA()
{
	check(GWorld);
	for (TActorIterator<AInstancedFoliageActor> Iter(GWorld, AInstancedFoliageActor::StaticClass()); Iter; ++Iter)
	{
		AInstancedFoliageActor* IFA = *Iter;
		if (IFA)
		{
			IFA->Destroy();
		}
	}
}

void AddForestInstancesToIFA(const FForestDataArray& ForestData, AInstancedFoliageActor* IFA)
{
	check(IFA);

	for (const auto ForestTypeData : ForestData)
	{
		UStaticMesh* Mesh = ForestTypeData.Params.MeshPath.LoadSynchronous();
		if (!Mesh)
		{
			UE_LOG(VCH_FoliageLog, Error, TEXT("Bad Mesh path"));
			continue;
		}
		auto FoliageType = IFA->GetLocalFoliageTypeForSource(Mesh);
		IFA->SetActorLocation(FVector::ZeroVector);
		FFoliageInfo* FoliageInfo = nullptr;
		if (FoliageType)
		{
			FoliageInfo = IFA->FindInfo(FoliageType);
		}
		else
		{
			FoliageInfo = IFA->AddMesh(Mesh, &FoliageType, nullptr);
			FoliageType->bCastDynamicShadow = true;
			FoliageType->bCastStaticShadow = false;
			FoliageType->bCastShadowAsTwoSided = false;
			FoliageType->Mobility = EComponentMobility::Static;
			FoliageType->CullDistance = { ForestTypeData.Params.MinDrawDistance, ForestTypeData.Params.MaxDrawDistance };
			//FFoliageEditUtility::SaveFoliageTypeObject(FoliageType);
		}
		check(FoliageType && FoliageInfo);

		TArray<const FFoliageInstance*> Insts;
		for (const FFoliageInstance& InstData : ForestTypeData.Instances)
		{
			Insts.Add(&InstData);
		}

		FoliageInfo->AddInstances(IFA, FoliageType, Insts);
	}

	IFA->MapRebuild();
	IFA->DetectFoliageTypeChangeAndUpdate();
}


