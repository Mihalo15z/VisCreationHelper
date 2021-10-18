// Fill out your copyright notice in the Description page of Project Settings.


#include "ForestManager/ThreadProcessingForest.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Math/UnrealMathUtility.h"
#include "ForestManager/FForestData.h"
#include "XMLParser.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Async/ParallelFor.h"
#include <ctime>
#include "Engine.h"
#include "GenericPlatform/GenericPlatformProcess.h"
//#include "PNGReader/PngReader.h"




#define TEST_FOREST 1
#define MAX_MAP_SIZE 73000000.0
//class PNGReader;

ThreadProcessingForest::ThreadProcessingForest(const float _minScale, const float _maxScale, const float _density, const float _ScaleCof/*, IImageWrapperModule& _ImageWrapperModule*//*, FCriticalSection &_SyncObject*/)
	: minScale(_minScale), maxScale(_maxScale), density(_density), scaleCof(_ScaleCof), ImageWrapperModule(FModuleManager::GetModuleChecked<IImageWrapperModule>(FName("ImageWrapper")))/*, SyncObject(&_SyncObject)*/
{
	PlayerPosition = FVector::ZeroVector;
}

ThreadProcessingForest::~ThreadProcessingForest()
{
	if (Thread.IsValid() && IsActive())
	{   
		Stop();
		WaitThread();
		//Thread->WaitForCompletion();
		Thread->Kill(true);	
	}
	else
	{
		if (Thread.IsValid())
		{
			Thread->Kill(true);
		}
	}
	Stop();
	WaitThread();
}

bool ThreadProcessingForest::Init()
{
	Stopping = false;
	return !Stopping;
}

uint32 ThreadProcessingForest::Run()
{
	FPlatformProcess::Sleep(10.f);
	while (!Stopping)
	{
		isFinishTakt = false;
		FPlatformProcess::Sleep(0.5f);
		FVector tempLocation(LocationSync->get());
		if(tempLocation.GetAbsMax() <  MAX_MAP_SIZE)
			PlayerPosition = tempLocation;
		checkActiveLevel();
		DeleteForest();
		checkAllLevels();
		//DebugsetDelete();
		//PlaceForestParallel();
		PlaceForest();
		//GEngine->AddOnScreenDebugMessage(195, 2, FColor::Blue, " Run() is Work", false);
		isFinishTakt = true;
	}
	return 0;
}

bool ThreadProcessingForest::IsActive() const
{
	return !Stopping;
}

void ThreadProcessingForest::Stop()
{
	Stopping = true;
}

bool ThreadProcessingForest::Start(TArray<UForestChunc*>& _Forest, /*const */TArray<FForestType>& forestType, TSharedPtr<FLocationSync> _LocationSync)
{
	//GEngine->AddOnScreenDebugMessage(0, 10, FColor::Red, FString("forestProcessor disable"), true);
	Forest = &_Forest;
	ForestType = &forestType;
	LocationSync = _LocationSync;
	if (!Thread)
	{
		FString path(FPaths::ProjectDir() + TEXT("OtherData/InfoLevels.xml"));
		//  парсинп хмл, мб стоит вынести из этого потока
		getLevelData(path, levelData);
		
		FString Filter = TEXT("*.png");
		FString Mask = FPaths::ProjectDir() + TEXT("OtherData/ForestMask/") + Filter;
		IFileManager::Get().FindFiles(NamesMask, *Mask, true, false);
		Filter = TEXT("*.raw");
		Mask = FPaths::ProjectDir() + TEXT("OtherData/HieghtMaps/") + Filter;
		IFileManager::Get().FindFiles(NamesHMAp, *Mask, true, false);

		if (FFileHelper::LoadFileToArray(randomData, *(FPaths::ProjectDir() + TEXT("OtherData/data1.rand"))) 
			&& Forest->Num() != 0 
			&& levelData.Num() != 0)
		{
			FString ThreadName = TEXT("ThreadForestGenerated");
			Thread = TUniquePtr<FRunnableThread>(FRunnableThread::Create(this, *ThreadName/*, 4*1024, TPri_Normal, 6*/));
			
			//std::thread thr(threadFunction, this);
			//thr.detach();
		}
		else
			return false;
	}
	return true;
}

