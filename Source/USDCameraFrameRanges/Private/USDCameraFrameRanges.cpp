// Copyright Epic Games, Inc. All Rights Reserved.

#include "USDCameraFrameRanges.h"

#include "USDCameraFrameRangesStyle.h"
#include "USDCameraFrameRangesCommands.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "USDStageActor.h"
#include "CineCameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "Editor.h"

#include "LevelSequence.h"
#include "MovieScene.h"
#include "LevelSequencePlayer.h"

#include "USDIncludesStart.h"
#include "UsdWrappers/UsdStage.h"
#include "UsdWrappers/UsdPrim.h"
#include "UsdWrappers/UsdAttribute.h"
#include "UsdWrappers/UsdRelationship.h"
#include "pxr/pxr.h"
#include "pxr/usd/usd/attribute.h"
#include "pxr/base/vt/value.h"
#include "pxr/usd/usdGeom/xform.h"
#include "USDIncludesEnd.h"

#include "Engine/ObjectLibrary.h"
#include "Tracks/MovieScene3DTransformTrack.h"
#include "Sections/MovieScene3DTransformSection.h"
#include "CineCameraComponent.h"
#include "UsdAttributeFunctionLibraryBPLibrary.h"
#include "Sections/MovieSceneFloatSection.h"
#include "Tracks/MovieSceneFloatTrack.h"

static const FName USDCameraFrameRangesTabName("USDCameraFrameRanges");

#define LOCTEXT_NAMESPACE "FUSDCameraFrameRangesModule"

void FUSDCameraFrameRangesModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FUSDCameraFrameRangesStyle::Initialize();
	FUSDCameraFrameRangesStyle::ReloadTextures();

	FUSDCameraFrameRangesCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FUSDCameraFrameRangesCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FUSDCameraFrameRangesModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FUSDCameraFrameRangesModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(USDCameraFrameRangesTabName, FOnSpawnTab::CreateRaw(this, &FUSDCameraFrameRangesModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FUSDCameraFrameRangesTabTitle", "USDCameraFrameRanges"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FUSDCameraFrameRangesModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FUSDCameraFrameRangesStyle::Shutdown();

	FUSDCameraFrameRangesCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(USDCameraFrameRangesTabName);
}

void FUSDCameraFrameRangesModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FUSDCameraFrameRangesCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FUSDCameraFrameRangesCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

void FUSDCameraFrameRangesModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(USDCameraFrameRangesTabName);    
}


/**
 * @brief Spawns the plugin tab for the USD Camera Frame Ranges module.
 * Creates the UI elements and layout for the plugin tab by initialising
 * the stage actor, retrieving camera information from the Usd stage,
 * and setting up input fields and buttons for user interaction.
 * 
 * @param SpawnTabArgs Arguments for spawning the tab.
 * @return A shared reference to the created SDockTab widget.
 */
