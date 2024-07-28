// Copyright Epic Games, Inc. All Rights Reserved.

#include "UsdAttributeFunctionLibraryBPLibrary.h"
#include "UsdAttributeFunctionLibrary.h"

#if USE_USD_SDK
#include "USDIncludesStart.h"

#include "UsdWrappers/UsdStage.h"
#include "USDStageActor.h"
#include "UsdWrappers/UsdAttribute.h"
#include "UsdWrappers/UsdPrim.h"
#include "pxr/base/vt/value.h"
#include "pxr/base/vt/types.h"
#include "pxr/base/tf/type.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3i.h"
#include "USDIncludesEnd.h"
#endif

UUsdAttributeFunctionLibraryBPLibrary::UUsdAttributeFunctionLibraryBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}
#if USE_USD_SDK
UE::FUsdAttribute UUsdAttributeFunctionLibraryBPLibrary::GetUsdAttributeInternal(AUsdStageActor* StageActor,
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
#endif



FVector UUsdAttributeFunctionLibraryBPLibrary::GetUsdVec3Attribute(AUsdStageActor* StageActor, FString PrimName,
                                                                   FString AttrName)
{
#if USE_USD_SDK
	UE::FUsdAttribute Attr = GetUsdAttributeInternal(StageActor, PrimName, AttrName);

	UE::FVtValue Value;

	bool bSuccess = Attr.Get(Value);

	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get animated value for Attribute: %s"), *Attr.GetName().ToString());
		return FVector();
	}

	
	pxr::VtValue& PxrValue = Value.GetUsdValue();
	pxr::TfType PxrValueType = PxrValue.GetType();

	if (PxrValueType.IsA<pxr::GfVec3f>()) {
		UE_LOG(LogTemp, Warning, TEXT("Vec3f"));
	
		return ConvertUsdVectorToFVector<pxr::GfVec3f>(PxrValue);
	} else if (PxrValueType.IsA<pxr::GfVec3d>()) {
		UE_LOG(LogTemp, Warning, TEXT("Vec3d"));
	
		return ConvertUsdVectorToFVector<pxr::GfVec3d>(PxrValue);
	} else if (PxrValueType.IsA<pxr::GfVec3i>()) {
		UE_LOG(LogTemp, Warning, TEXT("Vec3i"));
	
		return ConvertUsdVectorToFVector<pxr::GfVec3i>(PxrValue);
	} else {
		UE_LOG(LogTemp, Warning, TEXT("Unsupported type for Attribute."));
		return FVector();
	}
	
#else
	UE_LOG(LogTemp, Warning, TEXT("USE_USD_SDK not enabled, unable to access UsdValue"))
	return FVector();
#endif
	
}


float UUsdAttributeFunctionLibraryBPLibrary::GetUsdFloatAttribute(AUsdStageActor* StageActor, FString PrimName,
                                                                  FString AttrName)
{
#if USE_USD_SDK
	return GetUsdAttributeValueInternal<float>(StageActor, PrimName, AttrName);
#else
	return 0.0;
#endif

}


double UUsdAttributeFunctionLibraryBPLibrary::GetUsdDoubleAttribute(AUsdStageActor* StageActor, FString PrimName, FString AttrName)
{
#if USE_USD_SDK
	return GetUsdAttributeValueInternal<double>(StageActor, PrimName, AttrName);
#else
	return 0.0;
#endif
}

int UUsdAttributeFunctionLibraryBPLibrary::GetUsdIntAttribute(AUsdStageActor* StageActor, FString PrimName,
	FString AttrName)
{
#if USE_USD_SDK

	return GetUsdAttributeValueInternal<int>(StageActor, PrimName, AttrName);
#else
	return 0;
#endif
}

