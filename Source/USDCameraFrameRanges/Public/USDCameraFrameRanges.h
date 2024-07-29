// FUSDCameraFrameRangesModule.h

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "UsdWrappers/UsdAttribute.h" // Necessary include for FUsdAttribute
#include "UsdWrappers/SdfPath.h" // Necessary include for FSdfPath

class ACineCameraActor;

namespace UE
{
    class FSdfPath;
    class FUsdPrim;
    class FUsdAttribute;
}

class FToolBarBuilder;
class FMenuBuilder;
class AUsdStageActor;

struct FCameraInfo
{
    FString CameraName;
    UE::FSdfPath* PrimPath;
    UE::FUsdAttribute Translation;
    UE::FUsdAttribute Rotation;
    TArray<double> RotTimeSamples;
    TArray<double> TransTimeSamples;
    int32 StartFrame;
    int32 EndFrame;
    float FocalLength;
    float FocusDistance;
    float FStop;
    float HorizontalAperture;
    float VerticalAperture;
};

struct FMaterialInfo
{
    FString ObjName;
    FString MatName;
    bool bMatchFound = false;
    UE::FSdfPath PrimPath;
};

class FUSDCameraFrameRangesModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    
    /** This function will be bound to Command (by default it will bring up plugin window) */
    void PluginButtonClicked();
    
private:
    void RegisterMenus();

    // Property for the USD stage actor
    TObjectPtr<AUsdStageActor> StageActor;
    
    TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);
    
    static TObjectPtr<AUsdStageActor> FindUsdStageActor();
    TArray<FCameraInfo> GetCamerasFromUSDStage();


    static void TraverseAndCollectCameras(const UE::FUsdPrim& CurrentPrim, TArray<UE::FSdfPath>& OutCameraPaths);
    void TraverseAndCollectMaterials(const UE::FUsdPrim& CurrentPrim, TArray<FMaterialInfo>& MaterialNames);

    
    FReply OnDuplicateButtonClicked(FCameraInfo Camera, FString LevelSequencePath);
    FReply OnMaterialSwapButtonClicked();
    FReply OnAttributeExportButtonClicked(const FString& InputPrim, const FString& InputAttr, const FString& LevelSequencePath);
    FReply OnDisableManualFocusButtonClicked();
    
    TArray<UMaterial*>* GetAllMaterials();

    void AddCameraToLevelSequence(FString LevelSequencePath, TObjectPtr<ACineCameraActor> CameraActor, FCameraInfo Camera);
    static void DisableManualFocus(TObjectPtr<ACineCameraActor> CameraActor);

private:
    TSharedPtr<class FUICommandList> PluginCommands;
};
