using UnrealBuildTool;
using System.IO;

public class UsdAttributeFunctionLibrary : ModuleRules
{
	public UsdAttributeFunctionLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDefinitions.Add("USDATTRIBUTELIBRARY_API=__declspec(dllexport)");
		
		// PublicDefinitions.Add("USE_USD_SDK=1");


		// Determine the engine directory
		string EngineDir = Path.GetFullPath(Target.RelativeEnginePath);

		// Paths for USD headers and libraries
		string USDIncludeDir = Path.Combine(EngineDir, "Plugins", "Importers", "USDImporter", "Source", "ThirdParty", "USD", "include");
		string USDLibsDir = Path.Combine(EngineDir, "Plugins", "Importers", "USDImporter", "Source", "ThirdParty", "USD", "lib");

		// Paths for Python headers and libraries
		string PythonSourceTPSDir = Path.Combine(EngineDir, "Source", "ThirdParty", "Python3", Target.Platform.ToString());
		string PythonBinaryTPSDir = Path.Combine(EngineDir, "Binaries", "ThirdParty", "Python3", Target.Platform.ToString());

		// Add public and private include paths for USD and Python
		PublicIncludePaths.AddRange(
			new string[] {
				USDIncludeDir,
				Path.Combine(PythonSourceTPSDir, "include"),
				// ... add other public include paths required here ...
			}
		);
		
		PrivateIncludePaths.AddRange(
			new string[] {
				USDIncludeDir,
				Path.Combine(PythonSourceTPSDir, "include"),
				// ... add other private include paths required here ...
			}
		);

		// Add public dependency modules
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"UnrealUSDWrapper",
				// ... add other public dependencies that you statically link with here ...
			}
		);
			
		// Add private dependency modules
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"USDStage",
				"UnrealUSDWrapper",
				"USDUtilities",
				"Boost",
				// ... add private dependencies that you statically link with here ...	
			}
		);

		// Add library paths and libraries for USD
		PublicSystemIncludePaths.Add(USDIncludeDir);
		PublicSystemLibraryPaths.Add(USDLibsDir);

		foreach (string UsdLib in Directory.EnumerateFiles(USDLibsDir, "*.lib", SearchOption.AllDirectories))
		{
			PublicAdditionalLibraries.Add(UsdLib);
		}

		// Add library paths and libraries for Python
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicSystemIncludePaths.Add(Path.Combine(PythonSourceTPSDir, "include"));
			PublicSystemLibraryPaths.Add(Path.Combine(PythonSourceTPSDir, "libs"));
			RuntimeDependencies.Add(Path.Combine("$(TargetOutputDir)", "python311.dll"), Path.Combine(PythonBinaryTPSDir, "python311.dll"));
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			PublicSystemIncludePaths.Add(Path.Combine(PythonSourceTPSDir, "include"));
			PublicSystemLibraryPaths.Add(Path.Combine(PythonBinaryTPSDir, "lib"));
			RuntimeDependencies.Add(Path.Combine(PythonBinaryTPSDir, "bin", "python3.11"));
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			PublicSystemIncludePaths.Add(Path.Combine(PythonSourceTPSDir, "include"));
			PublicSystemLibraryPaths.Add(Path.Combine(PythonBinaryTPSDir, "lib"));
			RuntimeDependencies.Add(Path.Combine(PythonBinaryTPSDir, "lib", "libpython3.11.dylib"));
		}

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
	}
}
