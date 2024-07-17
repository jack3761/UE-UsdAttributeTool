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


FVector UUsdAttributeFunctionLibraryBPLibrary::GetUsdVec3Attribute(AUsdStageActor* StageActor, FString PrimName,
                                                                   FString AttrName)
{
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
}


float UUsdAttributeFunctionLibraryBPLibrary::GetUsdFloatAttribute(AUsdStageActor* StageActor, FString PrimName,
                                                                  FString AttrName)
{
	return GetUsdAttributeValueInternal<float>(StageActor, PrimName, AttrName);

}


double UUsdAttributeFunctionLibraryBPLibrary::GetUsdDoubleAttribute(AUsdStageActor* StageActor, FString PrimName, FString AttrName)
{
	return GetUsdAttributeValueInternal<double>(StageActor, PrimName, AttrName);
}

int UUsdAttributeFunctionLibraryBPLibrary::GetUsdIntAttribute(AUsdStageActor* StageActor, FString PrimName,
	FString AttrName)
{
	return GetUsdAttributeValueInternal<int>(StageActor, PrimName, AttrName);
}

FVector UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedVec3Attribute(AUsdStageActor* StageActor, FString PrimName,
	FString AttrName, double TimeSample)
{
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
}

float UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedFloatAttribute(AUsdStageActor* StageActor, FString PrimName,
                                                                          FString AttrName, double TimeSample)
{
	return GetUsdAnimatedAttributeValueInternal<float>(StageActor, PrimName, AttrName, TimeSample);
}

double UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedDoubleAttribute(AUsdStageActor* StageActor,
	FString PrimName, FString AttrName, double TimeSample)
{
	return GetUsdAnimatedAttributeValueInternal<double>(StageActor, PrimName, AttrName, TimeSample);
}

int UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedIntAttribute(AUsdStageActor* StageActor, FString PrimName,
	FString AttrName, double TimeSample)
{
	return GetUsdAnimatedAttributeValueInternal<int>(StageActor, PrimName, AttrName, TimeSample);
}

FRotator UUsdAttributeFunctionLibraryBPLibrary::ConvertToUnrealRotator(FVector InputVector)
{
	return FRotator(InputVector[0], (InputVector[1]*-1)-90, InputVector[2]);
}


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

template float UUsdAttributeFunctionLibraryBPLibrary::GetUsdAttributeValueInternal<float>(AUsdStageActor* StageActor, FString PrimName, FString AttrName);
template int UUsdAttributeFunctionLibraryBPLibrary::GetUsdAttributeValueInternal<int>(AUsdStageActor* StageActor, FString PrimName, FString AttrName);
template double UUsdAttributeFunctionLibraryBPLibrary::GetUsdAttributeValueInternal<double>(AUsdStageActor* StageActor, FString PrimName, FString AttrName);

template float UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedAttributeValueInternal<float>(AUsdStageActor* StageActor, FString PrimName, FString AttrName, double TimeSample);
template int UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedAttributeValueInternal<int>(AUsdStageActor* StageActor, FString PrimName, FString AttrName, double TimeSample);
template double UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedAttributeValueInternal<double>(AUsdStageActor* StageActor, FString PrimName, FString AttrName, double TimeSample);

template FVector UUsdAttributeFunctionLibraryBPLibrary::ConvertUsdVectorToFVector<pxr::GfVec3f>(const pxr::VtValue& pxrValue);
template FVector UUsdAttributeFunctionLibraryBPLibrary::ConvertUsdVectorToFVector<pxr::GfVec3d>(const pxr::VtValue& pxrValue);
template FVector UUsdAttributeFunctionLibraryBPLibrary::ConvertUsdVectorToFVector<pxr::GfVec3i>(const pxr::VtValue& pxrValue);
