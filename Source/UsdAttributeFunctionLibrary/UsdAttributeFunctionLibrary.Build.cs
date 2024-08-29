using System.IO;
using System.Collections.Generic;
using System.Linq;
using UnrealBuildTool;

public class UsdAttributeFunctionLibrary : ModuleRules
{
    public UsdAttributeFunctionLibrary(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        
        // Required to access from other modules
        PublicDefinitions.Add("USDATTRIBUTELIBRARY_API=__declspec(dllexport)");
        
        // Add public dependency modules
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "UnrealUSDWrapper",
                "Boost",
            }
        );
                
        // Add private dependency modules
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Projects",
                "EditorFramework",
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "USDStage",
                "UnrealUSDWrapper",
                "USDUtilities",
                "Boost",
            }
        );
        
        if (Target.Type == TargetRules.TargetType.Editor)
        {
            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "UnrealEd",
                    "EditorSubsystem"
                }
            );
        }

        
        // Required for runtime usd functionality
        bool bEnableUsdSdk = EnableUsdSdk(Target);

        if (bEnableUsdSdk)
        {
            // Determine the engine directory
            string EngineDir = Path.GetFullPath(Target.RelativeEnginePath);

            // Paths for USD headers and libraries
            string USDIncludeDir = Path.Combine(EngineDir, "Plugins", "Runtime", "USDCore", "Source", "ThirdParty", "USD", "include");
            string USDLibsDir = Path.Combine(EngineDir, "Plugins", "Runtime", "USDCore", "Source", "ThirdParty", "USD", "lib");

            // Add public and private include paths for USD
            PublicIncludePaths.Add(USDIncludeDir);
            PrivateIncludePaths.Add(USDIncludeDir);

            

            // Add library paths and libraries for USD
            PublicSystemIncludePaths.Add(USDIncludeDir);
            PublicSystemLibraryPaths.Add(USDLibsDir);

            foreach (string UsdLib in Directory.EnumerateFiles(USDLibsDir, "*.lib", SearchOption.AllDirectories))
            {
                PublicAdditionalLibraries.Add(UsdLib);
            }

            // Platform-specific Python and USD libraries setup
            SetUpPlatformSpecificLibraries(Target, EngineDir);
        }
        else
        {
            PublicDefinitions.Add("USE_USD_SDK=0");
            PublicDefinitions.Add("USD_USES_SYSTEM_MALLOC=0");
        }
    }

    private void SetUpPlatformSpecificLibraries(ReadOnlyTargetRules Target, string EngineDir)
    {
        var PythonSourceTPSDir = Path.Combine(EngineDir, "Source", "ThirdParty", "Python3", Target.Platform.ToString());
        var PythonBinaryTPSDir = Path.Combine(EngineDir, "Binaries", "ThirdParty", "Python3", Target.Platform.ToString());

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add("USD_USES_SYSTEM_MALLOC=1");

            // Python3
            PublicSystemIncludePaths.Add(Path.Combine(PythonSourceTPSDir, "include"));
            PublicSystemLibraryPaths.Add(Path.Combine(PythonSourceTPSDir, "libs"));
            RuntimeDependencies.Add(Path.Combine("$(TargetOutputDir)", "python311.dll"), Path.Combine(PythonBinaryTPSDir, "python311.dll"));

            // USD
            foreach (string UsdLib in Directory.EnumerateFiles(Path.Combine(EngineDir, "Plugins", "Runtime", "USDCore", "Source", "ThirdParty", "USD", "lib"), "*.lib", SearchOption.AllDirectories))
            {
                PublicAdditionalLibraries.Add(UsdLib);
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            PublicDefinitions.Add("USD_USES_SYSTEM_MALLOC=0");

            // Python3
            PublicSystemIncludePaths.Add(Path.Combine(PythonSourceTPSDir, "include"));
            PublicSystemLibraryPaths.Add(Path.Combine(PythonBinaryTPSDir, "lib"));
            RuntimeDependencies.Add(Path.Combine(PythonBinaryTPSDir, "bin", "python3.11"));

            // USD
            var USDBinDir = Path.Combine(EngineDir, "Plugins", "Runtime", "USDCore", "Source", "ThirdParty", "USD", "bin");
            foreach (string LibPath in Directory.EnumerateFiles(USDBinDir, "*.so", SearchOption.AllDirectories))
            {
                PublicAdditionalLibraries.Add(LibPath);
                RuntimeDependencies.Add(LibPath);
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicDefinitions.Add("USD_USES_SYSTEM_MALLOC=0");

            // Python3
            PublicSystemIncludePaths.Add(Path.Combine(PythonSourceTPSDir, "include"));
            PublicSystemLibraryPaths.Add(Path.Combine(PythonBinaryTPSDir, "lib"));
            RuntimeDependencies.Add(Path.Combine(PythonBinaryTPSDir, "lib", "libpython3.11.dylib"));

            // USD
            var USDBinDir = Path.Combine(EngineDir, "Plugins", "Runtime", "USDCore", "Source", "ThirdParty", "USD", "bin");
            foreach (string LibPath in Directory.EnumerateFiles(USDBinDir, "*.dylib", SearchOption.AllDirectories))
            {
                PublicDelayLoadDLLs.Add(LibPath);
                RuntimeDependencies.Add(LibPath);
            }
        }
    }

    bool EnableUsdSdk(ReadOnlyTargetRules Target)
    {
        bool bEnableUsdSdk = (
            Target.WindowsPlatform.Compiler != WindowsCompiler.Clang &&
            Target.WindowsPlatform.Compiler != WindowsCompiler.Intel &&
            Target.StaticAnalyzer == StaticAnalyzer.None
        );

        if (Target.GlobalDefinitions.Contains("UE_INCLUDE_TOOL=1"))
        {
            bEnableUsdSdk = false;
        }

        if (bEnableUsdSdk && Target.LinkType == TargetLinkType.Monolithic && !Target.GlobalDefinitions.Contains("FORCE_ANSI_ALLOCATOR=1") && !Target.GlobalDefinitions.Contains("UE_USE_MALLOC_FILL_BYTES=0"))
        {
            PublicDefinitions.Add("USD_FORCE_DISABLED=1");
            bEnableUsdSdk = false;
        }
        else
        {
            PublicDefinitions.Add("USD_FORCE_DISABLED=0");
        }

        return bEnableUsdSdk;
    }
}
