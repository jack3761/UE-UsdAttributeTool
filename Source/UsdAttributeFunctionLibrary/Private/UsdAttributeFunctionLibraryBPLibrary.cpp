// Copyright Epic Games, Inc. All Rights Reserved.

#include "UsdAttributeFunctionLibraryBPLibrary.h"
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

UUsdAttributeFunctionLibraryBPLibrary::UUsdAttributeFunctionLibraryBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}





FVector UUsdAttributeFunctionLibraryBPLibrary::GetUsdVec3Attribute(AUsdStageActor* StageActor, FString PrimName,
                                                                   FString AttrName)
{
	UE::FUsdAttribute Attr = FUsdAttributeFunctionLibraryModule::GetUsdAttributeInternal(StageActor, PrimName, AttrName);

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
	
		return FUsdAttributeFunctionLibraryModule::ConvertUsdVectorToFVector<pxr::GfVec3f>(PxrValue);
	} else if (PxrValueType.IsA<pxr::GfVec3d>()) {
		UE_LOG(LogTemp, Warning, TEXT("Vec3d"));
	
		return FUsdAttributeFunctionLibraryModule::ConvertUsdVectorToFVector<pxr::GfVec3d>(PxrValue);
	} else if (PxrValueType.IsA<pxr::GfVec3i>()) {
		UE_LOG(LogTemp, Warning, TEXT("Vec3i"));
	
		return FUsdAttributeFunctionLibraryModule::ConvertUsdVectorToFVector<pxr::GfVec3i>(PxrValue);
	} else {
		UE_LOG(LogTemp, Warning, TEXT("Unsupported type for Attribute."));
		return FVector();
	}
}


float UUsdAttributeFunctionLibraryBPLibrary::GetUsdFloatAttribute(AUsdStageActor* StageActor, FString PrimName,
                                                                  FString AttrName)
{
	return FUsdAttributeFunctionLibraryModule::GetUsdAttributeValueInternal<float>(StageActor, PrimName, AttrName);

}


double UUsdAttributeFunctionLibraryBPLibrary::GetUsdDoubleAttribute(AUsdStageActor* StageActor, FString PrimName, FString AttrName)
{
	return FUsdAttributeFunctionLibraryModule::GetUsdAttributeValueInternal<double>(StageActor, PrimName, AttrName);
}

int UUsdAttributeFunctionLibraryBPLibrary::GetUsdIntAttribute(AUsdStageActor* StageActor, FString PrimName,
	FString AttrName)
{
	return FUsdAttributeFunctionLibraryModule::GetUsdAttributeValueInternal<int>(StageActor, PrimName, AttrName);
}

FVector UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedVec3Attribute(AUsdStageActor* StageActor, FString PrimName,
	FString AttrName, double TimeSample)
{
	UE::FUsdAttribute Attr = FUsdAttributeFunctionLibraryModule::GetUsdAttributeInternal(StageActor, PrimName, AttrName);

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
	
		return FUsdAttributeFunctionLibraryModule::ConvertUsdVectorToFVector<pxr::GfVec3f>(PxrValue);
	} else if (PxrValueType.IsA<pxr::GfVec3d>()) {
		UE_LOG(LogTemp, Warning, TEXT("Vec3d"));
	
		return FUsdAttributeFunctionLibraryModule::ConvertUsdVectorToFVector<pxr::GfVec3d>(PxrValue);
	} else if (PxrValueType.IsA<pxr::GfVec3i>()) {
		UE_LOG(LogTemp, Warning, TEXT("Vec3i"));
	
		return FUsdAttributeFunctionLibraryModule::ConvertUsdVectorToFVector<pxr::GfVec3i>(PxrValue);
	} else {
		UE_LOG(LogTemp, Warning, TEXT("Unsupported type for Attribute."));
		return FVector();
	}
}

float UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedFloatAttribute(AUsdStageActor* StageActor, FString PrimName,
                                                                          FString AttrName, double TimeSample)
{
	return FUsdAttributeFunctionLibraryModule::GetUsdAnimatedAttributeValueInternal<float>(StageActor, PrimName, AttrName, TimeSample);
}

double UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedDoubleAttribute(AUsdStageActor* StageActor,
	FString PrimName, FString AttrName, double TimeSample)
{
	return FUsdAttributeFunctionLibraryModule::GetUsdAnimatedAttributeValueInternal<double>(StageActor, PrimName, AttrName, TimeSample);
}

int UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedIntAttribute(AUsdStageActor* StageActor, FString PrimName,
	FString AttrName, double TimeSample)
{
	return FUsdAttributeFunctionLibraryModule::GetUsdAnimatedAttributeValueInternal<int>(StageActor, PrimName, AttrName, TimeSample);
}

FRotator UUsdAttributeFunctionLibraryBPLibrary::ConvertToUnrealRotator(FVector InputVector)
{
	return FRotator(InputVector[0], (InputVector[1]*-1)-90, InputVector[2]);
}





