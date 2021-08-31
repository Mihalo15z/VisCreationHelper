// Fill out your copyright notice in the Description page of Project Settings.


#include "NameEncoder/FNameEncoder.h"

DECLARE_LOG_CATEGORY_CLASS(VCH_EncoderNameLog, Log, All);

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
		UE_LOG(VCH_EncoderNameLog, Error, TEXT("Bad Mask %s"), *Mask );
		StartLetterIndex = -1;
		LenLetterIndex = -1;
		bValid = false;
		return;
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
		UE_LOG(VCH_EncoderNameLog, Error, TEXT("Bad Mask %s"), *Mask);
		StartNumericIndex = -1;
		LenNumericIndex = -1;
		bValid = false;
		return;
	}

	bValid = true;
}

bool FNameEncoder::GetIndeces(const FString InName, int32& OutLetterIndex, int32& OutNumericIndex) const
{
	if (!CheckName(InName))
	{
		UE_LOG(VCH_EncoderNameLog, Error, TEXT("Bad Name %s"), *InName);
		return false;
	}
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
	return true;
}

FString FNameEncoder::GetName(int32 InLetterIndex, int32 InNumericIndex) const 
{
	if (InLetterIndex < 0 || InNumericIndex < 0)
	{
		UE_LOG(VCH_EncoderNameLog, Error, TEXT("Bad indexes L = %i, N = %i"), InLetterIndex, InNumericIndex);
		return {};
	}

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

	name.ReplaceInline((LetterBlock.GetCharArray().GetData()), LetterIndex.GetData());
	name.ReplaceInline((NumericBlock.GetCharArray().GetData()), NumericIndex.GetData());
	return name;
}

bool FNameEncoder::CheckName(FString InName) const
{
	if (InName.Len() != Mask.Len())
	{
		return false;
	}
	auto LetterData = InName.Mid(StartLetterIndex, LenLetterIndex);
	auto NumericData = InName.Mid(StartNumericIndex, LenNumericIndex);
	InName.ReplaceInline(LetterData.GetCharArray().GetData(), LetterBlock.GetCharArray().GetData());
	InName.ReplaceInline(NumericData.GetCharArray().GetData(), NumericBlock.GetCharArray().GetData());
	return Mask == InName;
}


