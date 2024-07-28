// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#if USE_USD_SDK
#include "USDIncludesStart.h"
#include <pxr/base/vt/value.h>
#include "pxr/base/vt/types.h"
#include "pxr/base/tf/type.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3i.h"
#include "UsdWrappers/VtValue.h"
#include "UsdWrappers/UsdAttribute.h"
#include "USDIncludesEnd.h"
#endif

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UsdAttributeFunctionLibraryBPLibrary.generated.h"


/* 
*	Function library class.
*	Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
*
*	When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
*	BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
*	BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
*	DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
*				Its lets you name the node using characters not allowed in C++ function names.
*	CompactNodeTitle - the word(s) that appear on the node.
*	Keywords -	the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu. 
*				Good example is "Print String" node which you can find also by using keyword "log".
*	Category -	the category your node will be under in the Blueprint drop-down menu.
*
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/
#if USE_USD_SDK
namespace UE
{
	class FUsdPrim;
	class FSdfPath;
	class FUsdAttribute;
	
}
#endif

class AUsdStageActor;

UCLASS()
class USDATTRIBUTELIBRARY_API UUsdAttributeFunctionLibraryBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()
	
public:
#if USE_USD_SDK
	static UE::FUsdAttribute GetUsdAttributeInternal(AUsdStageActor* StageActor, FString PrimName, FString AttrName);
	
	template <typename T>
	static T ExtractAttributeValue(UE::FVtValue& Value);
	
	template <typename T>
	static FVector ConvertUsdVectorToFVector(const pxr::VtValue& pxrValue);

	
	template <typename T>
	static T GetUsdAttributeValueInternal(AUsdStageActor* StageActor, FString PrimName, FString AttrName);

	template <typename T>
	static T GetUsdAnimatedAttributeValueInternal(AUsdStageActor* StageActor, FString PrimName, FString AttrName, double TimeSample);
#endif

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UsdAttributes")
	static FVector GetUsdVec3Attribute(AUsdStageActor* StageActor, FString PrimName, FString AttrName);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UsdAttributes")
	static float GetUsdFloatAttribute(AUsdStageActor* StageActor, FString PrimName, FString AttrName);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UsdAttributes")
	static double GetUsdDoubleAttribute(AUsdStageActor* StageActor, FString PrimName, FString AttrName);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "USD Attributes")
	static int GetUsdIntAttribute(AUsdStageActor* StageActor, FString PrimName, FString AttrName);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UsdAttributes")
	static FVector GetUsdAnimatedVec3Attribute(AUsdStageActor* StageActor, FString PrimName, FString AttrName, double TimeSample);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UsdAttributes")
	static float GetUsdAnimatedFloatAttribute(AUsdStageActor* StageActor, FString PrimName, FString AttrName, double TimeSample);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UsdAttributes")
	static double GetUsdAnimatedDoubleAttribute(AUsdStageActor* StageActor, FString PrimName, FString AttrName, double TimeSample);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "USD Attributes")
	static int GetUsdAnimatedIntAttribute(AUsdStageActor* StageActor, FString PrimName, FString AttrName, double TimeSample);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "USD Attributes")
	static FRotator ConvertToUnrealRotator(FVector InputVector);

#if USE_USD_SDK
	static void GetSdfPathWithName(UE::FUsdPrim& CurrentPrim, FString TargetName, UE::FSdfPath& OutPath);
#endif
};

#if USE_USD_SDK
template <typename T>
T UUsdAttributeFunctionLibraryBPLibrary::ExtractAttributeValue(UE::FVtValue& Value)
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
T UUsdAttributeFunctionLibraryBPLibrary::GetUsdAttributeValueInternal(
	AUsdStageActor* StageActor, FString PrimName, FString AttrName)
{
	UE::FUsdAttribute Attr = GetUsdAttributeInternal(StageActor, PrimName, AttrName);
	
	if (!Attr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Specified attribute is not holding any value"))
		return T();
	}

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
T UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedAttributeValueInternal(
	AUsdStageActor* StageActor, FString PrimName, FString AttrName, double TimeSample)
{
	UE::FUsdAttribute Attr = GetUsdAttributeInternal(StageActor, PrimName, AttrName);
	
	if (!Attr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Specified attribute is not holding any value"))
		return T();
	}
	
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
FVector UUsdAttributeFunctionLibraryBPLibrary::ConvertUsdVectorToFVector(const pxr::VtValue& pxrValue)
{
	T pxrVec = pxrValue.Get<T>();
	return FVector(static_cast<double>(pxrVec[0]), static_cast<double>(pxrVec[1]), static_cast<double>(pxrVec[2]));
}
#endif

