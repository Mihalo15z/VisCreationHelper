#include "PerfomanceStats/VCHTimerCounter.h"

DECLARE_LOG_CATEGORY_CLASS(VCH_PerfomansceLog, Log, All);

FVCHTimeConter::FVCHTimeConter(const FString & InCounterInfo) :CounterInfo(InCounterInfo)
{
	StartCicles = FPlatformTime::Cycles();
}

FVCHTimeConter::~FVCHTimeConter()
{
	UE_LOG(VCH_PerfomansceLog, Log, TEXT("%s: time = %f"), *CounterInfo, FPlatformTime::ToMilliseconds(FPlatformTime::Cycles() - StartCicles));
}
