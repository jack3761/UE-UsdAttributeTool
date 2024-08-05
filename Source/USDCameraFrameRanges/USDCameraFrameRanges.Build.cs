﻿// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class USDCameraFrameRanges : ModuleRules
{
	public USDCameraFrameRanges(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
				"UsdAttributeFunctionLibrary",
				"AutomationTest"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
				"EditorFramework",
				"UnrealEd",
				"ToolMenus",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"USDStage",
				"UnrealUSDWrapper",
				"USDUtilities",
				"CinematicCamera",
				"Boost", 
				"MovieScene",
				"MovieSceneTracks",
				"LevelSequence", 
				"UsdAttributeFunctionLibrary",
				"AutomationTest"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
		
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
		}
	}
}
