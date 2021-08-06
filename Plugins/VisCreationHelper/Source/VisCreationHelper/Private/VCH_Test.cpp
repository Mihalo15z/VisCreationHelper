// Fill out your copyright notice in the Description page of Project Settings.


#include "VCH_Test.h"
#include "MercatorConvertor/GeoMercatorConvertor.h"


//DECLARE_LOG_CATEGORY_EXTERN(VCH_TestLog, Log, All);
DECLARE_LOG_CATEGORY_CLASS(VCH_TestLog, Log, All);
//DEFINE_LOG_CATEGORY(VCH_TestLog);



class GeoMercatorConvertor final
{
public:
	constexpr GeoMercatorConvertor() = default;
	~GeoMercatorConvertor() = default;
private:

	static const double Radius_Major;
	static const double Radius_Minor;
	static const double ratio;
	static const double eccent;
	static const double com;
	static const double PI_D;
	static const double degreesToRadians;
	static const double radiansToDegrees;


	/*const static double Radius_Major = (6378137.0);
	const static double Radius_Minor = (6356752.3142);
	const static double ratio = (Radius_Minor / Radius_Major);
	const static double eccent = (sqrt(1.0 - ratio * ratio));
	const static double com = (0.5 * eccent);
	const static double PI_D = (3.14159265358979323846264338327950288419716939937510);
	const static double degreesToRadians = (PI_D / 180.0);
	const static double radiansToDegrees = (180.0 / PI_D);
*/

private:
	FORCEINLINE static double GetPhi(double ts, double e)
	{
		constexpr int iter = 15;
		//double halfPI = PI / 2;
		constexpr double tol = 0.0000000001;
		double eccnth, Phi, con, dphi;
		int i;
		eccnth = 0.5 * e;
		Phi = PI_D / 2 - 2 * atan(ts);
		i = iter;

		do
		{
			con = e * sin(Phi);
			dphi = PI_D / 2 - 2 * atan(ts * pow((1 - con) / (1 + con), eccnth)) - Phi;
			Phi += dphi;

		} while (FMath::Abs<double>(dphi) > tol && --i);

		return Phi;
	}

public:
	// return FDoubleVect2(X, Y)
	FORCEINLINE  static FDoubleVect2 GetGeoForMercator(double Lat, double Lon)
	{
		double x = Radius_Major * degreesToRadians * Lon;

		Lat = fmin(89.5, fmax(Lat, -89.5));
		double phi = degreesToRadians * Lat;
		double con = eccent * sin(phi);
		con = pow((1.0 - con) / (1.0 + con), com);
		double ts = tan(0.5 * (PI_D * 0.5 - phi)) / con;
		double y = 0.0 - Radius_Major * log(ts);
		return FDoubleVect2(x, y);
	}

	// return FDoubleVect2(Lat, Lon)
	FORCEINLINE static FDoubleVect2 GetMercatoforGeo(double X, double Y)
	{
		double lon = radiansToDegrees * X / Radius_Major;
		double lat = radiansToDegrees * GetPhi(exp(0.0 - (Y / Radius_Major)), eccent);
		return FDoubleVect2(lat, lon);
	}
};


const double GeoMercatorConvertor::Radius_Major(6378137.0);
const double GeoMercatorConvertor::Radius_Minor(6356752.3142);
const double GeoMercatorConvertor::ratio(Radius_Minor / Radius_Major);
const double GeoMercatorConvertor::eccent(sqrt(1.0 - ratio * ratio));
const double GeoMercatorConvertor::com(0.5 * eccent);
const double GeoMercatorConvertor::PI_D(3.14159265358979323846264338327950288419716939937510);
const double GeoMercatorConvertor::degreesToRadians(PI_D / 180.0);
const double GeoMercatorConvertor::radiansToDegrees(180.0 / PI_D);


FVCH_Test::FVCH_Test()
{
}

FVCH_Test::~FVCH_Test()
{
}

FString FVCH_Test::GlobalTest()
{
	UE_LOG(VCH_TestLog, Log, TEXT("Start Global Test"));

	TestConvertorToMercator();
	TestPerfomanceForConvertorToMercator();
	MultiTestConvertorToMercator();

	return FString();
}

FString FVCH_Test::TestConvertorToMercator()
{
	UE_LOG(VCH_TestLog, Log, TEXT("TestConvertorToMercator"));

	constexpr FDoubleVect2 TestLatLon(32.6553830000000, 57.3566600000000);

	auto TestCoorMerc = FGeoMercatorConvertor::GetGeoForMercator(TestLatLon.X, TestLatLon.Y);
	auto OldTestCoorMerc = GeoMercatorConvertor::GetGeoForMercator(TestLatLon.X, TestLatLon.Y);
	UE_LOG(VCH_TestLog, Log, TEXT("Geo: %s"), *TestCoorMerc.ToString());
	UE_LOG(VCH_TestLog, Log, TEXT("GeoOld: %s"), *OldTestCoorMerc.ToString());

	auto TestCoordGeo = FGeoMercatorConvertor::GetMercatoforGeo(TestCoorMerc.X, TestCoorMerc.Y);
	//GeoMercatorConvertor Geo;
	auto OldCoordGeo = GeoMercatorConvertor::GetMercatoforGeo(OldTestCoorMerc.X, OldTestCoorMerc.Y);
	UE_LOG(VCH_TestLog, Log, TEXT("Merk: %s"), *TestCoordGeo.ToString());
	UE_LOG(VCH_TestLog, Log, TEXT("OldMerk: %s"), *OldCoordGeo.ToString());

	if (!(TestCoorMerc.EqualTo(OldTestCoorMerc) && TestCoordGeo.EqualTo(OldCoordGeo) && TestLatLon.EqualTo(TestCoordGeo)))
	{
		UE_LOG(VCH_TestLog, Error, TEXT("Bad Math for GeoToMercator"));
		return TEXT("Error: bad Test");
	}
	return TEXT("Good Test");
}

