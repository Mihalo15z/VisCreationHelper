// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "Runtime/XmlParser/Public/XmlParser.h"
#include "Engine.h"
#include "Misc/AssertionMacros.h"
#include <map>

/**
 * The public interface to this module.  In most cases, this interface is only public to sibling modules 
 * within this plugin.
 */



class IConfPlugin : public IModuleInterface
{

public:

	struct CameraConf
	{
		double dX;
		double dY;
		double dZ;

		double rotX;
		double rotY;
		double rotZ;

		double fov_vert;
		double fov_horiz;

		int32 resX;
		int32 resY;

		int32 selfRender;

		FVector getDPos(){ return FVector(dX,dY,dZ); };
		FRotator getDRot(){ return FRotator(rotX, rotY, rotZ); };
	};

		struct mPlayer
		{
	
			int32 numPlayer;
			int32 numObject;
			int32 typeObject;
	
	
		};

		struct exPlayerTime
		{
			exPlayerTime()
			{
				numPlayer = 0;
				extrap2Dt = 0.01;
				extrap = false;
				extrapMode = 1;
			};

			exPlayerTime(int32 n, double t)
			{
				numPlayer = n; 
				extrap2Dt = t;
			};

			int32 numPlayer;

			bool extrap;
			int32 extrapMode;

			double extrap2Dt;

		};

		typedef std::map<int32, exPlayerTime> exPlayerTimeBuff;
		typedef std::map<int32, exPlayerTime>::iterator exPlayerTimeBuffIter;
	
		class configvalues
		{
		public:
	
			double timeDivider;

			double GeoScale;
		
			double CenterLatValue;
		
			double CenterLonValue;
	
			bool showDebugInfo;
	
			bool decModeEnabled;
			int32 roleId;
			int32 netPort;
			int32 netMode;

			bool extrapMain;
			bool extrapOther;

			bool syncObjectWait;

			int32 extrapModeMain;
			int32 extrapModeOther;
			double extrap2DtMain;
			double extrap2DtOther;

			exPlayerTimeBuff etPlayersTime;
	
			bool enableCollisions;
			int32 surfInfo;

			FName FrontWheelName;
			FName RightWheelName;
			FName LeftWheelName;

			FName CollisionModelName;


			FVector FrontWheel;
			FVector RightWheel;
			FVector LeftWheel;

			mPlayer mainPlayer;

			CameraConf cameraConfig;
	
			bool inited;
			configvalues()
			{
				FMemory::Memzero(this, sizeof(this));
				showDebugInfo = false;
				inited = false;
			}
		};
	

		
		static void showErrMsg(const FString msg, bool onlyThis=false,  FColor color = FColor::Red)
		{
			static bool onlyMode = false;
			if (onlyThis) onlyMode = true;
			if (Init().showDebugInfo && ((onlyMode && onlyThis) || (!onlyMode)))
			{
					GEngine->AddOnScreenDebugMessage(-1, 5.f, color, msg);


					
			}
		}

		

