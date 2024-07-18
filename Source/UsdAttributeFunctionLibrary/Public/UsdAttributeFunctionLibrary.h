// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "USDIncludesStart.h"
#include <pxr/base/vt/value.h>
#include "UsdWrappers/UsdAttribute.h"
#include "UsdWrappers/VtValue.h"


#include "Modules/ModuleManager.h"

namespace UE
{
	class FUsdPrim;
	class FSdfPath;
	class FUsdAttribute;
	// class FVtValue;
}

class AUsdStageActor;


class FUsdAttributeFunctionLibraryModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static UE::FUsdAttribute GetUsdAttributeInternal(AUsdStageActor* StageActor, FString PrimName, FString AttrName);
	
	// TODO implement all of these in the header instead of the cpp
	template <typename T>
	static T ExtractAttributeValue(UE::FVtValue& Value);
	
	template <typename T>
	static T GetUsdAttributeValueInternal(AUsdStageActor* StageActor, FString PrimName, FString AttrName);

	template <typename T>
	static T GetUsdAnimatedAttributeValueInternal(AUsdStageActor* StageActor, FString PrimName, FString AttrName, double TimeSample);

	template <typename T>
	static FVector ConvertUsdVectorToFVector(const pxr::VtValue& pxrValue);
	
	static void GetSdfPathWithName(UE::FUsdPrim& CurrentPrim, FString TargetName, UE::FSdfPath& OutPath);

};


template <typename T>
T FUsdAttributeFunctionLibraryModule::ExtractAttributeValue(UE::FVtValue& Value)
{
	pxr::VtValue& PxrValue = Value.GetUsdValue();
	if (PxrValue.IsHolding<T>())
	{
		T AttrValue = PxrValue.Get<T>();
		UE_LOG(LogTemp, Log, TEXT("Successfully retrieved attribute"));
		return AttrValue;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Attribute is not holding a value of specified type"));
	}

	return T();
}



template <typename T>
T FUsdAttributeFunctionLibraryModule::GetUsdAttributeValueInternal(
	AUsdStageActor* StageActor, FString PrimName, FString AttrName)
{
	UE::FUsdAttribute Attr = GetUsdAttributeInternal(StageActor, PrimName, AttrName);

	UE::FVtValue Value;

	bool bSuccess = Attr.Get(Value);

	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get value for Attribute: %s"), *Attr.GetName().ToString());
		return T();
	}

	return ExtractAttributeValue<T>(Value);
}

template <typename T>
T FUsdAttributeFunctionLibraryModule::GetUsdAnimatedAttributeValueInternal(
	AUsdStageActor* StageActor, FString PrimName, FString AttrName, double TimeSample)
{
	UE::FUsdAttribute Attr = GetUsdAttributeInternal(StageActor, PrimName, AttrName);

	UE::FVtValue Value;

	bool bSuccess = Attr.Get(Value, TimeSample);

	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get animated value for Attribute: %s"), *Attr.GetName().ToString());
		return T();
	}

	return ExtractAttributeValue<T>(Value);
}

template <typename T>
FVector FUsdAttributeFunctionLibraryModule::ConvertUsdVectorToFVector(const pxr::VtValue& pxrValue)
{
	T pxrVec = pxrValue.Get<T>();
	return FVector(static_cast<double>(pxrVec[0]), static_cast<double>(pxrVec[1]), static_cast<double>(pxrVec[2]));
}