const TArray<uint16> ThreadProcessingForest::getHeightMapData(const FString& path)
{
	TArray<uint16> Data;
	if (path.EndsWith(TEXT(".png")))
	{
		if (Data.Num() == 0)
		{
			Data.Empty();

			TArray<uint8> ImportData;
			if (FFileHelper::LoadFileToArray(ImportData, *path, FILEREAD_Silent))
			{
				TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

				TArray64<uint8> RawData;
				if (ImageWrapper->SetCompressed(ImportData.GetData(), ImportData.Num()) &&
					ImageWrapper->GetWidth() == 511 && ImageWrapper->GetHeight() == 511)
				{
					if (ImageWrapper->GetBitDepth() <= 8)
					{
						ImageWrapper->GetRaw(ERGBFormat::Gray, 8, RawData);

						if (ensure(RawData.Num() == 511 * 511))
						{
							Data.Empty(511 * 511);
							Data.AddUninitialized(511 * 511);

							for (int32 i = 0; i < RawData.Num(); i++)
							{
								Data[i] = (RawData)[i] * 0x101; // Expand to 16-bit
							}
						}
					}
					else
					{
						ImageWrapper->GetRaw(ERGBFormat::Gray, 16, RawData);

						if (ensure(RawData.Num() == 511 * 511 * 2))
						{
							Data.Empty(511 * 511);
							Data.AddUninitialized(511 * 511);
							FMemory::Memcpy(Data.GetData(), RawData.GetData(), RawData.Num());
						}
					}
				}
			}
		}
	}
	else
	{
		if (511 * 511 != Data.Num())
		{
			Data.Empty();

			TArray<uint8> ImportData;
			if (FFileHelper::LoadFileToArray(ImportData, *path, FILEREAD_Silent) &&
				ImportData.Num() == 511 * 511 * 2)
			{
				Data.Empty(511 * 511);
				Data.AddUninitialized(511 * 511);
				FMemory::Memcpy(Data.GetData(), ImportData.GetData(), ImportData.Num());
			}
		}
	}
	return Data;
}

const TArray64<uint8> ThreadProcessingForest::getForestMaskData(const FString& path)
{
	TArray64<uint8> data;
	if (FFileHelper::LoadFileToArray(data, *path, FILEREAD_Silent))
	{
		TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
		if (ImageWrapper->SetCompressed(data.GetData(), data.Num()) && (ImageWrapper->GetRaw(ERGBFormat::RGBA, 8, data)))
		{
			return data;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s"), *("bad file: " + path));
		}
	}
	return data;
}

void ThreadProcessingForest::DebugsetDelete()
{
	FVector position(PlayerPosition);  // ненужный конструктор копировани!
	for (auto chunc : *Forest)
	{
		if (chunc->bDelete)
		{
			chunc->bDelete=(true);
			chunc->bBuilt=(false);
		}
	}
}

void  ThreadProcessingForest::getLevelData(FString& path, TArray<FLevelData>& OutlevelData)
{
	FXmlFile file;
	if (!file.LoadFile(path))
		return;
	FXmlNode* levelsNode(file.GetRootNode()->FindChildNode("Levels"));
	for (auto levelInfo : levelsNode->GetChildrenNodes())
	{
		FLevelData data;
		data.name = levelInfo->GetAttribute("name");
		data.origin.InitFromString(levelInfo->GetAttribute("origin"));
		data.bound.InitFromString(levelInfo->GetAttribute("bound"));
		data.location.InitFromString(levelInfo->GetAttribute("location"));
		OutlevelData.Add(data);
	}
	file.Clear();
	return;
}

void ThreadProcessingForest::checkPlayerPoaition()
{
	for (auto data : levelData)
	{
		for (auto info : levelData)
		{
			if (PlayerPosition.X < info.origin.X + info.bound.X * BoundScaleCoff &&
				PlayerPosition.X > info.origin.X - info.bound.X * BoundScaleCoff &&
				PlayerPosition.Y > info.origin.Y - info.bound.Y * BoundScaleCoff &&
				PlayerPosition.Y < info.origin.Y + info.bound.Y * BoundScaleCoff)
			{
				FLevelData tempData(info);
			}
		}
	}
	return;
}

