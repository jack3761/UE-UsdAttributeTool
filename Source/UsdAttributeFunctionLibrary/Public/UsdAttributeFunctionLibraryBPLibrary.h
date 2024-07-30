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

/**
 * @brief A Blueprint Function Library to access Usd attributes from blueprints.
 * 
 * This class provides static functions that can be called from Blueprints to access
 * attributes from a UsdStageActor directly from the Usd file.
 * These functions call internal template functions to return the Usd value of that type
 * as well as values at given timesamples.
 *
 * Throughout the class, #if USE_USD_SDK is used frequently, which is required to access
 * USD functionality at runtime as of 5.4.2. Usd functionality will be updated with the
 * runtime USD changes in 5.5
 * 
 * For more information on creating custom Blueprint nodes, refer to:
 * https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
 */


#if USE_USD_SDK
namespace UE
{
	class FUsdPrim;
	class FSdfPath;
	class FUsdAttribute;
}
#endif

UCLASS()
class USDATTRIBUTELIBRARY_API UUsdAttributeFunctionLibraryBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:
#if USE_USD_SDK
    /**
     * @brief Retrieves a Usd attribute from a specified stage actor and attribute name.
     * 
     * @param StageActor The current UsdStageActor.
     * @param PrimName The name of the Usd prim.
     * @param AttrName The name of the attribute to retrieve.
     * @return UE::FUsdAttribute The requested Usd attribute.
     */
    static UE::FUsdAttribute GetUsdAttributeInternal(AUsdStageActor* StageActor, FString PrimName, FString AttrName);

    /**
     * @brief Extract the value of a useable type from the VtValue type.
     * 
     * @param Value The Usd value containing the attribute data.
     * @return The extracted value of type T.
     */
    template <typename T>
    static T ExtractAttributeValue(UE::FVtValue& Value);

    /**
     * @brief Converts a Usd vector to an Unreal FVector.
     * 
     * @param pxrValue The Usd vector value.
     * @return The corresponding FVector.
     */
    template <typename T>
    static FVector ConvertUsdVectorToFVector(const pxr::VtValue& pxrValue);

    /**
     * @brief Retrieves the value of a Usd attribute for a specified prim and attribute name.
     * 
     * @param StageActor The current UsdStageActor.
     * @param PrimName The name of the Usd prim.
     * @param AttrName The name of the attribute to retrieve.
     * @return The attribute value of type T.
     */
    template <typename T>
    static T GetUsdAttributeValueInternal(AUsdStageActor* StageActor, FString PrimName, FString AttrName);

    /**
     * @brief Retrieves the animated value of a Usd attribute at a specified time sample.
     * 
     * @param StageActor The current UsdStageActore.
     * @param PrimName The name of the Usd prim.
     * @param AttrName The name of the attribute to retrieve.
     * @param TimeSample The time sample for the animated attribute.
     * @return The animated attribute value of type T.
     */
    template <typename T>
    static T GetUsdAnimatedAttributeValueInternal(AUsdStageActor* StageActor, FString PrimName, FString AttrName, double TimeSample);
#endif

    /**
     * Blueprint Callable functions to access the templates using their given types
     */
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

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UsdAttributes")
    static int GetUsdAnimatedIntAttribute(AUsdStageActor* StageActor, FString PrimName, FString AttrName, double TimeSample);

    /**
     * @brief Converts a standard XYZ vector to the equivalent FRotator
     * 
     * @param InputVector XYZ Vector to convert to an FRotator
     * @return The converted FRotator
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UsdAttributes")
    static FRotator ConvertToUnrealRotator(FVector InputVector);

#if USE_USD_SDK
    /**
     * @brief Retrieves the SDF path of a prim with a specified name.
     * 
     * @param CurrentPrim The current Usd prim being examined.
     * @param TargetName The name of the target prim.
     * @param OutPath The path of the target prim if found.
     */
    static void GetSdfPathWithName(UE::FUsdPrim& CurrentPrim, FString TargetName, UE::FSdfPath& OutPath);
#endif
};

#if USE_USD_SDK
template <typename T>
T UUsdAttributeFunctionLibraryBPLibrary::ExtractAttributeValue(UE::FVtValue& Value)
{
    // Access the pxr VtValue from the Unreal wrapped FVtValue
    pxr::VtValue& PxrValue = Value.GetUsdValue();
    
	// Check to ensure the value is of the specified type
    if (PxrValue.IsHolding<T>())
    {
        // Access the value of the specified type from the VtValue
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

	// Check that an attribute has been found from the given inputs
    if (!Attr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Specified attribute is not holding any value"))
        return T();
    }

	// Using the Unreal wrapper of the pxr type VtValue
    UE::FVtValue Value;
    bool bSuccess = Attr.Get(Value);

    if (!bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to get value for Attribute: %s"), *Attr.GetName().ToString());
        return T();
    }

	// Required to return the useable type within Unreal
    return ExtractAttributeValue<T>(Value);
}

template <typename T>
T UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedAttributeValueInternal(
    AUsdStageActor* StageActor, FString PrimName, FString AttrName, double TimeSample)
{
    UE::FUsdAttribute Attr = GetUsdAttributeInternal(StageActor, PrimName, AttrName);
    
    // Check that an attribute has been found from the given inputs
    if (!Attr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Specified attribute is not holding any value"))
        return T();
    }

    // Using the Unreal wrapper of the pxr type VtValue
    UE::FVtValue Value;
    bool bSuccess = Attr.Get(Value, TimeSample);

    if (!bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to get animated value for Attribute: %s"), *Attr.GetName().ToString());
        return T();
    }

    // Required to return the useable type within Unreal
    return ExtractAttributeValue<T>(Value);
}

template <typename T>
FVector UUsdAttributeFunctionLibraryBPLibrary::ConvertUsdVectorToFVector(const pxr::VtValue& pxrValue)
{
    // Convert the GfVec3f, GfVec3d and GfVec3i into a standard FVector
    T pxrVec = pxrValue.Get<T>();
    return FVector(static_cast<double>(pxrVec[0]), static_cast<double>(pxrVec[1]), static_cast<double>(pxrVec[2]));
}
#endif