TSharedRef<SDockTab> FUSDCameraFrameRangesModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
    // Find the Usd stage actor in the scene
    StageActor = FindUsdStageActor();
    if (!StageActor)
    {
        // If the Usd stage actor is not found, display a message
        return SNew(SDockTab)
            .TabRole(ETabRole::NomadTab)
            [
                SNew(SBox)
                .Padding(20)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("USD Stage Actor not found. Please ensure a USD Stage Actor is present in the scene.")))
                ]
            ];
    }

    // Retrieve camera information from the Usd stage
    TArray<FCameraInfo> Cameras = GetCamerasFromUSDStage();
    if (Cameras.Num() == 0)
    {
        // If no cameras are found, display a message
        return SNew(SDockTab)
            .TabRole(ETabRole::NomadTab)
            [
                SNew(SBox)
                .Padding(20)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("No cameras found in the USD Stage. Please ensure there are cameras in the USD Stage.")))
                ]
            ];
    }

	FindCameraMainFrameRanges(Cameras);

    // Create input text boxes for sequence path, prim name, and attribute name
    TSharedPtr<SEditableTextBox> SequenceInputTextBox = SNew(SEditableTextBox);
    TSharedPtr<SEditableTextBox> PrimInputTextBox = SNew(SEditableTextBox);
    TSharedPtr<SEditableTextBox> AttrInputTextBox = SNew(SEditableTextBox);

    // Create vertical boxes for level sequence buttons and camera list
    TSharedPtr<SVerticalBox> LevelSequenceButtons = SNew(SVerticalBox);
    TSharedPtr<SVerticalBox> CameraList = SNew(SVerticalBox);

    // Add input fields with labels to the level sequence buttons
    LevelSequenceButtons->AddSlot()
    .Padding(10)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(5)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Level sequence path:")))
        ]
        + SHorizontalBox::Slot()
        .FillWidth(1.0)
        .Padding(5)
        [
            SequenceInputTextBox.ToSharedRef()
        ]
    ];

    // Add prim name and attribute name fields on the same row
    LevelSequenceButtons->AddSlot()
    .Padding(10)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(5)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Prim name:")))
        ]
        + SHorizontalBox::Slot()
        .FillWidth(1.0)
        .Padding(5)
        [
            PrimInputTextBox.ToSharedRef()
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(5)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Attribute name:")))
        ]
        + SHorizontalBox::Slot()
        .FillWidth(1.0)
        .Padding(5)
        [
            AttrInputTextBox.ToSharedRef()
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(5)
        [
            SNew(SButton)
            .Text(FText::FromString(TEXT("Export to sequence")))
            .OnClicked_Lambda([this, PrimInputTextBox, AttrInputTextBox, SequenceInputTextBox]()
            {
                return OnAttributeExportButtonClicked(PrimInputTextBox->GetText().ToString(), AttrInputTextBox->GetText().ToString(), SequenceInputTextBox->GetText().ToString());
            })
        ]
    ];

    // Add header for the camera list
    CameraList->AddSlot()
    .Padding(2)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .FillWidth(0.25)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Camera name")))
        ]
        + SHorizontalBox::Slot()
        .FillWidth(0.25)
        [
            SNew(STextBlock)
            .Text(FText::FromString("Frame range"))
        ]
    	+ SHorizontalBox::Slot()
	    .FillWidth(0.25)
        [
            SNew(STextBlock)
            .Text(FText::FromString("Camera Main Frame range"))
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Create CineCameraActor")))
        ]
    ];

    // Loop through Cameras array and create a row widget for each camera
    for (const FCameraInfo& Camera : Cameras)
    {
        CameraList->AddSlot()
        .Padding(2)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .FillWidth(0.25)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(Camera.CameraName))
                ]
                + SHorizontalBox::Slot()
                .FillWidth(0.25)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(FString::Printf(TEXT("%d - %d"), Camera.StartFrame, Camera.EndFrame)))
                ]
            	+ SHorizontalBox::Slot()
				.FillWidth(0.25)
				[
					Camera.inCameraMain ? SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("%d - %d"), Camera.CameraMainStartFrame, Camera.CameraMainEndFrame)))
					: SNew(STextBlock)
						.Text(FText::FromString("n/a"))
				]
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                    .Text(FText::FromString(TEXT("Duplicate")))
                    .OnClicked_Lambda([this, Camera, SequenceInputTextBox]()
                    {
                        return OnDuplicateButtonClicked(Camera, SequenceInputTextBox->GetText().ToString());
                    })
                ]
            ]
        ];
    }

    // Create and return the final tab layout with scroll boxes and buttons
    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(SScrollBox)
            + SScrollBox::Slot()
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(20)
                [
                    LevelSequenceButtons.ToSharedRef()
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(20)
                [
                    SNew(SScrollBox)
                    + SScrollBox::Slot()
                    [
                        SNew(SBorder)
                        .Padding(FMargin(20))
                        [
                            CameraList.ToSharedRef()
                        ]
                    ]
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(10)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(10)
                    [
                        SNew(SButton)
                        .Text(FText::FromString(TEXT("Material swap")))
                        .OnClicked(FOnClicked::CreateRaw(this, &FUSDCameraFrameRangesModule::OnMaterialSwapButtonClicked))
                    ]
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(10)
                    [
                        SNew(SButton)
                        .Text(FText::FromString(TEXT("Disable Manual Focus")))
                        .OnClicked(FOnClicked::CreateRaw(this, &FUSDCameraFrameRangesModule::OnDisableManualFocusButtonClicked))
                    ]
                ]
            ]
        ];
}

/**
 * @brief Finds the first Usd Stage Actor in the current level.
 * Searches for an AUsdStageActor in the current editor world context.
 * If there are multiple then only then only the first one is returned.
 * 
 * @return A pointer to the found AUsdStageActor, or nullptr if not found or multiple actors are present.
 */
