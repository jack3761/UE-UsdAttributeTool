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

/**
 * @struct FCameraInfo
 * @brief Contains information about a camera found within the Usd Stage, including its attributes, time samples, and other relevant data.
 */
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

/**
 * @struct FMaterialInfo
 * @brief Contains information about a material found within the Usd Stage and if there is a matching material within the Unreal project
 */
struct FMaterialInfo
{
    FString ObjName;
    FString MatName;
    bool bMatchFound = false;
    UE::FSdfPath PrimPath;
};

/**
 * @class FUSDCameraFrameRangesModule
 * @brief Manages the USD camera frame ranges and provides functionality for interacting with USD data.
 *        Inherits from IModuleInterface to integrate with the Unreal Engine module system.
 */
class FUSDCameraFrameRangesModule : public IModuleInterface
{
public:
    /** 
     * @brief Called when the module is starting up.
     *        Initialize any resources or state required for the module.
     */
    virtual void StartupModule() override;

    /**
     * @brief Called when the module is shutting down.
     *        Clean up any resources or state used by the module.
     */
    virtual void ShutdownModule() override;

    /**
     * @brief Handler for the plugin button click event.
     *        By default, this will bring up the plugin window.
     */
    void PluginButtonClicked();

    

private:
    /**
     * @brief Registers the plugin's menus and commands.
     */
    void RegisterMenus();

    /**
     * @brief Property for the Usd stage actor.
     */
    TObjectPtr<AUsdStageActor> StageActor;

    /**
     * @brief Spawns the plugin tab in the editor.
     * @param SpawnTabArgs Arguments for spawning the tab.
     * @return A shared reference to the newly spawned tab.
     */
    TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

    /**
     * @brief Finds the Usd stage actor in the current level.
     * @return A pointer to the Usd stage actor if found, otherwise nullptr.
     */
    static TObjectPtr<AUsdStageActor> FindUsdStageActor();

    /**
     * @brief Retrieves all cameras from the Usd stage.
     * @return An array of camera information extracted from the USD stage.
     */
    TArray<FCameraInfo> GetCamerasFromUSDStage();

    /**
        * @brief Handles the button click event for duplicating a Usd camera.
        * @param Camera The Usd camera information to be duplicated.
        * @param LevelSequencePath The path to the level sequence where the camera will be duplicated.
        * @return The reply indicating the result of the button click.
        */
    FReply OnDuplicateButtonClicked(FCameraInfo Camera, FString LevelSequencePath);

    /**
     * @brief Handles the button click event for swapping materials.
     * @return The reply indicating the result of the button click.
     */
    FReply OnMaterialSwapButtonClicked();

    /**
     * @brief Handles the button click event for exporting attributes.
     * @param InputPrim The USD prim from which attributes will be exported.
     * @param InputAttr The specific attribute to export.
     * @param LevelSequencePath The path to the level sequence where the export will occur.
     * @return The reply indicating the result of the button click.
     */
    FReply OnAttributeExportButtonClicked(const FString& InputPrim, const FString& InputAttr, const FString& LevelSequencePath);

    /**
     * @brief Handles the button click event for disabling manual focus.
     * @return The reply indicating the result of the button click.
     */
    FReply OnDisableManualFocusButtonClicked();

    /**
     * @brief Traverses the Usd prim hierarchy and collects camera paths.
     * @param CurrentPrim The current Usd prim being traversed.
     * @param OutCameraPaths Array to store the collected camera paths.
     */
    static void TraverseAndCollectCameras(const UE::FUsdPrim& CurrentPrim, TArray<UE::FSdfPath>& OutCameraPaths);

    /**
     * @brief Traverses the Usd prim hierarchy and collects material information.
     * @param CurrentPrim The current Usd prim being traversed.
     * @param MaterialNames Array to store the collected material information.
     */
    void TraverseAndCollectMaterials(const UE::FUsdPrim& CurrentPrim, TArray<FMaterialInfo>& MaterialNames);

    /**
     * @brief Retrieves all materials from the project.
     * @return An array of pointers to all materials in the project.
     */
    TArray<UMaterial*>* GetAllMaterials();

    /**
     * @brief Adds a camera to a level sequence.
     * @param LevelSequencePath The path to the level sequence where the camera will be added.
     * @param CameraActor The CineCameraActor to be added.
     * @param Camera The camera information to be added.
     */
    void AddCameraToLevelSequence(FString LevelSequencePath, TObjectPtr<ACineCameraActor> CameraActor, FCameraInfo Camera);

    /**
     * @brief Disables manual focus on a CineCameraActor.
     * @param CameraActor The CineCameraActor on which manual focus will be disabled.
     */
    static void DisableManualFocus(TObjectPtr<ACineCameraActor> CameraActor);

private:
    /**
     * @brief The command list for plugin UI commands.
     */
    TSharedPtr<class FUICommandList> PluginCommands;
};
