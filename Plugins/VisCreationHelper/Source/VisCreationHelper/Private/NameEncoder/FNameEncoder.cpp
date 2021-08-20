// Fill out your copyright notice in the Description page of Project Settings.


#include "NameEncoder/FNameEncoder.h"


const  FString FNameEncoder::NumberChar = TEXT("0123456789-");
const FString FNameEncoder::Alphabet =  TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ");

FNameEncoder::FNameEncoder(FString InMask, TCHAR InLetterSymbol, TCHAR InNumberSymbol):Mask(InMask), LetterSymbol(InLetterSymbol), NumericSymbol(InNumberSymbol)
{
	ParseMask();
}

FNameEncoder::~FNameEncoder()
{
}

void FNameEncoder::ParseMask()
{
	//  Letter Index
	if (Mask.FindChar(LetterSymbol, StartLetterIndex))
	{
		int32 lastIndex;
		Mask.FindLastChar(LetterSymbol, lastIndex);
		LenLetterIndex = 1 + lastIndex - StartLetterIndex;
		for (int32 i(0); i < LenLetterIndex; ++i)
		{
			LetterBlock += LetterSymbol;
		}
	}
	else
	{
		StartLetterIndex = -1;
		LenLetterIndex = -1;
	}

	// Numeric Index
	if (Mask.FindChar(NumericSymbol, StartNumericIndex))
	{
		int32 lastIndex;
		Mask.FindLastChar(NumericSymbol, lastIndex);
		LenNumericIndex = 1 + lastIndex - StartNumericIndex;
		for (int32 i(0); i < LenNumericIndex; ++i)
		{
			NumericBlock += NumericSymbol;
		}
	}
	else
	{
		StartNumericIndex = -1;
		LenNumericIndex = -1;
	}
}

void FNameEncoder::GetIndeces(const FString InName, int32 & OutLetterIndex, int32 & OutNumericIndex)
{
	FString letters(InName.Mid(StartLetterIndex, LenLetterIndex));
	FString numerics(InName.Mid(StartNumericIndex, LenNumericIndex));

	OutNumericIndex = FCString::Atoi(*numerics);

	OutLetterIndex = 0;
	for (int32 i(0); i < letters.Len(); ++i)
	{
		int32 value;
		Alphabet.FindChar(letters[i], value);
		OutLetterIndex += value * FMath::Pow(AlphabetLen, letters.Len() - 1 - i);
	}
}

FString FNameEncoder::GetName(int32 InLetterIndex, int32 InNumericIndex)
{
	FString name(Mask);
	// Get letter index
	TArray<TCHAR> LetterIndex;
	LetterIndex.SetNumZeroed(LenLetterIndex);
	if (InLetterIndex >= 0)
	{
		for (int32 i(0); i < LenLetterIndex; ++i)
		{
			LetterIndex[LenLetterIndex - i - 1] = (Alphabet[InLetterIndex % AlphabetLen]);
			InLetterIndex /= AlphabetLen;
		}
		LetterIndex.Add('\0');
	}

	// Get numeric index
	TArray<TCHAR> NumericIndex;
	NumericIndex.SetNumZeroed(LenNumericIndex);
	if (InNumericIndex >= 0)
	{
		for (int32 i(0); i < LenNumericIndex; ++i)
		{
			NumericIndex[LenNumericIndex - i - 1] = NumberChar[InNumericIndex % NumberLen];
			InNumericIndex /= NumberLen;
		}
		NumericIndex.Add('\0');
	}

	name = name.Replace((LetterBlock.GetCharArray().GetData()), LetterIndex.GetData());
	name = name.Replace((NumericBlock.GetCharArray().GetData()), NumericIndex.GetData());
	return name;
}