void ThreadProcessingForest::checkActiveLevel()
{
	FVector position(PlayerPosition);  // ненужный конструктор копировани!
	for (auto chunc : *Forest)
	{
		if (!(chunc->bActive) || chunc->bDelete)								//  проверка  только активных уровней
			continue;
		for (auto info : levelData)
		{
			if (chunc->name.Equals(info.name))
			{
				if ((position.X < (info.origin.X + (info.bound.X * BoundScaleCoff))) &&
					(position.X > (info.origin.X - (info.bound.X * BoundScaleCoff))) &&
					(position.Y < (info.origin.Y + (info.bound.Y * BoundScaleCoff))) &&
					(position.Y > (info.origin.Y - (info.bound.Y * BoundScaleCoff))))
				{
					//GEngine->AddOnScreenDebugMessage(-1, 7, FColor::Red, FString("Location - x  = ") + position.ToString() + " bound - " + info.origin.ToString()+ "  " + info.name, false);
					break;
				}
				else
				{
					chunc->bDelete=(true);
					chunc->bBuilt=(false);
					chunc->bNewLevel=(false);
					//GEngine->AddOnScreenDebugMessage(-1, 7, FColor::Yellow, FString("Location - x  = ") + position.ToString() + " bound - "  + info.origin.ToString()+ "  " + info.name, false);
					break;
				
				}
			}
		}
	}
}

 void ThreadProcessingForest::DeleteForest()
 {
	 for (auto& chunc : *Forest)
	 {
		 if (chunc->bDelete && !(chunc->bActive) /*&& !chunc->isBuilding*/)
		 {
			
			 for (auto typeForest : chunc->Forest)
			 {
				 typeForest->ClearInstances();
			 } 
			 chunc->name = "None";
			 chunc->bBuilt = false;
			 chunc->bNewLevel = false;
			 chunc->bDelete = false;
		 }
	 }
 }

 void ThreadProcessingForest::checkAllLevels()
 {
	 FVector position(PlayerPosition);
	 for (auto& info : levelData)
	 {
		 if ((position.X < (info.origin.X + info.bound.X * BoundScaleCoff)) &&
			 (position.X > (info.origin.X - info.bound.X * BoundScaleCoff)) &&
			 (position.Y > (info.origin.Y - info.bound.Y * BoundScaleCoff)) &&
			 (position.Y < (info.origin.Y + info.bound.Y * BoundScaleCoff)))
		 {
			 //  проверка на содерржания левела в обрабатываемых данных
			 bool isConteinLevel(false);
			 for (auto chunc : *Forest)
			 {
				 if (chunc->name.Equals(info.name))
				 {
					 //GEngine->AddOnScreenDebugMessage(120, 10, FColor::Red, info.name + "isActive", false);
					 isConteinLevel = true;
					 break;
				 }					 
			 }
			 if (isConteinLevel)
				 continue;
			 // end
			 for (auto& chunc : *Forest)
			 {
				 if (IsValid(chunc) && chunc->name.Equals("None"))
				 {
					 chunc->name = info.name;
					 chunc->bNewLevel=(true);
					 // попытаюь работать с очередями
					 FProcessingData data;
					 data.chunc = chunc;
					 data.levelData = &info;
					 placeQueue.Enqueue(data);
						// GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString("add Queue alement - ") + info.name, false);
					 break;
				 }	 
			 }
		 }
	 }
 }

