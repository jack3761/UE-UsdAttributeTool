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



UE::FUsdAttribute FUsdAttributeFunctionLibraryModule::GetUsdAttributeInternal(AUsdStageActor* StageActor,
	FString PrimName, FString AttrName)
{
	if (!StageActor)
	{
		UE_LOG(LogTemp, Error, TEXT("StageActor is null"));
		return UE::FUsdAttribute();
	}
	
	UE::FUsdStage StageBase = StageActor->GetUsdStage();

	if (!StageBase)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Usd Stage found"));
		return UE::FUsdAttribute();
	}

	UE_LOG(LogTemp, Log, TEXT("Found stage"));


	UE::FSdfPath PrimPath;
	UE::FUsdPrim root = StageBase.GetPseudoRoot();

	if (!root)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get PseudoRoot"));
		return UE::FUsdAttribute();
	}
	GetSdfPathWithName(root, PrimName, PrimPath);

	if (PrimPath.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("PrimPath is empty for PrimName: %s"), *PrimName);
		return UE::FUsdAttribute();
	}


	UE::FUsdPrim CurrentPrim = StageBase.GetPrimAtPath(PrimPath);

	if (!CurrentPrim)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Prim found at path: %s"), *PrimPath.GetString());
		return UE::FUsdAttribute();
	}
	
	const TCHAR* AttrNameTChar = *AttrName;
	UE::FUsdAttribute Attr = CurrentPrim.GetAttribute(AttrNameTChar);

	if (!Attr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Attribute found with name: %s"), AttrNameTChar);
		return UE::FUsdAttribute();
	}

	return Attr;
}

void FUsdAttributeFunctionLibraryModule::GetSdfPathWithName(UE::FUsdPrim& CurrentPrim, FString TargetName,
															   UE::FSdfPath& OutPath)
{
	if (!CurrentPrim)
	{
		UE_LOG(LogTemp, Error, TEXT("CurrentPrim is invalid"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Searching in Prim: %s"), *CurrentPrim.GetName().ToString());

	if (CurrentPrim.GetName().ToString().Equals(TargetName))
	{
		OutPath = CurrentPrim.GetPrimPath();
		UE_LOG(LogTemp, Log, TEXT("Found Prim: %s with TargetName: %s"), *CurrentPrim.GetName().ToString(), *TargetName);
		return;
	}
    
	for (UE::FUsdPrim& Child : CurrentPrim.GetChildren())
	{
		if (!Child)
		{
			UE_LOG(LogTemp, Warning, TEXT("Encountered invalid child prim"));
			continue;
		}

		GetSdfPathWithName(Child, TargetName, OutPath);

		if (!OutPath.IsEmpty())
		{
			return;
		}
	}
	UE_LOG(LogTemp, Log, TEXT("Finished searching children of Prim: %s"), *CurrentPrim.GetName().ToString());
}

template float FUsdAttributeFunctionLibraryModule::GetUsdAttributeValueInternal<float>(AUsdStageActor* StageActor, FString PrimName, FString AttrName);
template int FUsdAttributeFunctionLibraryModule::GetUsdAttributeValueInternal<int>(AUsdStageActor* StageActor, FString PrimName, FString AttrName);
template double FUsdAttributeFunctionLibraryModule::GetUsdAttributeValueInternal<double>(AUsdStageActor* StageActor, FString PrimName, FString AttrName);

template float FUsdAttributeFunctionLibraryModule::GetUsdAnimatedAttributeValueInternal<float>(AUsdStageActor* StageActor, FString PrimName, FString AttrName, double TimeSample);
template int FUsdAttributeFunctionLibraryModule::GetUsdAnimatedAttributeValueInternal<int>(AUsdStageActor* StageActor, FString PrimName, FString AttrName, double TimeSample);
template double FUsdAttributeFunctionLibraryModule::GetUsdAnimatedAttributeValueInternal<double>(AUsdStageActor* StageActor, FString PrimName, FString AttrName, double TimeSample);

template FVector FUsdAttributeFunctionLibraryModule::ConvertUsdVectorToFVector<pxr::GfVec3f>(const pxr::VtValue& pxrValue);
template FVector FUsdAttributeFunctionLibraryModule::ConvertUsdVectorToFVector<pxr::GfVec3d>(const pxr::VtValue& pxrValue);
template FVector FUsdAttributeFunctionLibraryModule::ConvertUsdVectorToFVector<pxr::GfVec3i>(const pxr::VtValue& pxrValue);
#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUsdAttributeFunctionLibraryModule, UsdAttributeFunctionLibrary)