FVector UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedVec3Attribute(AUsdStageActor* StageActor, FString PrimName,
	FString AttrName, double TimeSample)
{
#if USE_USD_SDK
	UE::FUsdAttribute Attr = GetUsdAttributeInternal(StageActor, PrimName, AttrName);

	UE::FVtValue Value;

	bool bSuccess = Attr.Get(Value, TimeSample);

	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get animated value for Attribute: %s"), *Attr.GetName().ToString());
		return FVector();
	}

	pxr::VtValue& PxrValue = Value.GetUsdValue();
	pxr::TfType PxrValueType = PxrValue.GetType();

	if (PxrValueType.IsA<pxr::GfVec3f>()) {
		UE_LOG(LogTemp, Warning, TEXT("Vec3f"));
	
		return ConvertUsdVectorToFVector<pxr::GfVec3f>(PxrValue);
	} else if (PxrValueType.IsA<pxr::GfVec3d>()) {
		UE_LOG(LogTemp, Warning, TEXT("Vec3d"));
	
		return ConvertUsdVectorToFVector<pxr::GfVec3d>(PxrValue);
	} else if (PxrValueType.IsA<pxr::GfVec3i>()) {
		UE_LOG(LogTemp, Warning, TEXT("Vec3i"));
	
		return ConvertUsdVectorToFVector<pxr::GfVec3i>(PxrValue);
	} else {
		UE_LOG(LogTemp, Warning, TEXT("Unsupported type for Attribute."));
		return FVector();
	}
#else
	UE_LOG(LogTemp, Warning, TEXT("USE_USD_SDK not enabled, unable to access UsdValue"))
	return FVector();
#endif
}

float UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedFloatAttribute(AUsdStageActor* StageActor, FString PrimName,
                                                                          FString AttrName, double TimeSample)
{
#if USE_USD_SDK
	if (float FoundValue = GetUsdAnimatedAttributeValueInternal<float>(StageActor, PrimName, AttrName, TimeSample))
	{
		return FoundValue;
	}
	return 0.0;
	
#else
	return 0.0;
#endif
}

double UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedDoubleAttribute(AUsdStageActor* StageActor,
	FString PrimName, FString AttrName, double TimeSample)
{
#if USE_USD_SDK
	if (double FoundValue = GetUsdAnimatedAttributeValueInternal<double>(StageActor, PrimName, AttrName, TimeSample))
	{
		return FoundValue;
	}
	return 0.0;
#else
	return 0.0;
#endif
}

int UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedIntAttribute(AUsdStageActor* StageActor, FString PrimName,
	FString AttrName, double TimeSample)
{
#if USE_USD_SDK
	if (int FoundValue = GetUsdAnimatedAttributeValueInternal<int>(StageActor, PrimName, AttrName, TimeSample))
	{
		return FoundValue;
	}
	return 0;
#else
	return 0;
#endif
}

FRotator UUsdAttributeFunctionLibraryBPLibrary::ConvertToUnrealRotator(FVector InputVector)
{
	return FRotator(InputVector[0], (InputVector[1]*-1)-90, InputVector[2]);
}

#if USE_USD_SDK
void UUsdAttributeFunctionLibraryBPLibrary::GetSdfPathWithName(UE::FUsdPrim& CurrentPrim, FString TargetName,
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
#endif

#if USE_USD_SDK
template float UUsdAttributeFunctionLibraryBPLibrary::GetUsdAttributeValueInternal<float>(AUsdStageActor* StageActor, FString PrimName, FString AttrName);
template int UUsdAttributeFunctionLibraryBPLibrary::GetUsdAttributeValueInternal<int>(AUsdStageActor* StageActor, FString PrimName, FString AttrName);
template double UUsdAttributeFunctionLibraryBPLibrary::GetUsdAttributeValueInternal<double>(AUsdStageActor* StageActor, FString PrimName, FString AttrName);

template float UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedAttributeValueInternal<float>(AUsdStageActor* StageActor, FString PrimName, FString AttrName, double TimeSample);
template int UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedAttributeValueInternal<int>(AUsdStageActor* StageActor, FString PrimName, FString AttrName, double TimeSample);
template double UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedAttributeValueInternal<double>(AUsdStageActor* StageActor, FString PrimName, FString AttrName, double TimeSample);

template FVector UUsdAttributeFunctionLibraryBPLibrary::ConvertUsdVectorToFVector<pxr::GfVec3f>(const pxr::VtValue& pxrValue);
template FVector UUsdAttributeFunctionLibraryBPLibrary::ConvertUsdVectorToFVector<pxr::GfVec3d>(const pxr::VtValue& pxrValue);
template FVector UUsdAttributeFunctionLibraryBPLibrary::ConvertUsdVectorToFVector<pxr::GfVec3i>(const pxr::VtValue& pxrValue);
#endif