// void Build(TArray<FClusterNode>& ClusterTree, TArray<int32>& SortedInstances, TArray<int32>& InstanceReorderTable, TArray<FMatrix>& InstanceTransforms, int32& OcclusionLayerNumNodes);
#pragma optimize( "", off )
 /*FORCEINLINE*/ void ThreadProcessingForest::PlaceForest()
 {
	 //placeQueue
	while (!placeQueue.IsEmpty())
	{
		// GEngine->AddOnScreenDebugMessage(0, 30, FColor::Green, FString("add Queue alement"), false);
		 FProcessingData data;
		 placeQueue.Dequeue(data);
		 if (!data.chunc || !data.levelData)
			 continue;

		 FForestData forestData;
		 const FString pathHMap(FPaths::ProjectDir() + "OtherData/HieghtMaps/" + data.chunc->name + ".raw");
		 const FString pathFMask(FPaths::ProjectDir() + "OtherData/ForestMask/" + data.chunc->name + ".png");

		 if (!(NamesMask.Contains(data.chunc->name + ".png")) && !(NamesHMAp.Contains(data.chunc->name + ".raw")))
		 {
			 data.chunc->bNewLevel = false;
			 data.chunc->name = "None";
			 continue;
		 }

		 uint64 timeB(clock());
		// UE_LOG(LogTemp, Error, TEXT("Start Load data fo level %s"), *data.chunc->name);
		 forestData.heightMap = getHeightMapData(pathHMap);	// загрузка карты высот и маски леса
		 forestData.forestMask = getForestMaskData(pathFMask);
		 FPlatformProcess::Sleep(0.0001);
		// UE_LOG(LogTemp, Error, TEXT("End Load data fo level %s"), *data.chunc->name);


		 //  при отсутвии масок  переводим в статус свободного чанка;
		 if (forestData.heightMap.Num() == 0 || forestData.forestMask.Num() == 0)
		 {
			 data.chunc->bNewLevel = false;
			 data.chunc->name = "None";
			 continue;
		 }
		// data.chunc->isBuilding = true;
		 //  расчет основных параметров для генерации
		 forestData.hMapResolution = int32(FMath::Sqrt(forestData.heightMap.Num()));
		 forestData.fMaskResolution = int32(FMath::Sqrt(forestData.forestMask.Num() / 4 ));
		 forestData.hMapPixelSize = (data.levelData->bound.X * 2.f) / (float)(forestData.hMapResolution - 1);
		 forestData.fMaskPixelSize = (data.levelData->bound.X * 2.f) / (float)(forestData.fMaskResolution - 1);
		 forestData.psevdoNumInstance = (int32)((data.levelData->bound.X * 2.f) / density); //  количество интсансов в одном ряду или столбце

		// UE_LOG(LogTemp, Error, TEXT("Check param fo level %s"), *data.chunc->name);


		FTransform transform;
		transform.SetRotation(FQuat(FRotator::ZeroRotator));
		transform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));
		transform.SetLocation(FVector(-10000.f, -10000.f, 10000.f)); //  на всякий случай иницилизируем не в 0 локаци
		//  выставляем флаг строительства, что обеспечить безопасный доступ к памяти
		//for (auto& Hismc : data.chunc->Forest)
		//{
		//	Hismc->bIsAsyncBuilding = true;
		//}
		
		for (int32 i = 0; i < forestData.psevdoNumInstance; i++)
		{
			 float x(i * density);
			 for (int32 j = 0; j < forestData.psevdoNumInstance; j++)
			 {
				 float y(j * density);
				 // расчет индекса для рандома;
				 int64 index;
				 if ((i + j * forestData.psevdoNumInstance) > randomData.Num() - 1)
				 {
					 index = ((i + j * forestData.psevdoNumInstance) % randomData.Num());
				 }
				 else
				 {
					 index = ((i + j * forestData.psevdoNumInstance));
				 }
				 //прорердить на 25 процентов
				 /*if ((index & 3) == 0)
				 {
					continue;
				 }*/
					 

				 float X = x + density * (float)randomData[index] / 255;
				 float Y = y + density * (float)randomData[randomData.Num() - index - 1] / 255;

				 //проверяем на наличие леса в маске
				 int32 pixelFX(int32(x / forestData.fMaskPixelSize));
				 int32 pixelFY(int32(y / forestData.fMaskPixelSize));

				 if (pixelFX > (forestData.fMaskResolution - 2) 
					 || pixelFY > (forestData.fMaskResolution - 2) 
					 || (pixelFX + pixelFY * forestData.fMaskResolution) * 4 + (int32)MaskChannel::G > forestData.forestMask.Num() - 1 
					 || !(forestData.forestMask[(pixelFX + pixelFY * forestData.fMaskResolution) * 4 + (int32)MaskChannel::G] > 0))
					 continue;

				 /*if (!(forestData.forestMask[(pixelFX + pixelFY * forestData.fMaskResolution) * 4 + (int32)MaskChannel::G] > 0))
					 continue;*/
				 int32 point((randomData[index]) % data.chunc->Forest.Num());
				 uint8 typeF(forestData.forestMask[(pixelFX + pixelFY * forestData.fMaskResolution) * 4 + (int32)MaskChannel::G]);
				 for (const auto& forestT : *ForestType)
				 {
					 if (forestT.minRange <= typeF &&  forestT.maxRange >= typeF)
					 {
						 uint32 tempIndex(forestT.GetIndex(randomData[index]));
						 if (tempIndex != -1)
						 {
							 point = tempIndex;
							 break;
						 }
						 else
						 {
							 continue;  //  при отсутвии индекса пропускаем итерацию
						 }
					 }
				 }
				 // проверяем не выходим ли за карту высот
				 uint32 pixelHX(uint32(X / forestData.hMapPixelSize));
				 uint32 pixelHY(uint32(Y / forestData.hMapPixelSize));
				 /*if ((int32)pixelHX > forestData.hMapResolution - 2 || (int32)pixelHY > forestData.hMapResolution - 2)
					 continue;*/

				 //  определяем к какому треугольнику относится точка(ABCD  = ABC + ACD)
// 				 FVector A((float)pixelHX * forestData.hMapPixelSize, (float)pixelHY * forestData.hMapPixelSize, (float)((forestData.heightMap[pixelHX + pixelHY * forestData.hMapResolution] - (32768)) * (-scaleCof) * 1.4453125));   /// * 1437.890   scale Z
// 				 FVector C((float)(pixelHX + 1) * forestData.hMapPixelSize, (float)(pixelHY + 1) * forestData.hMapPixelSize, (float)((forestData.heightMap[pixelHX + 1 + (pixelHY + 1) * forestData.hMapResolution] - (32768 )) * (-scaleCof) * 1.4453125));//84660
// 				 FVector B;
// 				 if (((A.X * C.Y - C.X * A.Y) + (A.Y - C.Y) * X) / (A.X - C.X) > Y)
// 					 B = FVector((float)(pixelHX)* forestData.hMapPixelSize, (float)(pixelHY + 1) * forestData.hMapPixelSize, (float)((forestData.heightMap[pixelHX + (pixelHY + 1) * forestData.hMapResolution] - (32768 )) * (-scaleCof) * 1.4453125));
// 				 else
// 					 B = FVector((float)(pixelHX + 1) * forestData.hMapPixelSize, (float)(pixelHY)* forestData.hMapPixelSize, (float)((forestData.heightMap[pixelHX + 1 + pixelHY * forestData.hMapResolution] - (32768 )) * (-scaleCof) * 1.4453125));
// 				 

				 constexpr float NormalizeValue = 256.f / (65'536 / 2 - 1);
				 constexpr uint16 OffsetH = 65'536 / 2;

				 FVector A(
					 (float)pixelHX * forestData.hMapPixelSize,
					 (float)pixelHY * forestData.hMapPixelSize,
					 (float)(forestData.heightMap[pixelHX + pixelHY * forestData.hMapResolution] - OffsetH) * (scaleCof) * NormalizeValue);   // * 1437.890   scale Z
				 FVector C(
					 (float)(pixelHX + 1) * forestData.hMapPixelSize,
					 (float)(pixelHY + 1) * forestData.hMapPixelSize,
					 (float)(forestData.heightMap[pixelHX + 1 + (pixelHY + 1) * forestData.hMapResolution] - OffsetH) * (scaleCof)* NormalizeValue);
				 FVector B;
				 if (((A.X * C.Y - C.X * A.Y) + (A.Y - C.Y) * X) / (A.X - C.X) > Y)
				 {
					 B = FVector(
						 (float)(pixelHX)* forestData.hMapPixelSize,
						 (float)(pixelHY + 1) * forestData.hMapPixelSize,
						 (float)(forestData.heightMap[pixelHX + (pixelHY + 1) * forestData.hMapResolution] - OffsetH) * (scaleCof)* NormalizeValue);
				 }
				 else
				 {
					 B = FVector(
						 (float)(pixelHX + 1) * forestData.hMapPixelSize,
						 (float)(pixelHY)* forestData.hMapPixelSize,
						 (float)(forestData.heightMap[pixelHX + 1 + pixelHY * forestData.hMapResolution] - OffsetH)  * (scaleCof)* NormalizeValue);
				 }
				 float scale(((float)randomData[index] / 255.0f) * (maxScale - minScale) + minScale);
				 transform.SetScale3D(FVector(scale, scale, scale));
				 transform.SetRotation(FQuat(FRotator(0.f, randomData[index], 0.f)));
				 FPlane plane(A, B, C);
				 const FVector Point1(X, Y, 10000.f);
				 const FVector Point2(X, Y, 0.f);
				 FVector treeLocation(FMath::LinePlaneIntersection(Point1, Point2, plane));
				 
				 transform.SetLocation(data.levelData->location + treeLocation);
				// InstanceTransforms.Add(transform)
				 data.chunc->Forest[point]->AddInstanceWorldSpace(transform);
				// data.chunc->Forest[point]->AddInstance(transform)
				 FPlatformProcess::Sleep(0.000001f);
			 }
		}
		data.chunc->buildTime = clock() - timeB;
		//UE_LOG(LogTemp, Error, TEXT("finish calc location fo level %s"), *data.chunc->name);
		// build forest
		for (auto Hismc : data.chunc->Forest)
		{
			//int32 NumInstances = InstanceTransforms.Num();
			//if (NumInstances)
			//{
			//	FBox MeshBox = Hismc->GetStaticMesh()->GetBoundingBox();
			//	TArray<int32> SortedInstances;
			//	TArray<int32> InstanceReorderTable;
			//	TArray<float> InstanceCustomDataDummy;
			//	UHierarchicalInstancedStaticMeshComponent::BuildTreeAnyThread(InstanceTransforms, InstanceCustomDataDummy, 0, MeshBox, ClusterTree, SortedInstances, InstanceReorderTable, OutOcclusionLayerNum, DesiredInstancesPerLeaf, false);

			//	// in-place sort the instances

			//	for (int32 FirstUnfixedIndex = 0; FirstUnfixedIndex < NumInstances; FirstUnfixedIndex++)
			//	{
			//		int32 LoadFrom = SortedInstances[FirstUnfixedIndex];
			//		if (LoadFrom != FirstUnfixedIndex)
			//		{
			//			check(LoadFrom > FirstUnfixedIndex);
			//			InstanceBuffer.SwapInstance(FirstUnfixedIndex, LoadFrom);

			//			int32 SwapGoesTo = InstanceReorderTable[FirstUnfixedIndex];
			//			check(SwapGoesTo > FirstUnfixedIndex);
			//			check(SortedInstances[SwapGoesTo] == FirstUnfixedIndex);
			//			SortedInstances[SwapGoesTo] = LoadFrom;
			//			InstanceReorderTable[LoadFrom] = SwapGoesTo;

			//			InstanceReorderTable[FirstUnfixedIndex] = FirstUnfixedIndex;
			//			SortedInstances[FirstUnfixedIndex] = FirstUnfixedIndex;
			//		}
			//	}
			//}


			//UE_LOG(LogTemp, Error, TEXT("finish calc location fo level %d"), Hismc->PerInstanceSMData.Num());
			//if (0 == Hismc->PerInstanceSMData.Num())
			//	continue;
			//int32 Num = Hismc->PerInstanceSMData.Num();
			//TArray<FMatrix> InstanceTransforms;
			//InstanceTransforms.AddUninitialized(Num);
		
			//for (int32 Index = 0; Index < Num; Index++)
			//{
			//	InstanceTransforms[Index] = Hismc->PerInstanceSMData[Index].Transform;
			//}

			//Hismc->ClusterTreePtr = MakeShareable(new TArray<FClusterNode>);
			//TArray<FClusterNode>& ClusterTree = *(Hismc->ClusterTreePtr);
			//TArray<FClusterNode> ClusterNode;
			//TArray<int32> SortedInstances;
			//TArray<int32> InstanceReorderTable;
			//int32 OcclusionLayerNumNodes(16);
			//FBox box = Hismc->GetStaticMesh()->GetBoundingBox();

			//Build(ClusterTree, SortedInstances, InstanceReorderTable, InstanceTransforms, OcclusionLayerNumNodes);
			FPlatformProcess::Sleep(0.0001f);
			//check(false);
			Hismc->BuildTreeIfOutdated(true, true);
			//Hismc->BuildTreeAnyThread
			//Hismc->BuildTreeAnyThread(InstanceTransforms, box, ClusterTree/*ClusterNode*/ ,SortedInstances, InstanceReorderTable, OcclusionLayerNumNodes, Hismc->DesiredInstancesPerLeaf(),);
			FPlatformProcess::Sleep(0.0001f);
			//Hismc->SortedInstances = MoveTemp(SortedInstances);
			//Hismc->InstanceReorderTable = MoveTemp(InstanceReorderTable);
			//Hismc->OcclusionLayerNumNodes = OcclusionLayerNumNodes;
			//Hismc->BuiltInstanceBounds = (ClusterTree.Num() > 0 ? FBox(ClusterTree[0].BoundMin, ClusterTree[0].BoundMax) : FBox(EForceInit::ForceInitToZero));
			//Hismc->NumBuiltInstances = Hismc->SortedInstances.Num();
			//Hismc->CacheMeshExtendedBounds = Hismc->GetStaticMesh()->GetBounds();
			////Hismc->RemovedInstances.Empty();
			//Hismc->UnbuiltInstanceBounds.Init();
			//Hismc->UnbuiltInstanceBoundsList.Empty();
			////Hismc->UnbuiltInstanceIndexList.Empty();

			//Hismc->ReleasePerInstanceRenderData();
			//Hismc->InitPerInstanceRenderData(true);
			//Hismc->MarkRenderStateDirty();
		}
		
		//UE_LOG(LogTemp, Error, TEXT("Finish build fo level %s"), *data.chunc->name);
		 FPlatformProcess::Sleep(0.01f);
		
		 data.chunc->bNewLevel=(false);
		 data.chunc->bBuilt=(true);
		 data.chunc->bDelete=(false);
		// data.chunc->isBuilding = false;
		
	}

 }
