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
	return x >= 0 && x < std::numeric_limits<double>::infinity()
		? Detail::sqrtNewtonRaphson(x, x, 0)
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

};


class VISCREATIONHELPER_API GeoMercatorConvertor final
{
public:
	constexpr GeoMercatorConvertor() = default;
	~GeoMercatorConvertor() = default;
private:

	static constexpr double Radius_Major = (6378137.0);
	static constexpr double Radius_Minor = (6356752.3142);
	static constexpr double ratio = (Radius_Minor / Radius_Major);
	static constexpr double eccent = (My_Sqrt(1.0 - ratio * ratio));
	static constexpr double com = (0.5 * eccent);
	static constexpr double PI_D = (3.14159265358979323846264338327950288419716939937510);
	static constexpr double degreesToRadians = (PI_D / 180.0);
	static constexpr double radiansToDegrees = (180.0 / PI_D);

private:
	FORCEINLINE double GetPhi(double ts, double e)
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
		con = pow((1 - con) / (1 + con), com);
		double ts = tan(0.5 * (PI_D * 0.5 - phi)) / con;
		double y = 0 - Radius_Major * log(ts);
		return FDoubleVect2(x, y);
	}

	// return FDoubleVect2(Lat, Lon)
	FORCEINLINE FDoubleVect2 GetMercatoforGeo(double X, double Y)
	{
		double lon = radiansToDegrees * X / Radius_Major;
		double lat = radiansToDegrees * GetPhi(exp(0 - (Y / Radius_Major)), eccent);
		return FDoubleVect2(lat, lon);
	}
};
