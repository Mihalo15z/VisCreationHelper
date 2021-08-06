// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <limits>



/**
 *  Convert  coordinate to mercator
 */



namespace Detail
{
	double constexpr sqrtNewtonRaphson(double x, double curr, double prev)
	{
		return curr == prev
			? curr
			: sqrtNewtonRaphson(x, 0.5 * (curr + x / curr), curr);
	}
}

/*
* Constexpr version of the square root
* Return value:
*   - For a finite and non-negative value of "x", returns an approximation for the square root of "x"
*   - Otherwise, returns NaN
*/
double constexpr My_Sqrt(double x)
{
	return x >= 0.0 && x < std::numeric_limits<double>::infinity()
		? Detail::sqrtNewtonRaphson(x, x, 0.0)
		: std::numeric_limits<double>::quiet_NaN();
}


struct  VISCREATIONHELPER_API FDoubleVect2
{
	double X;
	double Y;

	constexpr FDoubleVect2() :X(0.0), Y(0.0)
	{};

	constexpr FDoubleVect2(double x, double y) :X(x), Y(y)
	{};

	FORCEINLINE FVector operator=(const FDoubleVect2& rh)
	{
		return FVector(rh.X, rh.Y, 0.0f);
	}
	FString ToString()const
	{
		return FString::Printf(TEXT("X = %.9f, Y =%.9f"), X, Y);
	}

	bool EqualTo(const FDoubleVect2& OtherVal, double Epsilon = 0.000'000'001) const
	{
		return abs(OtherVal.X - X + OtherVal.Y - Y) < Epsilon;
	}
};


class VISCREATIONHELPER_API FGeoMercatorConvertor final
{
public:
	constexpr FGeoMercatorConvertor() = default;
	~FGeoMercatorConvertor() = default;
private:

	static constexpr double Radius_Major = (6'378'137.0);
	static constexpr double Radius_Minor = (6'356'752.314'2);
	static constexpr double Ratio = (Radius_Minor / Radius_Major);
	static constexpr double Eccent = (My_Sqrt(1.0 - Ratio * Ratio));
	static constexpr double Com = (0.5 * Eccent);
	static constexpr double PI_D = (3.141'592'653'589'793'238'462);
	static constexpr double DegreesToRadians = (PI_D / 180.0);
	static constexpr double RadiansToDegrees = (180.0 / PI_D);
	static constexpr double HalfPI = PI_D / 2.0;
	static constexpr double MaxLim = 89.5;
	static constexpr double MinLim = -MaxLim;
	static constexpr double TempRadToDegDivRMajor = RadiansToDegrees / Radius_Major;
	static constexpr double TempMulRMajAndDToR = Radius_Major * DegreesToRadians;
	static constexpr double tol = 0.000'000'000'1;
	static constexpr double Two_D = 2.0;
	static constexpr double One_D = 1.0;
	static constexpr double Half_D = 0.5;

private:

	FORCEINLINE static double GetPhi(double ts)
	{
		//int i(15);
		double Phi( HalfPI - 2.0 * atan(ts));

		double con, dphi;
		// Faster 770+
		CalcDPhi(con, dphi, Phi, ts) 		 
			&& CalcDPhi(con, dphi, Phi, ts) 
			&& CalcDPhi(con, dphi, Phi, ts) 
			&& CalcDPhi(con, dphi, Phi, ts) 
			&& CalcDPhi(con, dphi, Phi, ts) 
											
			&& CalcDPhi(con, dphi, Phi, ts) 
			&& CalcDPhi(con, dphi, Phi, ts) 
			&& CalcDPhi(con, dphi, Phi, ts) 
			&& CalcDPhi(con, dphi, Phi, ts) 
			&& CalcDPhi(con, dphi, Phi, ts) 
											
			&& CalcDPhi(con, dphi, Phi, ts) 
			&& CalcDPhi(con, dphi, Phi, ts) 
			&& CalcDPhi(con, dphi, Phi, ts) 
			&& CalcDPhi(con, dphi, Phi, ts) 
			&& CalcDPhi(con, dphi, Phi, ts);

		//do
		//{
		//	con = Eccent * sin(Phi);
		//	dphi = HalfPI - 2.0 * atan(ts * pow((1.0 - con) / (1.0 + con), Com)) - Phi;
		//	Phi += dphi;

		//} while (FMath::Abs<double>(dphi) > tol && --i);

		return Phi;
	}

	FORCEINLINE static bool CalcDPhi(double& con, double& dphi, double& Phi, const double& ts)
	{
		con = Eccent * sin(Phi);
		dphi = HalfPI - Two_D * atan(ts * pow((One_D - con) / (One_D + con), Com)) - Phi;
		Phi += dphi;
		return  FMath::Abs<double>(dphi) > tol;
	}

public:
	// return FDoubleVect2(X, Y)
	FORCEINLINE  static FDoubleVect2 GetGeoForMercator(double Lat, double Lon)
	{
		//double x = TempMulRMajAndDToR * Lon;
		Lat = fmin(MaxLim, fmax(Lat, MinLim));
		double phi = DegreesToRadians * Lat;
		double con = Eccent * sin(phi);
		con = pow((One_D - con) / (One_D + con), Com);
		double ts = tan(Half_D * (HalfPI - phi)) / con;
		//double y = -Radius_Major * log(ts);
		return FDoubleVect2(TempMulRMajAndDToR * Lon, -Radius_Major * log(ts));
	}

	// return FDoubleVect2(Lat, Lon)
	FORCEINLINE static FDoubleVect2 GetMercatoforGeo(double X, double Y)
	{
		//double lon = TempRadToDegDivRMajor * X;
		//double lat = RadiansToDegrees * GetPhi(exp(-(Y / Radius_Major)));
		return FDoubleVect2(RadiansToDegrees * GetPhi(exp(-(Y / Radius_Major))), TempRadToDegDivRMajor * X);
	}
};
