// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * convert name to indexes and convert indexes to name
 * Sample: Mask Test@@##, name TestAC53; indexes X =  03, y = 53
 */
class VISCREATIONHELPER_API FNameEncoder
{
public:
	FNameEncoder(FString InMask, TCHAR InLetterSymbol = '@', TCHAR InNumberSymbol = '#');
	~FNameEncoder();
	FNameEncoder(const FNameEncoder& Other) = delete;
private:
	FString Mask;
	TCHAR LetterSymbol;
	TCHAR NumericSymbol;
	int32 LenLetterIndex;
	int32 LenNumericIndex;
	int32 StartLetterIndex;
	int32 StartNumericIndex;

	FString LetterBlock;
	FString NumericBlock;

	// static data
	static const FString NumberChar;
	static const FString Alphabet;
	static constexpr int32 AlphabetLen = 26;
	static constexpr int32 NumberLen = 10;

private:
	// get Start Position and get len for numeric and letter data.
	void ParseMask();

public:
	//  Get Indexes from name
	bool GetIndeces(const FString InName, int32& OutLetterIndex, int32& OutNumericIndex) const;
	// Get name from Indexes
	FString GetName(int32 InLetterIndex, int32 InNumericIndex) const;

	bool CheckName(FString InName) const;
	bool IsValid() const
	{
		return  bValid;
	}
private:
	bool bValid = false;

};