#pragma optimize( "", on )

 void ThreadProcessingForest::PlaceForestParallel()
 {
	 //placeQueue
	 TArray<FProcessingData> workData;
	 while (!placeQueue.IsEmpty())
	 {
		 FProcessingData data;
		 placeQueue.Dequeue(data);
		 if (!data.chunc || !data.levelData)
			 continue;
		 workData.Add(data);
	 }
	 if (workData.Num() == 0)
		 return;
	 uint64 TTT(clock());
	 ParallelFor(workData.Num(), [&](int32 NumberDATA)
	 {
		 //workData[index];
		 const FString pathHMap(FPaths::ProjectDir() + TEXT("OtherData/HieghtMaps/") + workData[NumberDATA].chunc->name + TEXT(".raw"));
		 const FString pathFMask(FPaths::ProjectDir() + TEXT("OtherData/ForestMask/") + workData[NumberDATA].chunc->name + TEXT(".png"));

		 if (!(NamesMask.Contains(workData[NumberDATA].chunc->name + TEXT(".png"))) && !(NamesHMAp.Contains(workData[NumberDATA].chunc->name + TEXT(".raw"))))
		 {
			 workData[NumberDATA].chunc->bNewLevel = false;
			 workData[NumberDATA].chunc->name = "None";
			 return;
		 }
		 FForestData forestData;
		 /*for (int x = 0; x < 3000; ++x)
		 {
			 forestData.heightMap.Empty();
			 forestData.heightMap = getHeightMapData(pathHMap);
		 }*/
		 forestData.heightMap = getHeightMapData(pathHMap);	// загрузка карты высот и маски леса
		 forestData.forestMask = getForestMaskData(pathFMask);

#if !TEST_FOREST
		 if (forestData.heightMap.Num() == 0 || forestData.forestMask.Num() == 0)
		 {
			 workData[NumberDATA].chunc->bNewLevel = false;
			 workData[NumberDATA].chunc->name = TEXT("None");
			 return;
		 }
		 forestData.hMapResolution = int32(FMath::Sqrt(forestData.heightMap.Num()));
		 forestData.fMaskResolution = int32(FMath::Sqrt(forestData.forestMask.Num() / 4));
		 forestData.hMapPixelSize = (workData[NumberDATA].levelData->bound.X * 2.f) / (float)(forestData.hMapResolution - 1);
		 forestData.fMaskPixelSize = (workData[NumberDATA].levelData->bound.X * 2.f) / (float)(forestData.fMaskResolution - 1);
		 forestData.psevdoNumInstance = (int32)((workData[NumberDATA].levelData->bound.X * 2.f) / density); //  количество интсансов в одном ряду или столбце

																						  // UE_LOG(LogTemp, Error, TEXT("Check param fo level %s"), *data.chunc->name);


		 FTransform transform;
		 transform.SetRotation(FQuat(FRotator::ZeroRotator));
		 transform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));
		 transform.SetLocation(FVector(-10000f, -10000f, 10000f)); //  на всякий случай иницилизируем не в 0 локаци
		 //  выставляем флаг строительства, что обеспечить безопасный доступ к памяти
		 for (auto Hismc : workData[NumberDATA].chunc->Forest)
		 {
			 Hismc->bIsAsyncBuilding = true;
		 }

		 for (int32 i = 0; i < forestData.psevdoNumInstance; i++)
		 {
			 float x(i * density);
			 for (int32 j = 0; j < forestData.psevdoNumInstance; j++)
			 {
				 float y(j * density);
				 // расчет индекса для рандома;
				 int64 indexR;
				 if ((i + j * forestData.psevdoNumInstance) > randomData.Num() - 1)
				 {
					 indexR = ((i + j * forestData.psevdoNumInstance) % randomData.Num());
				 }
				 else
				 {
					 indexR = ((i + j * forestData.psevdoNumInstance));
				 }

				 float X = x + density * (float)randomData[indexR] / 255;
				 float Y = y + density * (float)randomData[randomData.Num() - indexR - 1] / 255;

				 //проверяем на наличие леса в маске
				 int32 pixelFX(int32(x / forestData.fMaskPixelSize));
				 int32 pixelFY(int32(y / forestData.fMaskPixelSize));

				 if (pixelFX > (forestData.fMaskResolution - 2) || pixelFY > (forestData.fMaskResolution - 2) ||
					 (pixelFX + pixelFY * forestData.fMaskResolution) * 4 + (int32)MaskChannel::G > forestData.forestMask.Num() - 1)
					 continue;

				 if (!(forestData.forestMask[(pixelFX + pixelFY * forestData.fMaskResolution) * 4 + (int32)MaskChannel::G] > 0))
					 continue;

				 // проверяем не выходим ли за карту высот
				 uint32 pixelHX(uint32(X / forestData.hMapPixelSize));
				 uint32 pixelHY(uint32(Y / forestData.hMapPixelSize));
				 if ((int32)pixelHX > forestData.hMapResolution - 2 || (int32)pixelHY > forestData.hMapResolution - 2)
					 continue;

				 //  определяем к какому треугольнику относится точка(ABCD  = ABC + ACD)
				 FVector A((float)pixelHX * forestData.hMapPixelSize, (float)pixelHY * forestData.hMapPixelSize, (float)((forestData.heightMap[pixelHX + pixelHY * forestData.hMapResolution] - (32768)) * (-scaleCof) * 1.4453125));   /// * 1437.890   scale Z
				 FVector C((float)(pixelHX + 1) * forestData.hMapPixelSize, (float)(pixelHY + 1) * forestData.hMapPixelSize, (float)((forestData.heightMap[pixelHX + 1 + (pixelHY + 1) * forestData.hMapResolution] - (32768)) * (-scaleCof) * 1.4453125));//84660
				 FVector B;
				 if (((A.X * C.Y - C.X * A.Y) + (A.Y - C.Y) * X) / (A.X - C.X) > Y)
					 B = FVector((float)(pixelHX)* forestData.hMapPixelSize, (float)(pixelHY + 1) * forestData.hMapPixelSize, (float)((forestData.heightMap[pixelHX + (pixelHY + 1) * forestData.hMapResolution] - (32768)) * (-scaleCof) * 1.4453125));
				 else
					 B = FVector((float)(pixelHX + 1) * forestData.hMapPixelSize, (float)(pixelHY)* forestData.hMapPixelSize, (float)((forestData.heightMap[pixelHX + 1 + pixelHY * forestData.hMapResolution] - (32768)) * (-scaleCof) * 1.4453125));
				 int32 point((randomData[indexR]) % workData[NumberDATA].chunc->Forest.Num());
				 float scale(((float)randomData[indexR] / 255.0) * (maxScale - minScale) + minScale);
				 transform.SetScale3D(FVector(scale, scale, scale));
				 transform.SetRotation(FQuat(FRotator(0, randomData[indexR], 0)));
				 FPlane plane(A, B, C);
				 const FVector Point1(X, Y, 10000.f);
				 const FVector Point2(X, Y, 0.f);
				 FVector treeLocation(FMath::LinePlaneIntersection(Point1, Point2, plane));
				 transform.SetLocation(workData[NumberDATA].levelData->location - treeLocation /*- FVector(UGameplayStatics::GetWorldOriginLocation(Hismc->getOwner()).X, UGameplayStatics::GetWorldOriginLocation(Hismc->getOwner()).Y, 0)*/);
				 workData[NumberDATA].chunc->Forest[point]->AddInstance(transform);
			 }
		 }

		 //UE_LOG(LogTemp, Error, TEXT("finish calc location fo level %s"), *data.chunc->name);
		 // build forest
		 for (auto Hismc : workData[NumberDATA].chunc->Forest)
		 {
			 //UE_LOG(LogTemp, Error, TEXT("finish calc location fo level %d"), Hismc->PerInstanceSMData.Num());
			 if (0 == Hismc->PerInstanceSMData.Num())
				 continue;
			 int32 Num = Hismc->PerInstanceSMData.Num();
			 TArray<FMatrix> InstanceTransforms;
			 InstanceTransforms.AddUninitialized(Num);

			 for (int32 Index = 0; Index < Num; Index++)
			 {
				 InstanceTransforms[Index] = Hismc->PerInstanceSMData[Index].Transform;
			 }
			// uint64 hh(clock());
			 Hismc->ClusterTreePtr = MakeShareable(new TArray<FClusterNode>);
			 TArray<FClusterNode>& ClusterTree = *(Hismc->ClusterTreePtr);
			 TArray<FClusterNode> ClusterNode;
			 TArray<int32> SortedInstances;
			 TArray<int32> InstanceReorderTable;
			 int32 OcclusionLayerNumNodes(0);
			 FBox box = Hismc->StaticMesh->GetBoundingBox();

			 ///////////Build(ClusterTree, SortedInstances, InstanceReorderTable, InstanceTransforms, OcclusionLayerNumNodes);

			 Hismc->BuildTreeAnyThread(InstanceTransforms, box, ClusterTree/*ClusterNode*/, SortedInstances, InstanceReorderTable, OcclusionLayerNumNodes, Hismc->DesiredInstancesPerLeaf());

			 Hismc->SortedInstances = MoveTemp(SortedInstances);
			 Hismc->InstanceReorderTable = MoveTemp(InstanceReorderTable);
			 Hismc->OcclusionLayerNumNodes = OcclusionLayerNumNodes;
			 Hismc->BuiltInstanceBounds = (ClusterTree.Num() > 0 ? FBox(ClusterTree[0].BoundMin, ClusterTree[0].BoundMax) : FBox(EForceInit::ForceInitToZero));
			 Hismc->NumBuiltInstances = Hismc->SortedInstances.Num();
			 Hismc->CacheMeshExtendedBounds = Hismc->GetStaticMesh()->GetBounds();
			// Hismc->RemovedInstances.Empty();
			 Hismc->UnbuiltInstanceBounds.Init();
			 Hismc->UnbuiltInstanceBoundsList.Empty();
			 Hismc->bIsAsyncBuilding = false;
			
			 
			 //Hismc->MarkRenderStateDirty();
			 //Hismc->UnbuiltInstanceBounds.Init();
			 Hismc->ReleasePerInstanceRenderData();
			 Hismc->InitPerInstanceRenderData();
			// Hismc->MarkRenderStateDirty();
			 //GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Red, FString::Printf(TEXT("time reg - %i"), clock() - hh), true);
			// if (IsInGameThread())
				// GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Red, FString::Printf(TEXT("time reg +++++++++++++++++++++++++++++++%i"), clock() - hh), true);
		 }

		 //UE_LOG(LogTemp, Error, TEXT("Finish build fo level %s"), *data.chunc->name);
		// FPlatformProcess::Sleep(0.1);
#endif

#if !TEST_FOREST
		 workData[NumberDATA].chunc->bNewLevel=(false);
		 workData[NumberDATA].chunc->bBuilt=(true);
		 workData[NumberDATA].chunc->bDelete=(false);
#else
		workData[NumberDATA].chunc->bDelete=(true);
#endif
	 }, false);
	 if(!IsInGameThread())
		GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Orange, FString::Printf(TEXT("time reg - %i"), clock() - TTT), true);
 }


 