TObjectPtr<AUsdStageActor> FUSDCameraFrameRangesModule::FindUsdStageActor()
{
	UE_LOG(LogTemp, Log, TEXT("Searching for UsdStageActor"));

	// Check if GEditor is available
	if (!GEditor)
	{
		UE_LOG(LogTemp, Warning, TEXT("GEditor is not available"));
		return nullptr;
	}
    
	// Get the current editor world
	UWorld* World = GEditor->GetEditorWorldContext().World();

	// Find all AUsdStageActor instances in the world
	TArray<AActor*> StageActors;
	UGameplayStatics::GetAllActorsOfClass(World, AUsdStageActor::StaticClass(), StageActors);

	// Check the number of found stage actors
	if (StageActors.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No AUsdStageActor found in the world"));
		return nullptr;
	}
	else if (StageActors.Num() > 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("Multiple UsdStageActor's detected, using first one found"));
	}

	// Cast the first found actor to AUsdStageActor
	TObjectPtr<AUsdStageActor> FoundStageActor = Cast<AUsdStageActor>(StageActors[0]);
	if (!FoundStageActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("USDStageActor pointer is null"));
		return nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("USDStageActor assigned"));
	return FoundStageActor;
}


/**
 * @brief Handles the event when the duplicate button is clicked.
 * Spawns a new CineCameraActor, sets its properties based on the found Usd camera information,
 * and adds it to the specified level sequence if a path is provided.
 * 
 * @param Camera The camera information to duplicate.
 * @param LevelSequencePath The path to the level sequence where the new camera should be added.
 * @return FReply Indicates that the event has been handled.
 */
FReply FUSDCameraFrameRangesModule::OnDuplicateButtonClicked(FCameraInfo Camera, FString LevelSequencePath)
{
    UE_LOG(LogTemp, Log, TEXT("Duplicate button clicked for camera: %s"), *Camera.CameraName);

    // Get the current editor world
    UWorld* World =  GEditor->GetEditorWorldContext().World();

    // Spawn a new CineCameraActor
    TObjectPtr<ACineCameraActor> NewCameraActor = World->SpawnActor<ACineCameraActor>();

    if (!NewCameraActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to spawn new CineCameraActor"));
        return FReply::Handled();
    }

    // Set the label for the new camera
    FString NewLabel = Camera.CameraName + TEXT("_duplicate");
    NewCameraActor->SetActorLabel(NewLabel);

    UE_LOG(LogTemp, Log, TEXT("New camera created with label: %s"), *NewCameraActor->GetActorLabel());
    UE_LOG(LogTemp, Log, TEXT("Camera name: %s"), *NewCameraActor->GetName());

    // Retrieve the translation value from the Usd attribute
    UE::FVtValue Value;
    UE_LOG(LogTemp, Log, TEXT("Getting value"));
    FVector Translation = UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedVec3Attribute(StageActor, Camera.CameraName, "xformOp:translate", 0.0);
    UE_LOG(LogTemp, Log, TEXT("Value received"));

    if (Translation == FVector::ZeroVector)
    {
        UE_LOG(LogTemp, Warning, TEXT("Zero vector returned, check to see if value found correctly"));
    }

    // Set the location of the new camera, factoring in Unreal's Z up
    FVector CameraLocation(Translation[0], Translation[2], Translation[1]);
    NewCameraActor->SetActorLocation(CameraLocation);

    // Retrieve the rotation value from the Usd attribute
    FVector Rotation = UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedVec3Attribute(StageActor, Camera.CameraName, "xformOp:rotateXYZ", 0.0);

    if (Rotation == FVector::ZeroVector)
    {
        UE_LOG(LogTemp, Warning, TEXT("Zero vector returned, check to see if value found correctly"));
    }

    // Set the rotation of the new camera, converting to the unreal rotation standard
    FRotator CameraRotation = UUsdAttributeFunctionLibraryBPLibrary::ConvertToUnrealRotator(Rotation);
    NewCameraActor->SetActorRotation(CameraRotation);

    // Set the focus and filmback settings of the new camera based on the Usd camera
    FCameraFocusSettings FocusSettings;
    FocusSettings.ManualFocusDistance = Camera.FocusDistance;
    FCameraFilmbackSettings FilmbackSettings;
    FilmbackSettings.SensorWidth = Camera.HorizontalAperture;
    FilmbackSettings.SensorHeight = Camera.VerticalAperture;

    UE_LOG(LogTemp, Log, TEXT("Camera settings: \n Focal Length: %f\n Focus Distance: %f\n Aperture: %f\n Sensor Width: %f\n Sensor Height: %f\n"), Camera.FocalLength, Camera.FocusDistance, Camera.FStop, Camera.HorizontalAperture, Camera.VerticalAperture);

    NewCameraActor->GetCineCameraComponent()->SetCurrentFocalLength(Camera.FocalLength);
    NewCameraActor->GetCineCameraComponent()->SetFocusSettings(FocusSettings);
    NewCameraActor->GetCineCameraComponent()->SetCurrentAperture(Camera.FStop);
    NewCameraActor->GetCineCameraComponent()->SetFilmback(FilmbackSettings);

    // Add the new camera to the level sequence if a path is provided
    if (!LevelSequencePath.IsEmpty())
    {
        AddCameraToLevelSequence(LevelSequencePath, NewCameraActor, Camera);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Level sequence path empty, static camera created"));
    }

    return FReply::Handled();
}


