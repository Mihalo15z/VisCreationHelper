// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <limits>
#include "DoubleVect2.h"




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


class MERCATORCONVERTORMS_API FGeoMercatorConvertor final
{
private:

	static constexpr double Radius_Major = (6'378'137.0);
	static constexpr double Radius_Minor = (6'356'752.314'2);
	static constexpr double Ratio = (Radius_Minor / Radius_Major);
	static constexpr double Eccent = (My_Sqrt(1.0 - Ratio * Ratio));
	static constexpr double Com = (0.5 * Eccent);
	static constexpr double PI_D = (3.141'592'653'589);
	static constexpr double DegreesToRadians = (PI_D / 180.0);
	static constexpr double RadiansToDegrees = (180.0 / PI_D);
	static constexpr double HalfPI = PI_D / 2.0;
	static constexpr double MaxLim = 89.5;
	static constexpr double MinLim = -MaxLim;
	static constexpr double TempRadToDegDivRMajor = RadiansToDegrees / Radius_Major;
	static constexpr double TempMulRMajAndDToR = Radius_Major * DegreesToRadians;
	static constexpr double Tol = 0.000'000'000'1;
	static constexpr double Two_D = 2.0;
	static constexpr double One_D = 1.0;
	static constexpr double Half_D = 0.5;

private:

	FORCEINLINE static double GetPhi(double Ts)
	{
		//int i(15);
		double Phi( HalfPI - 2.0 * atan(Ts));

		double Con, dPhi;
		// 770+ times faster
		CalcDPhi(Con, dPhi, Phi, Ts) 		 
			&& CalcDPhi(Con, dPhi, Phi, Ts) 
			&& CalcDPhi(Con, dPhi, Phi, Ts) 
			&& CalcDPhi(Con, dPhi, Phi, Ts) 
			&& CalcDPhi(Con, dPhi, Phi, Ts) 
											
			&& CalcDPhi(Con, dPhi, Phi, Ts) 
			&& CalcDPhi(Con, dPhi, Phi, Ts) 
			&& CalcDPhi(Con, dPhi, Phi, Ts) 
			&& CalcDPhi(Con, dPhi, Phi, Ts) 
			&& CalcDPhi(Con, dPhi, Phi, Ts) 
											
			&& CalcDPhi(Con, dPhi, Phi, Ts) 
			&& CalcDPhi(Con, dPhi, Phi, Ts) 
			&& CalcDPhi(Con, dPhi, Phi, Ts) 
			&& CalcDPhi(Con, dPhi, Phi, Ts) 
			&& CalcDPhi(Con, dPhi, Phi, Ts);


		//***** old code
		
		//do
		//{
		//	Con = Eccent * sin(Phi);
		//	dPhi = HalfPI - 2.0 * atan(Ts * pow((1.0 - Con) / (1.0 + Con), Com)) - Phi;
		//	Phi += dPhi;

		//} while (FMath::Abs<double>(dPhi) > Tol && --i);

		return Phi;
	}

	FORCEINLINE static bool CalcDPhi(double& Con, double& dPhi, double& Phi, const double& ts)
	{
		Con = Eccent * sin(Phi);
		dPhi = HalfPI - Two_D * atan(ts * pow((One_D - Con) / (One_D + Con), Com)) - Phi;
		Phi += dPhi;
		return  FMath::Abs<double>(dPhi) > Tol;
	}

public:
	// return FDoubleVect2(X, Y)
	FORCEINLINE  static FDoubleVect2 GetGeoForMercator(double Lat, double Lon)
	{
		//double x = TempMulRMajAndDToR * Lon;
		Lat = fmin(MaxLim, fmax(Lat, MinLim));
		double Phi = DegreesToRadians * Lat;
		double Con = Eccent * sin(Phi);
		Con = pow((One_D - Con) / (One_D + Con), Com);
		double Ts = tan(Half_D * (HalfPI - Phi)) / Con;
		//double y = -Radius_Major * log(Ts);
		return FDoubleVect2(TempMulRMajAndDToR * Lon, -Radius_Major * log(Ts));
	}

	FORCEINLINE  static FDoubleVect2 GetGeoForMercator(const FDoubleVect2& LatAndLon)
	{
		return GetGeoForMercator(LatAndLon.X, LatAndLon.Y);
	}

	// return FDoubleVect2(Lat, Lon)
	FORCEINLINE static FDoubleVect2 GetMercatoforGeo(double X, double Y)
	{
		//double lon = TempRadToDegDivRMajor * X;
		//double lat = RadiansToDegrees * GetPhi(exp(-(Y / Radius_Major)));
		return FDoubleVect2(RadiansToDegrees * GetPhi(exp(-(Y / Radius_Major))), TempRadToDegDivRMajor * X);
	}

	FORCEINLINE static FDoubleVect2 GetMercatoforGeo(const FDoubleVect2& CoordsXY)
	{
		return GetMercatoforGeo(CoordsXY.X, CoordsXY.Y);
	}
};
