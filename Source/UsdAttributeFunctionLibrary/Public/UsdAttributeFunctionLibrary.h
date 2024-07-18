// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#ifndef USDATTRIBUTELIBRARY_API
#ifdef USDATTRIBUTELIBRARY_EXPORTS
#define USDATTRIBUTELIBRARY_API __declspec(dllexport)
#else
#define USDATTRIBUTELIBRARY_API __declspec(dllimport)
#endif
#endif

#include "Modules/ModuleManager.h"

class FUsdAttributeFunctionLibraryModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