/**
 * @brief Handles the Material Swap button click event.
 * 
 * Swaps the materials of the Usd stage actor's generated components with 
 * the corresponding materials found in the Unreal project.
 * 
 * @return A reply indicating whether the event was handled.
 */
FReply FUSDCameraFrameRangesModule::OnMaterialSwapButtonClicked()
{
    UE::FUsdStage Stage = StageActor->GetUsdStage();
    UE::FUsdPrim root = Stage.GetPseudoRoot();
    TArray<FMaterialInfo> MaterialNames;

	// Find the materials present in the project
    TArray<UMaterial*>* FoundMaterials = GetAllMaterials();

	// Traverse the Usd stage to find materials within the Usd and
    TraverseAndCollectMaterials(root, MaterialNames);

	// Iterate over all of the Usd materials and set those components to have the Unreal equivalent
    if (MaterialNames.Num() > 0)
    {
        for (FMaterialInfo Mat : MaterialNames)
        {
            UE::FUsdPrim CurrentPrim = Stage.GetPrimAtPath(Mat.PrimPath);
            USceneComponent* GeneratedComponent = StageActor->GetGeneratedComponent(Mat.PrimPath.GetString());
            
            if (GeneratedComponent)
            {
                UE_LOG(LogTemp, Log, TEXT("ObjName: %s Generated Component name: %s"), *Mat.ObjName, *GeneratedComponent->GetName());

                // Find the Unreal Material with the same name as the Usd material
                FString MaterialName = Mat.MatName;
                for (UMaterial* FoundMaterial : *FoundMaterials)
                {
                    if (MaterialName.Equals(FoundMaterial->GetName()))
                    {
                        UMeshComponent* MeshComponent = Cast<UMeshComponent>(GeneratedComponent);

                        MeshComponent->SetMaterial(0, FoundMaterial);
                        UE_LOG(LogTemp, Log, TEXT("Assigned material: %s to component: %s"), *MaterialName, *MeshComponent->GetName());
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Material: %s not found in project"), *MaterialName);
                    }
                }
            }
        }
    }
    return FReply::Handled();
}

/**
 * @brief Handles the Attribute Export button click event.
 * 
 * Exports the specified float Usd attribute to a level sequence in Unreal Engine.
 * 
 * @param InputPrim The Usd prim name.
 * @param InputAttr The Usd attribute name.
 * @param LevelSequencePath The path to the level sequence in Unreal Engine.
 * @return A reply indicating whether the event was handled.
 */
FReply FUSDCameraFrameRangesModule::OnAttributeExportButtonClicked(const FString& InputPrim, const FString& InputAttr, const FString& LevelSequencePath)
{
    if (InputPrim.IsEmpty() || InputAttr.IsEmpty() || LevelSequencePath.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("One of the inputs is empty, please use valid input"))
        return FReply::Unhandled();
    }

	// Get the level sequence from the specified oath
    ULevelSequence* LevelSequence = Cast<ULevelSequence>(StaticLoadObject(ULevelSequence::StaticClass(), nullptr, *LevelSequencePath));

    if (LevelSequence == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("No level sequence found at path %s"), *LevelSequencePath);
        return FReply::Unhandled();
    }

	// Create the float track and section on the specified level sequence
    UMovieSceneFloatTrack* FloatTrack = LevelSequence->MovieScene->AddTrack<UMovieSceneFloatTrack>();
    UMovieSceneFloatSection* FloatSection = Cast<UMovieSceneFloatSection>(FloatTrack->CreateNewSection());

	// Get the channel on the Level sequence to add the values onto
    FMovieSceneFloatChannel* FloatVal = FloatSection->GetChannelProxy().GetChannel<FMovieSceneFloatChannel>(0);

	// Access the attribute to find its timesamples
    UE::FUsdAttribute TargetAttr = UUsdAttributeFunctionLibraryBPLibrary::GetUsdAttributeInternal(StageActor, InputPrim, InputAttr);

    if (!TargetAttr)
    {
        UE_LOG(LogTemp, Error, TEXT("Valid attribute not found"));
        return FReply::Unhandled();
    }

    TArray<double> TimeSamples;
    TargetAttr.GetTimeSamples(TimeSamples);

	// Only add to the sequence if the attribute contains animation
    if (TimeSamples.Num() > 1)
    {
        int StartFrame = TimeSamples[0];
        int EndFrame = TimeSamples.Last();

    	// Calculate the ticks per frame as unreal doesn't natively provide access to the frames on the sequence
        int TicksPerFrame = LevelSequence->MovieScene->GetTickResolution().AsDecimal() / LevelSequence->MovieScene->GetDisplayRate().AsDecimal();
        FloatSection->SetRange(TRange<FFrameNumber>(FFrameNumber(StartFrame * TicksPerFrame), FFrameNumber(EndFrame * TicksPerFrame)));

    	// Iterate over the time samples and add a key for the value at that frame
        for (double Time : TimeSamples)
        {
            int KeyInt = static_cast<int>(Time);
            FFrameNumber FrameNumber = FFrameNumber(KeyInt * TicksPerFrame);
            float AttrVal = UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedFloatAttribute(StageActor, InputPrim, InputAttr, Time);

            FloatVal->AddConstantKey(FrameNumber, AttrVal);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No timesamples found on specified attribute"))
    }

	// Add the keyed section to the level sequence track
    FloatTrack->AddSection(*FloatSection);

    return FReply::Handled();
}