FString FVCH_Test::TestPerfomanceForConvertorToMercator()
{
	UE_LOG(VCH_TestLog, Log, TEXT("TestPerfomanceForConvertorToMercator"));
	//make test data
	TArray<FDoubleVect2> TestCoords;
	constexpr float MaxCoord = 88.f;
	constexpr float MinCoord = -MaxCoord;
	constexpr int32 TestDataSize = 1'000'000;
	for (int32 i(0); i < TestDataSize; ++i)
	{
		TestCoords.Add({ FMath::RandRange(MaxCoord, MinCoord),FMath::RandRange(MaxCoord, MinCoord) });
	}
	//test GetGeoForMercator
	{
		FVCHTimeConter TestCounter(TEXT("New GetGeoForMercator"));
		for (const auto& coord : TestCoords)
		{
			FGeoMercatorConvertor::GetGeoForMercator(coord.X, coord.Y);
		}
	}
	// test old
	{
		FVCHTimeConter TestCounter(TEXT("Old GetGeoForMercator"));
		for (const auto& coord : TestCoords)
		{
			GeoMercatorConvertor::GetGeoForMercator(coord.X, coord.Y);
		}
	}
	// make test data
	TArray<FDoubleVect2> TestCoordsForGeo;
	TestCoordsForGeo.Reserve(TestDataSize);
	{
		FVCHTimeConter TestCounter(TEXT("Make TestData for Get Geo"));
		for (const auto& coord : TestCoords)
		{
			TestCoordsForGeo.Add(FGeoMercatorConvertor::GetGeoForMercator(coord.X, coord.Y));
		}
	}
	// test GetMercatoforGeo
	{
		FVCHTimeConter TestCounter(TEXT("New GetMercatoforGeo"));
		for (const auto& coord : TestCoordsForGeo)
		{
			FGeoMercatorConvertor::GetMercatoforGeo(coord.X, coord.Y);
		}
	}
	// test old GetMercatoforGeo
	{
		//FGeoMercatorConvertor Geo;
		FVCHTimeConter TestCounter(TEXT("Old GetMercatoforGeo"));
		for (const auto& coord : TestCoordsForGeo)
		{
			GeoMercatorConvertor::GetMercatoforGeo(coord.X, coord.Y);
		}
	}


	return FString();
}

FString FVCH_Test::MultiTestConvertorToMercator()
{
	UE_LOG(VCH_TestLog, Log, TEXT(" *** MultiTestConvertorToMercator ***"));
	TArray<FDoubleVect2> TestCoords;
	constexpr float MaxCoord = 88.f;
	constexpr float MinCoord = -MaxCoord;
	constexpr int32 TestDataSize = 150;
	for (int32 i(0); i < TestDataSize; ++i)
	{
		TestCoords.Add({ FMath::RandRange(MaxCoord, MinCoord),FMath::RandRange(MaxCoord, MinCoord) });
	}
	int32 ErrorCounter(0);
	for (const auto& TestLatLon : TestCoords)
	{
		auto TestCoorMerc = FGeoMercatorConvertor::GetGeoForMercator(TestLatLon.X, TestLatLon.Y);
		auto OldTestCoorMerc = GeoMercatorConvertor::GetGeoForMercator(TestLatLon.X, TestLatLon.Y);

		auto TestCoordGeo = FGeoMercatorConvertor::GetMercatoforGeo(TestCoorMerc.X, TestCoorMerc.Y);
		auto OldCoordGeo = GeoMercatorConvertor::GetMercatoforGeo(OldTestCoorMerc.X, OldTestCoorMerc.Y);

		if (!(TestCoorMerc.EqualTo(OldTestCoorMerc, 0.000'001) && TestCoordGeo.EqualTo(OldCoordGeo) && TestLatLon.EqualTo(TestCoordGeo)))
		{
			UE_LOG(VCH_TestLog, Error, TEXT("Bad Math for GeoToMercator TestLatLon(%s) TestCoordGeo(%s), TestCoorMerc(%s), OldTestCoorMerc(%s)"),
				*TestLatLon.ToString(), *TestCoordGeo.ToString(), *TestCoorMerc.ToString(), *OldTestCoorMerc.ToString());
			++ErrorCounter;
		}
	}

	if(ErrorCounter > 0)
	{
		UE_LOG(VCH_TestLog, Error, TEXT("Bad Math for GeoToMercator num = %i"), ErrorCounter);
	}
	else
	{
		UE_LOG(VCH_TestLog, Warning, TEXT("Good Math for GeoToMercator num = %i"), ErrorCounter);
	}

	return FString();
}

FVCHTimeConter::FVCHTimeConter(const FString & InCounterInfo) :CounterInfo(InCounterInfo)
{
	StartCicles = FPlatformTime::Cycles();
}

FVCHTimeConter::~FVCHTimeConter()
{
	UE_LOG(VCH_TestLog, Log, TEXT("%s: time = %f"), *CounterInfo, FPlatformTime::ToMilliseconds(FPlatformTime::Cycles() - StartCicles));
}