		static configvalues Init(bool reinit=false, bool disableWait=false , unsigned char currentParalaxState = 0)
		{

			static configvalues dataSet;
		
			if (disableWait)
			{
				dataSet.syncObjectWait = false;
				return dataSet;
			}

			if (reinit) dataSet.inited = false;

			if (!dataSet.inited)
			{
				
				dataSet.mainPlayer.typeObject = -100;
				

				FString Path = FPaths::ProjectDir() + "visualization.config";

				switch (currentParalaxState)
				{
				case 0:
					Path = FPaths::ProjectDir() + "visualization.config";          //for game mode
					break;
				case 1:
					Path = FPaths::ProjectDir() + "visualization_1.config";          //for game mode
					break;
				case 2:
					Path = FPaths::ProjectDir() + "visualization_2.config";          //for game mode
					break;
				default:
					Path = FPaths::ProjectDir() + "visualization.config";          //for game mode
					break;
				}

				if (FPaths::FileExists(*Path))
				{
					
					FXmlFile ConfigFile;
					ConfigFile.LoadFile(Path);
					FXmlNode* MainTag = ConfigFile.GetRootNode();


					FXmlNode* WorldSettings = MainTag->FindChildNode("WorldSettings");
						if (WorldSettings)
						{
	
							dataSet.GeoScale = FCString::Atof(*WorldSettings->GetAttribute("GeoScale"));
							dataSet.CenterLatValue = FCString::Atof(*WorldSettings->GetAttribute("CenterLatValue"));
							dataSet.CenterLonValue = FCString::Atof(*WorldSettings->GetAttribute("CenterLonValue"));

							dataSet.timeDivider = FCString::Atof(*WorldSettings->GetAttribute("timeDivider"));

							dataSet.showDebugInfo = FCString::Atoi(*WorldSettings->GetAttribute("ShowDebugInfo"))!=0;
						}


						FXmlNode* CameraCfg = MainTag->FindChildNode("CameraConfig");
						if (CameraCfg)
						{

							dataSet.cameraConfig.dX = FCString::Atod(*CameraCfg->GetAttribute("dX"));
							dataSet.cameraConfig.dY = FCString::Atod(*CameraCfg->GetAttribute("dY"));
							dataSet.cameraConfig.dZ = FCString::Atod(*CameraCfg->GetAttribute("dZ"));

							dataSet.cameraConfig.rotX = FCString::Atof(*CameraCfg->GetAttribute("rotX"));
							dataSet.cameraConfig.rotY = FCString::Atof(*CameraCfg->GetAttribute("rotY"));
							dataSet.cameraConfig.rotZ = FCString::Atof(*CameraCfg->GetAttribute("rotZ"));

							dataSet.cameraConfig.fov_vert = FCString::Atof(*CameraCfg->GetAttribute("fov_vert"));
							dataSet.cameraConfig.fov_horiz = FCString::Atof(*CameraCfg->GetAttribute("fov_horiz"));

							dataSet.cameraConfig.resX = FCString::Atoi(*CameraCfg->GetAttribute("resX"));
							dataSet.cameraConfig.resY = FCString::Atoi(*CameraCfg->GetAttribute("resY"));

							dataSet.cameraConfig.selfRender = FCString::Atoi(*CameraCfg->GetAttribute("selfRender"));

						}
	
						
					dataSet.inited = true;
				}


				Path = FPaths::ProjectDir() + "network.config";
				if (FPaths::FileExists(*Path))
				{
					dataSet.mainPlayer.typeObject = -200;

					FXmlFile ConfigFile;
					ConfigFile.LoadFile(Path);
					FXmlNode* MainTag = ConfigFile.GetRootNode();

					FXmlNode* Role = MainTag->FindChildNode("Role");
					if (Role)
					{
						dataSet.roleId = FCString::Atoi(*Role->GetAttribute("id"));

					}


					FXmlNode* DecMode = MainTag->FindChildNode("DecMode");
					if (Role)
					{
						dataSet.decModeEnabled = (FCString::Atoi(*DecMode->GetAttribute("enabled")) > 0);

					}
					else
					{
						dataSet.decModeEnabled = false;
					}

					

					FXmlNode* NetworkCfg = MainTag->FindChildNode("NetworkCfg");
					if (NetworkCfg)
					{
						dataSet.netPort = FCString::Atoi(*NetworkCfg->GetAttribute("port"));
						dataSet.netMode = FCString::Atoi(*NetworkCfg->GetAttribute("netMode"));

					}


					FXmlNode* MainPlayer = MainTag->FindChildNode("MainPlayer");
					if (MainPlayer)
					{
						dataSet.mainPlayer.numObject = FCString::Atoi(*MainPlayer->GetAttribute("NumObject"));
						dataSet.mainPlayer.numPlayer = FCString::Atoi(*MainPlayer->GetAttribute("NumPlayer"));
						dataSet.mainPlayer.typeObject = FCString::Atoi(*MainPlayer->GetAttribute("TypeObject"));
					}
				}
				else
				{
					dataSet.roleId = 0;
					dataSet.netPort = 7200;
					dataSet.netMode = 2;
					dataSet.mainPlayer.numObject = 0;
					dataSet.mainPlayer.numPlayer = 1;
					dataSet.mainPlayer.typeObject = 2;
				}


				Path = FPaths::ProjectDir() + "extrapolator.config";
				if (FPaths::FileExists(*Path))
				{
					
					FXmlFile ConfigFile;
					ConfigFile.LoadFile(Path);
					FXmlNode* MainTag = ConfigFile.GetRootNode();
					dataSet.etPlayersTime.clear();

					FXmlNode* ExtrapolatorCfg = MainTag->FindChildNode("ExtrapolatorCfg");
					if (ExtrapolatorCfg)
					{


						dataSet.extrapMain = (FCString::Atoi(*ExtrapolatorCfg->GetAttribute("extrapMain")) > 0 ? true : false);
						dataSet.extrapOther = (FCString::Atoi(*ExtrapolatorCfg->GetAttribute("extrapOther")) > 0 ? true : false);
						dataSet.syncObjectWait = (FCString::Atoi(*ExtrapolatorCfg->GetAttribute("syncObjectWait")) > 0 ? true : false);
						dataSet.extrapModeMain = FCString::Atoi(*ExtrapolatorCfg->GetAttribute("extrapModeMain"));
						dataSet.extrapModeOther = FCString::Atoi(*ExtrapolatorCfg->GetAttribute("extrapModeOther"));

						dataSet.extrap2DtMain = FCString::Atof(*ExtrapolatorCfg->GetAttribute("extrap2DtMain"));
						dataSet.extrap2DtOther = FCString::Atof(*ExtrapolatorCfg->GetAttribute("extrap2DtOther"));


					}
					TArray<FXmlNode*> SubTags = MainTag->GetChildrenNodes();
					for (auto SubTag : SubTags)
					{
						if (SubTag->GetTag().Equals("playerDt"))
						{
							exPlayerTime exVal;
							exVal.numPlayer = FCString::Atoi(*SubTag->GetAttribute("numPlayer"));
							exVal.extrap2Dt = FCString::Atof(*SubTag->GetAttribute("extrap2Dt"));

							exVal.extrap = (FCString::Atoi(*SubTag->GetAttribute("extrap")) > 0 ? true : false);
							exVal.extrapMode = FCString::Atoi(*SubTag->GetAttribute("extrapMode"));

							dataSet.etPlayersTime.insert(std::pair<int32, exPlayerTime>(exVal.numPlayer, exVal));
						}
					}

				}
				else
				{
					dataSet.extrapMain = true;
					dataSet.extrapOther = true;

					dataSet.extrapModeMain = 1;
					dataSet.extrapModeOther = 1;

					dataSet.extrap2DtMain = 0.02;
					dataSet.extrap2DtOther = 0.02;
				}


				Path = FPaths::ProjectDir() + "Collisions.config";
				if (FPaths::FileExists(*Path))
				{
					FXmlFile ConfigFile;
					ConfigFile.LoadFile(Path);
					FXmlNode* MainTag = ConfigFile.GetRootNode();

					FXmlNode* CollisionsCfg = MainTag->FindChildNode("CollisionsCfg");
					if (CollisionsCfg)
					{
						dataSet.enableCollisions = (FCString::Atoi(*CollisionsCfg->GetAttribute("enable")) > 0);
						dataSet.surfInfo = FCString::Atoi(*CollisionsCfg->GetAttribute("enable"));

						dataSet.FrontWheelName = *CollisionsCfg->GetAttribute("FrontWheelName");
						dataSet.RightWheelName = *CollisionsCfg->GetAttribute("RightWheelName");
						dataSet.LeftWheelName = *CollisionsCfg->GetAttribute("LeftWheelName");

						dataSet.CollisionModelName = *CollisionsCfg->GetAttribute("CollisionModelName");


					}
				}
				else
				{
					dataSet.enableCollisions = false;
					dataSet.surfInfo = 0;
				}

	
			}
			return dataSet;
		};
	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline IConfPlugin& Get()
	{
		return FModuleManager::LoadModuleChecked< IConfPlugin >( "ConfPlugin" );
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded( "ConfPlugin" );
	}
};