/**
 * @brief Handles the Disable Manual Focus button click event.
 * 
 * Finds all CineCameraActors in the current world and disables manual focus for each of them.
 * 
 * @return A reply indicating whether the event was handled.
 */
FReply FUSDCameraFrameRangesModule::OnDisableManualFocusButtonClicked()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	TArray<AActor*> CameraActors;
	UGameplayStatics::GetAllActorsOfClass(World, ACineCameraActor::StaticClass(), CameraActors);

	for (AActor* CameraActor : CameraActors)
	{
		DisableManualFocus(Cast<ACineCameraActor>(CameraActor));
	}

	return FReply::Handled();
}

/**
 * @brief Retrieves all UMaterial assets in the project.
 * 
 * Searches in the /Game/Materials directory in the project and collects all UMaterial assets.
 * 
 * Adapted from https://forums.unrealengine.com/t/plugin-get-all-materials-in-current-project/342793/10
 * 
 * @return A pointer to an array of UMaterial assets found in the project.
 */
TArray<UMaterial*>* FUSDCameraFrameRangesModule::GetAllMaterials()
{
	TArray<UMaterial*>* Assets = new TArray<UMaterial*>();

	// Create a library to load all asset data, not filtered by a specific class
	UObjectLibrary *lib = UObjectLibrary::CreateLibrary(UObject::StaticClass(), false, true);
	UE_LOG(LogTemp, Warning, TEXT("Searching for assets in /Game/Materials..."));

	// Load asset data from the specified path
	lib->LoadAssetDataFromPath(TEXT("/Game/Materials"));

	// Retrieve asset data list
	TArray<FAssetData> assetData;
	lib->GetAssetDataList(assetData);
	UE_LOG(LogTemp, Warning, TEXT("Found %d assets"), assetData.Num());

	// Iterate through all asset data and add to the Assets array
	for (FAssetData asset : assetData)
	{
		UMaterial* obj = Cast<UMaterial>(asset.GetAsset());
		if (obj)
		{
			// UE_LOG(LogTemp, Warning, TEXT("Asset: %s, type: %s"), *obj->GetName(), *obj->GetClass()->GetName());
			Assets->Add(obj);
		}
	}

	return Assets;
}


/**
 * @brief Adds a CineCameraActor to a specified Level Sequence and sets up its transform track with animation data.
 * 
 * Loads a Level Sequence from a given path, creates a possessable for the camera actor, and sets up
 * a transform track with keyframes for translation and rotation based on the provided camera information.
 * 
 * @param LevelSequencePath Path to the Level Sequence asset.
 * @param CameraActor Pointer to the CineCameraActor to be added to the Level Sequence.
 * @param Camera Information about the camera including translation and rotation keyframes.
 */
