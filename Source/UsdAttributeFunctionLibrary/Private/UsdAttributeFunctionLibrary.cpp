// Copyright Epic Games, Inc. All Rights Reserved.

#include "UsdAttributeFunctionLibrary.h"

#include "USDIncludesStart.h"

#include "UsdWrappers/UsdStage.h"
#include "USDStageActor.h"
#include "UsdWrappers/UsdAttribute.h"
#include "UsdWrappers/UsdPrim.h"
#include "pxr/pxr.h"
#include "pxr/base/vt/value.h"
#include "pxr/base/vt/types.h"
#include "pxr/base/tf/type.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3i.h"
#include "USDIncludesEnd.h"

#define LOCTEXT_NAMESPACE "FUsdAttributeFunctionLibraryModule"

void FUsdAttributeFunctionLibraryModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
}

void FUsdAttributeFunctionLibraryModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUsdAttributeFunctionLibraryModule, UsdAttributeFunctionLibrary)