// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FVisCreationHelperModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command. */
	void PluginButtonClicked();
	
private:

	void RegisterMenus();


private:
	TSharedPtr<class FUICommandList> PluginCommands;
};

struct  VISCREATIONHELPER_API FDoubleVect2
{
	double X;
	double Y;
	//double& Lat() const { return X; }
	//double& Lon() const { return Y; }

	constexpr FDoubleVect2() :X(0.0), Y(0.0)
	{};

	constexpr FDoubleVect2(double x, double y) :X(x), Y(y)
	{};

	constexpr FDoubleVect2(const FDoubleVect2&) = default;
	constexpr FDoubleVect2(FDoubleVect2&&) = default;
	FDoubleVect2& operator=(const FDoubleVect2&) = default;
	FDoubleVect2& operator=(FDoubleVect2&&) = default;

	FDoubleVect2& operator+=(const FDoubleVect2& Value)
	{
		X += Value.X;
		Y += Value.Y;
		return *this;
	}

	constexpr FDoubleVect2 operator*(const double& Value) const 
	{
		return FDoubleVect2( X * Value, Y * Value );
	}

	constexpr FDoubleVect2 operator*(const FDoubleVect2& Other) const 
	{
		return FDoubleVect2(X * Other.X, Y * Other.Y);
	}

	FORCEINLINE FVector ToFVector() const 
	{
		return FVector(X, Y, 0.0f);
	}
	FString ToString()const
	{
		return FString::Printf(TEXT("X = %.9f, Y =%.9f"), X, Y);
	}

	bool EqualTo(const FDoubleVect2& OtherVal, double Epsilon = 0.000'000'001) const
	{
		return abs(OtherVal.X - X + OtherVal.Y - Y) < Epsilon;
	}

	constexpr FDoubleVect2 operator-(const FDoubleVect2& other) const
	{
		return FDoubleVect2(X - other.X, Y - other.Y);
	}

	constexpr FDoubleVect2 operator+(const FDoubleVect2& other) const
	{
		return FDoubleVect2(X + other.X, Y + other.Y);
	}

	constexpr FDoubleVect2 operator/(const double& Value) const
	{
		return FDoubleVect2(X / Value, Y / Value);
	}

	constexpr FDoubleVect2 operator/(const FDoubleVect2& Value) const
	{
		return FDoubleVect2(X / Value.X, Y / Value.Y);
	}
};

struct FLevelImportData
{
public:
	//FLevelImportData& operator=(FLevelImportData&&) = default;
	FLevelImportData& operator=(const FLevelImportData&) = default;

	FString ToString() const;
	FString ToCSVString() const;
	static FLevelImportData FromString(FString StrData);
	static FLevelImportData FromCSV(FString SrtData);
	static FLevelImportData FromStringArray(const TArray<FString> & StrArrData);

public:
	
	FDoubleVect2 LatAndLon;
	FDoubleVect2 EndLatAndLon;
	FDoubleVect2 CoordsXY;
	FDoubleVect2 EndCoordsXY;
	FDoubleVect2 SizeXY;
	int32 ImportStatus = 0;
	FString LevelName;

};