void FUSDCameraFrameRangesModule::AddCameraToLevelSequence(const FString& LevelSequencePath, const TObjectPtr<ACineCameraActor>& CameraActor, FCameraInfo Camera)
{
    // Load the Level Sequence from the given path
    ULevelSequence* LevelSequence = Cast<ULevelSequence>(StaticLoadObject(ULevelSequence::StaticClass(), nullptr, *LevelSequencePath));

    if (LevelSequence == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("No level sequence found at path %s"), *LevelSequencePath);
        return;
    }

    // Create a possessable for the camera actor in the Level Sequence
    FGuid Guid = Cast<UMovieSceneSequence>(LevelSequence)->CreatePossessable(CameraActor);

    if (Guid.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("Camera actor added to %s with Guid %s"), *LevelSequencePath, *Guid.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Guid invalid"));
        return;
    }

    // Add a transform track and section for the camera
    UMovieScene3DTransformTrack* TransformTrack = LevelSequence->MovieScene->AddTrack<UMovieScene3DTransformTrack>(Guid);
    UMovieScene3DTransformSection* TransformSection = Cast<UMovieScene3DTransformSection>(TransformTrack->CreateNewSection());

    int TicksPerFrame = LevelSequence->MovieScene->GetTickResolution().AsDecimal() / LevelSequence->MovieScene->GetDisplayRate().AsDecimal();
    TransformSection->SetRange(TRange<FFrameNumber>(FFrameNumber(Camera.StartFrame * TicksPerFrame), FFrameNumber(Camera.EndFrame * TicksPerFrame)));

    // Get channels for translation and rotation
    FMovieSceneDoubleChannel* TranslateX = TransformSection->GetChannelProxy().GetChannel<FMovieSceneDoubleChannel>(0);
    FMovieSceneDoubleChannel* TranslateY = TransformSection->GetChannelProxy().GetChannel<FMovieSceneDoubleChannel>(1);
    FMovieSceneDoubleChannel* TranslateZ = TransformSection->GetChannelProxy().GetChannel<FMovieSceneDoubleChannel>(2);

    FMovieSceneDoubleChannel* RotateX = TransformSection->GetChannelProxy().GetChannel<FMovieSceneDoubleChannel>(3);
    FMovieSceneDoubleChannel* RotateY = TransformSection->GetChannelProxy().GetChannel<FMovieSceneDoubleChannel>(4);
    FMovieSceneDoubleChannel* RotateZ = TransformSection->GetChannelProxy().GetChannel<FMovieSceneDoubleChannel>(5);

    UE::FVtValue UsdValue;

    // Add translation keyframes
    for (double Time : Camera.TransTimeSamples)
    {		
        int KeyInt = static_cast<int>(Time);
        FFrameNumber FrameNumber = FFrameNumber(KeyInt * TicksPerFrame);
    	FVector Translation = UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedVec3Attribute(StageActor, Camera.CameraName, "xformOp:translate", Time);
    	if (Translation.IsZero())
    	{
    		UE_LOG(LogTemp, Warning, TEXT("Zero vector found when finding translate attribute"))
    	}
    	TranslateX->AddConstantKey(FrameNumber, Translation[0]);
    	TranslateY->AddConstantKey(FrameNumber, Translation[2]);
    	TranslateZ->AddConstantKey(FrameNumber, Translation[1]);
    }

    // Add rotation keyframes
    for (double Time : Camera.RotTimeSamples)
    {
        int KeyInt = static_cast<int>(Time);
        FFrameNumber FrameNumber = FFrameNumber(KeyInt * TicksPerFrame);
    	FVector Rotation = UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedVec3Attribute(StageActor, Camera.CameraName, "xformOp:rotateXYZ", Time);
    	if (Rotation.IsZero())
    	{
    		UE_LOG(LogTemp, Warning, TEXT("Zero vector found when finding rotation attribute"))
    	}
    	RotateX->AddConstantKey(FrameNumber, Rotation[2]);
    	RotateY->AddConstantKey(FrameNumber, Rotation[0]);
    	RotateZ->AddConstantKey(FrameNumber, (Rotation[1] * -1) - 90);
    }

    // Add the transform section to the track
    TransformTrack->AddSection(*TransformSection);
}


/**
 * @brief Disables manual focus for the specified CineCameraActor.
 * 
 * @param CameraActor The camera actor for which to disable manual focus.
 */
void FUSDCameraFrameRangesModule::DisableManualFocus(TObjectPtr<ACineCameraActor> CameraActor)
{
	FCameraFocusSettings FocusSettings;
	FocusSettings.FocusMethod = ECameraFocusMethod::DoNotOverride;

	CameraActor->GetCineCameraComponent()->SetFocusSettings(FocusSettings);    
}

void FUSDCameraFrameRangesModule::FindCameraMainFrameRanges(TArray<FCameraInfo>& Cameras)
{
	UE::FUsdAttribute CameraNumberAttr = UUsdAttributeFunctionLibraryBPLibrary::GetUsdAttributeInternal(StageActor, "cameraMain", "cameraNumber");

	TArray<double> CameraNumberTimeSamples;

	bool bSuccess = CameraNumberAttr.GetTimeSamples(CameraNumberTimeSamples);


	int currentVal = 0;
	int32 prevTime = 0;
	for (double Time : CameraNumberTimeSamples)
	{
		int prevVal = currentVal;
		currentVal = UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedIntAttribute(StageActor, "cameraMain", "cameraNumber", Time);

		if (currentVal == prevVal)
		{
			// UE_LOG(LogTemp, Log, TEXT("Frame range found for camera%d with times %d and %f"), currentVal, prevTime, Time);
			FString CurrentName = TEXT("camera") + FString::FromInt(currentVal);
			for (FCameraInfo& Camera : Cameras)
			{
				if (Camera.CameraName == CurrentName)
				{
					Camera.CameraMainStartFrame = prevTime;
					Camera.CameraMainEndFrame = static_cast<int32>(Time);
					Camera.inCameraMain = true;
					UE_LOG(LogTemp, Log, TEXT("Frame range found for %s with times %d and %d"), *Camera.CameraName, Camera.CameraMainStartFrame, Camera.CameraMainEndFrame);

					break;
				}
			}
		}

		// UE_LOG(LogTemp, Log, TEXT("%f : Camera%d "), Time, currentVal);

		prevTime = static_cast<int32>(Time);
	}
}

