#pragma once
#include"CoreMinimal.h"

class VISCREATIONHELPER_API FVCHTimeConter
{
public:
	FVCHTimeConter() = delete;
	FVCHTimeConter(const FString& InCounterInfo);
	~FVCHTimeConter();
private:
	FString CounterInfo;
	int64 StartCicles;
};