/**
 * @brief Retrieves camera information from the Usd stage associated with StageActor.
 *
 * Runs through all of the cameras found within the Usd file to find the
 * name, path, Translation and Rotation attribute and timesamples,
 * start and end frame, focal length, focus distance, FStop,
 * and horizontal and vertical aperture
 * 
 * @return TArray<FCameraInfo> An array of FCameraInfo structures containing details about each camera found in the USD stage.
 */
TArray<FCameraInfo> FUSDCameraFrameRangesModule::GetCamerasFromUSDStage()
{
    TArray<FCameraInfo> Cameras;

    // Check if StageActor is valid
    if (!StageActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("StageActor is null."));
        return Cameras;
    }

    // Get the Usd stage from StageActor
    UE::FUsdStage StageBase = StageActor->GetUsdStage();
    UE::FUsdPrim root = StageBase.GetPseudoRoot();

    // Array to hold the paths of all cameras
    TArray<UE::FSdfPath> CameraPaths;
    TraverseAndCollectCameras(root, CameraPaths);

    // Check if any cameras were found
    if (CameraPaths.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No cameras found in the USD Stage."));
        return Cameras;
    }

    // Iterate through each camera path found
    for (UE::FSdfPath path : CameraPaths)
    {
        UE::FUsdPrim CurrentPrim = StageBase.GetPrimAtPath(path);
        if (!CurrentPrim)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to get Prim at path: %s"), *path.GetString());
            continue;
        }

        // Create a new FCameraInfo structure to hold the camera's details
        FCameraInfo CameraInfo;
        CameraInfo.CameraName = CurrentPrim.GetName().ToString();
        
        // Get translation and rotation attributes for the camera
        CameraInfo.Translation = CurrentPrim.GetAttribute(TEXT("xformOp:translate"));
        CameraInfo.Rotation = CurrentPrim.GetAttribute(TEXT("xformOp:rotateXYZ"));

        if (CameraInfo.Rotation && CameraInfo.Translation)
        {
            // Get time samples for the translation and rotation attributes
            CameraInfo.Rotation.GetTimeSamples(CameraInfo.RotTimeSamples);
            CameraInfo.Translation.GetTimeSamples(CameraInfo.TransTimeSamples);
            
            // Determine the start and end frames based on time samples
            if (CameraInfo.RotTimeSamples.Num() > 1 || CameraInfo.TransTimeSamples.Num() > 1)
            {
                if (CameraInfo.RotTimeSamples.Num() > CameraInfo.TransTimeSamples.Num())
                {
                    if (CameraInfo.RotTimeSamples[1] == 2.0)
                        CameraInfo.StartFrame = CameraInfo.RotTimeSamples[0];
                    else
                        CameraInfo.StartFrame = CameraInfo.RotTimeSamples[1];

                    CameraInfo.EndFrame = CameraInfo.RotTimeSamples.Last();
                }
                else
                {
                    if (CameraInfo.RotTimeSamples[1] == 2.0)
                        CameraInfo.StartFrame = CameraInfo.RotTimeSamples[0];
                    else
                        CameraInfo.StartFrame = CameraInfo.RotTimeSamples[1];

                    CameraInfo.EndFrame = CameraInfo.TransTimeSamples.Last();
                }
            }
            else
            {
                CameraInfo.StartFrame = 1;
                CameraInfo.EndFrame = 1;
            }

            // Retrieve additional camera attributes
            CameraInfo.FocalLength = UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedFloatAttribute(StageActor, CameraInfo.CameraName, FString(TEXT("focalLength")), 1.0);
            CameraInfo.FocusDistance = UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedFloatAttribute(StageActor, CameraInfo.CameraName, FString(TEXT("focusDistance")), 1.0);
            CameraInfo.FStop = UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedFloatAttribute(StageActor, CameraInfo.CameraName, FString(TEXT("fStop")), 1.0);
            CameraInfo.HorizontalAperture = UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedFloatAttribute(StageActor, CameraInfo.CameraName, FString(TEXT("horizontalAperture")), 1.0);
            CameraInfo.VerticalAperture = UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedFloatAttribute(StageActor, CameraInfo.CameraName, FString(TEXT("verticalAperture")), 1.0);

            // Add the camera info to the array
            Cameras.Add(CameraInfo);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to get necessary attributes for camera: %s"), *CameraInfo.CameraName);
        }
    }

    return Cameras;
}


/**
 * @brief Recursively traverses the Usd stage to collect camera prim paths.
 * 
 * @param CurrentPrim The current Usd prim being examined.
 * @param OutCameraPaths Array to store the paths of found camera prims.
 */
void FUSDCameraFrameRangesModule::TraverseAndCollectCameras(const UE::FUsdPrim& CurrentPrim,
	TArray<UE::FSdfPath>& OutCameraPaths)
{

	// Ensure there is a valid prim
	if (!CurrentPrim.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid prim found on the UsdStageActor"))
		return;
	}
	
    // Check if the current prim is of type "Camera"
	if (CurrentPrim.IsA(FName(TEXT("Camera"))))
	{
		// Add prim path to array if it is
		UE::FSdfPath path = CurrentPrim.GetPrimPath();
		OutCameraPaths.Add(path);
	}

    // Recursively traverse child prims
	for (UE::FUsdPrim& Child : CurrentPrim.GetChildren())
	{
		TraverseAndCollectCameras(Child, OutCameraPaths);
	}
}

/**
 * @brief Recursively traverses the Usd stage to collect material information.
 *
 * This is specifically looking for materials under 'Shader' in the Usd.
 * Out of Maya, this is after the shading group which appears as mtl. This may need
 * adjustments for different workflows.
 * 
 * @param CurrentPrim The current Usd prim being examined.
 * @param MaterialNames Array to store material information found in the stage.
 */
void FUSDCameraFrameRangesModule::TraverseAndCollectMaterials(const UE::FUsdPrim& CurrentPrim, TArray<FMaterialInfo>& MaterialNames)
{
    // Get the Usd stage from StageActor
	UE::FUsdStage Stage = StageActor->GetUsdStage();
	if (!CurrentPrim)
	{
		return;
	}

    // Define the relationship name for material bindings
	const TCHAR* RelationshipName = TEXT("material:binding");

    // Check if the current prim has a material binding relationship
	if (UE::FUsdRelationship MaterialBindingRel = CurrentPrim.GetRelationship(RelationshipName))
	{
		TArray<UE::FSdfPath> TargetPaths;
        // Get the target paths for the material binding relationship
		bool bTargets = MaterialBindingRel.GetTargets(TargetPaths);
		if (bTargets)
		{
			for (const UE::FSdfPath& Path : TargetPaths)
			{
				// UE_LOG(LogTemp, Log, TEXT("Current prim: %s Target path: %s"), *CurrentPrim.GetName().ToString(), *Path.GetString());

                // Get the prim at the target path
				if (UE::FUsdPrim MaterialPrim = Stage.GetPrimAtPath(Path))
				{
					FString MaterialName = MaterialPrim.GetName().ToString();
					// UE_LOG(LogTemp, Log, TEXT("Material found: %s"), *MaterialName);

                    // Iterate through child prims of the material prim
					for (UE::FUsdPrim ChildPrim : MaterialPrim.GetChildren())
					{
                        // Check if the child prim is a shader
						if (ChildPrim.IsA("Shader"))
						{
							FString ShaderName = ChildPrim.GetName().ToString();
							// UE_LOG(LogTemp, Log, TEXT("Shader found: %s"), *ShaderName);

                            // Create and populate a FMaterialInfo structure
							FMaterialInfo MaterialInfo;
							MaterialInfo.ObjName = CurrentPrim.GetName().ToString();
							MaterialInfo.MatName = ShaderName;
							MaterialInfo.PrimPath = CurrentPrim.GetPrimPath();

							// UE_LOG(LogTemp, Log, TEXT("Adding material info, ObjName: %s MatName: %s PrimPath: %s"), *MaterialInfo.ObjName,  *MaterialInfo.MatName, *MaterialInfo.PrimPath.GetString());

                            // Add the material info to the array
							MaterialNames.Add(MaterialInfo);
						}
					}
				}
			}
		}
	}

    // Recursively traverse child prims for additional materials
	for (pxr::UsdPrim ChildPrim : CurrentPrim.GetChildren())
	{
		UE::FUsdPrim UEChildPrim(ChildPrim);
		TraverseAndCollectMaterials(UEChildPrim, MaterialNames);
	}
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUSDCameraFrameRangesModule, USDCameraFrameRanges)