// Copyright Epic Games, Inc. All Rights Reserved.

#include "USDCameraFrameRanges.h"

#include "USDCameraFrameRangesStyle.h"
#include "USDCameraFrameRangesCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "USDStageActor.h"
#include "CineCameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "Editor.h"
#include "EditorLevelUtils.h"

#include "LevelSequence.h"
#include "MovieScene.h"
// #include "MovieSceneFloatChannel.h"
// #include "MovieScene3DTransformTrack.h"
// #include "MovieScene3DTransformSection.h"
#include "FileHelpers.h"
#include "LevelSequenceActor.h"
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
#include "pxr/base/gf/vec3d.h"
#include "pxr/usd/usdShade/material.h"
#include "pxr/usd/usdShade/shader.h"
#include "USDIncludesEnd.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/ObjectLibrary.h"
#include "Experimental/Async/AwaitableTask.h"
#include "Tracks/MovieScene3DTransformTrack.h"
#include "Sections/MovieScene3DTransformSection.h"
#include "UObject/SavePackage.h"

#include "UsdAttributeFunctionLibrary/Public/UsdAttributeFunctionLibrary.h"

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



TSharedRef<SDockTab> FUSDCameraFrameRangesModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	StageActor = FindUsdStageActor();
    if (!StageActor)
    {
        // Handle case when StageActor is not found
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

    TArray<FCameraInfo> Cameras = GetCamerasFromUSDStage();

    if (Cameras.Num() == 0)
    {
        // Handle case when no cameras are found
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

    TSharedPtr<SEditableTextBox> SequenceInputTextBox = SNew(SEditableTextBox);
	TSharedPtr<SEditableTextBox> PrimInputTextBox = SNew(SEditableTextBox);
	TSharedPtr<SEditableTextBox> AttrInputTextBox = SNew(SEditableTextBox);

    TSharedPtr<SVerticalBox> CameraList = SNew(SVerticalBox);

    CameraList->AddSlot()
    .Padding(2)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .FillWidth(0.4)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Camera name")))
        ]
        + SHorizontalBox::Slot()
        .FillWidth(0.4)
        [
            SNew(STextBlock)
            .Text(FText::FromString("Frame range"))
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Create CineCameraActor")))
        ]
    ];

	CameraList->AddSlot()
	.Padding(10)
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Level sequence path:")))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SequenceInputTextBox.ToSharedRef()
		]
	];

	CameraList->AddSlot()
	.Padding(10)
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(20)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				PrimInputTextBox.ToSharedRef()
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				AttrInputTextBox.ToSharedRef()
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Export to sequence")))
				.OnClicked_Lambda([this, PrimInputTextBox, AttrInputTextBox, SequenceInputTextBox]()
				{
					return OnAttributeExportButtonClicked(PrimInputTextBox->GetText().ToString(), AttrInputTextBox->GetText().ToString(), SequenceInputTextBox->GetText().ToString());
				})
			]
		]
	];


    // Loop through Cameras array and create a row widget for each camera
    for (const FCameraInfo& Camera : Cameras)
    {
        CameraList->AddSlot()
        .Padding(2)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .FillWidth(0.4)
            [
                SNew(STextBlock)
                .Text(FText::FromString(Camera.CameraName))
            ]
            + SHorizontalBox::Slot()
            .FillWidth(0.4)
            [
                SNew(STextBlock)
                .Text(FText::FromString(FString::Printf(TEXT("%d - %d"), Camera.StartFrame, Camera.EndFrame)))
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
        ];
    }

	return SNew(SDockTab)
	.TabRole(ETabRole::NomadTab)
	[
		// Main content of the tab
		SNew(SVerticalBox) 
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
		.Padding(20)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("Material swap")))
			.OnClicked(FOnClicked::CreateRaw(this, &FUSDCameraFrameRangesModule::OnMaterialSwapButtonClicked))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(20)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("Disable Manual Focus")))
			.OnClicked(FOnClicked::CreateRaw(this, &FUSDCameraFrameRangesModule::OnDisableManualFocusButtonClicked))
		]
	];

}

TObjectPtr<AUsdStageActor> FUSDCameraFrameRangesModule::FindUsdStageActor()
{
	UE_LOG(LogTemp, Log, TEXT("Searching for UsdStageActor"));
	
	if (!GEditor)
	{
		UE_LOG(LogTemp, Warning, TEXT("GEditor is not available"));
		return nullptr;
	}
    
	UWorld* World = GEditor->GetEditorWorldContext().World();

	TArray<AActor*> StageActors;
	UGameplayStatics::GetAllActorsOfClass(World, AUsdStageActor::StaticClass(), StageActors);

	if (StageActors.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No AUsdStageActor found in the world"));
		return nullptr;
	}
	else if (StageActors.Num() > 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("Multiple UsdStageActor's detected, using first one found"));
		return nullptr;
	}
		

	// Will only work with one UsdStageActor, which is the aim for this project
	TObjectPtr<AUsdStageActor> FoundStageActor = Cast<AUsdStageActor>(StageActors[0]);
	if (!FoundStageActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("USDStageActor pointer is null"));
		return nullptr;
	}
    
	UE_LOG(LogTemp, Log, TEXT("USDStageActor assigned"));
	return FoundStageActor;
}

FReply FUSDCameraFrameRangesModule::OnDuplicateButtonClicked(FCameraInfo Camera, FString LevelSequencePath)
{
	UE_LOG(LogTemp, Log, TEXT("Duplicate button clicked for camera: %s"), *Camera.CameraName);

	UWorld* World =  GEditor->GetEditorWorldContext().World();
	
	TObjectPtr<ACineCameraActor> NewCameraActor = World->SpawnActor<ACineCameraActor>();
	
	if (!NewCameraActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn new CineCameraActor"));
		return FReply::Handled();
	}
	
	FString NewLabel = Camera.CameraName + TEXT("_duplicate");
	NewCameraActor->SetActorLabel(NewLabel);

	UE_LOG(LogTemp, Log, TEXT("New camera created with label: %s"), *NewCameraActor->GetActorLabel());
	UE_LOG(LogTemp, Log, TEXT("Camera name: %s"), *NewCameraActor->GetName());
	
	UE::FVtValue Value;
	
	UE_LOG(LogTemp, Log, TEXT("Getting value"));

	// bool bSuccess = Camera.Translation.Get(Value, 0.0);
	FVector Translation = UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedVec3Attribute(StageActor, Camera.CameraName, "xformOp:translate", 0.0);

	UE_LOG(LogTemp, Log, TEXT("Value received"));

	if (Translation == FVector::ZeroVector)
	{
		UE_LOG(LogTemp, Warning, TEXT("Zero vector returned, check to see if value found correctly"))
	}

	FVector CameraLocation(Translation[0], Translation[2], Translation[1]);
	
	NewCameraActor->SetActorLocation(CameraLocation);

	FVector Rotation = 	UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedVec3Attribute(StageActor, Camera.CameraName, "xformOp:rotateXYZ", 0.0);

	if (Rotation == FVector::ZeroVector)
	{
		UE_LOG(LogTemp, Warning, TEXT("Zero vector returned, check to see if value found correctly"))
	}

	FRotator CameraRotation = UUsdAttributeFunctionLibraryBPLibrary::ConvertToUnrealRotator(Rotation);

	NewCameraActor->SetActorRotation(CameraRotation);

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

	// CreateCameraLevelSequence(NewCameraActor, StageActor, Camera);
	if (!LevelSequencePath.IsEmpty())
	{
		AddCameraToLevelSequence(LevelSequencePath, NewCameraActor, Camera);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Level sequence path empty, static camera created"))
	}

	return FReply::Handled();
}

FReply FUSDCameraFrameRangesModule::OnMaterialSwapButtonClicked()
{
	UE::FUsdStage Stage = StageActor->GetUsdStage();
	UE::FUsdPrim root = Stage.GetPseudoRoot();
	TArray<FMaterialInfo> MaterialNames;

	TArray<UMaterial*>* FoundMaterials = GetAllMaterials();
	
	TraverseAndCollectMaterials(root, MaterialNames);

	if (MaterialNames.Num() >0 )
	{
		for (FMaterialInfo Mat : MaterialNames)
		{
			UE::FUsdPrim CurrentPrim = Stage.GetPrimAtPath(Mat.PrimPath);
			USceneComponent* GeneratedComponent = StageActor->GetGeneratedComponent(Mat.PrimPath.GetString());
			
			if (GeneratedComponent)
			{
				UE_LOG(LogTemp, Log, TEXT("ObjName: %s Generated Component name: %s"), *Mat.ObjName, *GeneratedComponent->GetName());

				// Find the Unreal Material with the same name as the USD material
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

FReply FUSDCameraFrameRangesModule::OnAttributeExportButtonClicked(const FString& InputPrim, const FString& InputAttr, const FString& LevelSequencePath)
{
	
	ULevelSequence* LevelSequence = Cast<ULevelSequence>(StaticLoadObject(ULevelSequence::StaticClass(), nullptr, *LevelSequencePath));

	if (LevelSequence == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("No level sequence found at path %s"), *LevelSequencePath);
		FReply::Unhandled();
	}
	
	UMovieSceneFloatTrack* FloatTrack = LevelSequence->MovieScene->AddTrack<UMovieSceneFloatTrack>();
	UMovieSceneFloatSection* FloatSection = Cast<UMovieSceneFloatSection>(FloatTrack->CreateNewSection());
	
	FMovieSceneFloatChannel* FloatVal = FloatSection->GetChannelProxy().GetChannel<FMovieSceneFloatChannel>(0);

	UE::FUsdAttribute TargetAttr = UUsdAttributeFunctionLibraryBPLibrary::GetUsdAttributeInternal(StageActor, InputPrim, InputAttr);

	TArray<double> TimeSamples;
	TargetAttr.GetTimeSamples(TimeSamples);


	if (TimeSamples.Num() > 1)
	{
		int StartFrame = TimeSamples[0];
		int EndFrame = TimeSamples.Last();
	
		int TicksPerFrame = LevelSequence->MovieScene->GetTickResolution().AsDecimal() / LevelSequence->MovieScene->GetDisplayRate().AsDecimal();
		FloatSection->SetRange(TRange<FFrameNumber>(FFrameNumber(StartFrame * TicksPerFrame), FFrameNumber(EndFrame * TicksPerFrame)));
	
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

	FloatTrack->AddSection(*FloatSection);

	return FReply::Handled();
}

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

// adapted from https://forums.unrealengine.com/t/plugin-get-all-materials-in-current-project/342793/10
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
	for (FAssetData asset : assetData) {
		UMaterial* obj = Cast<UMaterial>(asset.GetAsset());
		if (obj) {
			UE_LOG(LogTemp, Warning, TEXT("Asset: %s, type: %s"), *obj->GetName(), *obj->GetClass()->GetName());
			Assets->Add(obj);
		}
	}

	return Assets;
}

void FUSDCameraFrameRangesModule::AddCameraToLevelSequence(FString LevelSequencePath,
	TObjectPtr<ACineCameraActor> CameraActor, FCameraInfo Camera)
{
	ULevelSequence* LevelSequence = Cast<ULevelSequence>(StaticLoadObject(ULevelSequence::StaticClass(), nullptr, *LevelSequencePath));

	if (LevelSequence == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("No level sequence found at path %s"), *LevelSequencePath);
		return;
	}

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

	UMovieScene3DTransformTrack* TransformTrack = LevelSequence->MovieScene->AddTrack<UMovieScene3DTransformTrack>(Guid);
	UMovieScene3DTransformSection* TransformSection = Cast<UMovieScene3DTransformSection>(TransformTrack->CreateNewSection());

	int TicksPerFrame = LevelSequence->MovieScene->GetTickResolution().AsDecimal() / LevelSequence->MovieScene->GetDisplayRate().AsDecimal();
	TransformSection->SetRange(TRange<FFrameNumber>(FFrameNumber(Camera.StartFrame * TicksPerFrame), FFrameNumber(Camera.EndFrame * TicksPerFrame)));
	
	FMovieSceneDoubleChannel* TranslateX = TransformSection->GetChannelProxy().GetChannel<FMovieSceneDoubleChannel>(0);
	FMovieSceneDoubleChannel* TranslateY = TransformSection->GetChannelProxy().GetChannel<FMovieSceneDoubleChannel>(1);
	FMovieSceneDoubleChannel* TranslateZ = TransformSection->GetChannelProxy().GetChannel<FMovieSceneDoubleChannel>(2);

	FMovieSceneDoubleChannel* RotateX = TransformSection->GetChannelProxy().GetChannel<FMovieSceneDoubleChannel>(3);
	FMovieSceneDoubleChannel* RotateY = TransformSection->GetChannelProxy().GetChannel<FMovieSceneDoubleChannel>(4);
	FMovieSceneDoubleChannel* RotateZ = TransformSection->GetChannelProxy().GetChannel<FMovieSceneDoubleChannel>(5);

	UE::FVtValue UsdValue;
	for (double Time : Camera.TransTimeSamples)
	{		
		int KeyInt = static_cast<int>(Time);
		FFrameNumber FrameNumber = FFrameNumber(KeyInt * TicksPerFrame);
		if (Camera.Translation.Get(UsdValue, Time))
		{
			pxr::VtValue PxrValue = UsdValue.GetUsdValue();
			if (PxrValue.IsHolding<pxr::GfVec3d>())
			{
				pxr::GfVec3d Translation = PxrValue.Get<pxr::GfVec3d>();
				TranslateX->AddConstantKey(FrameNumber, Translation[0]);
				TranslateY->AddConstantKey(FrameNumber, Translation[2]);
				TranslateZ->AddConstantKey(FrameNumber, Translation[1]);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Translation value is not holding GfVec3d at time %d"), KeyInt);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get translation value at time %d"), KeyInt);
		}
	}

	for (double Time : Camera.RotTimeSamples)
	{
		int KeyInt = static_cast<int>(Time);
		FFrameNumber FrameNumber = FFrameNumber(KeyInt * TicksPerFrame);
		if (Camera.Rotation.Get(UsdValue, Time))
		{
			pxr::VtValue PxrValue = UsdValue.GetUsdValue();
			if (PxrValue.IsHolding<pxr::GfVec3f>())
			{
				pxr::GfVec3f Rotation = PxrValue.Get<pxr::GfVec3f>();
				RotateX->AddConstantKey(FrameNumber, Rotation[2]);
				RotateY->AddConstantKey(FrameNumber, Rotation[0]);
				RotateZ->AddConstantKey(FrameNumber, (Rotation[1] * -1) - 90);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Rotation value is not holding GfVec3f at time %d"), KeyInt);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get rotation value at time %d"), KeyInt);
		}
	}

	TransformTrack->AddSection(*TransformSection);

}

void FUSDCameraFrameRangesModule::DisableManualFocus(TObjectPtr<ACineCameraActor> CameraActor)
{
	FCameraFocusSettings FocusSettings;
	FocusSettings.FocusMethod = ECameraFocusMethod::DoNotOverride;

	CameraActor->GetCineCameraComponent()->SetFocusSettings(FocusSettings);	
}


void FUSDCameraFrameRangesModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(USDCameraFrameRangesTabName);	
}


TArray<FCameraInfo> FUSDCameraFrameRangesModule::GetCamerasFromUSDStage()
{
    TArray<FCameraInfo> Cameras;

    if (!StageActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("StageActor is null."));
        return Cameras;
    }

    UE::FUsdStage StageBase = StageActor->GetUsdStage();
    UE::FUsdPrim root = StageBase.GetPseudoRoot();

    TArray<UE::FSdfPath> CameraPaths;
    TraverseAndCollectCameras(root, CameraPaths);

    if (CameraPaths.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No cameras found in the USD Stage."));
        return Cameras;
    }

    for (UE::FSdfPath path : CameraPaths)
    {
        UE::FUsdPrim CurrentPrim = StageBase.GetPrimAtPath(path);
        if (!CurrentPrim)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to get Prim at path: %s"), *path.GetString());
            continue;
        }

        FCameraInfo CameraInfo;
        CameraInfo.CameraName = CurrentPrim.GetName().ToString();
        
        CameraInfo.Translation = CurrentPrim.GetAttribute(TEXT("xformOp:translate"));
        CameraInfo.Rotation = CurrentPrim.GetAttribute(TEXT("xformOp:rotateXYZ"));

        if (CameraInfo.Rotation && CameraInfo.Translation)
        {
            CameraInfo.Rotation.GetTimeSamples(CameraInfo.RotTimeSamples);
            CameraInfo.Translation.GetTimeSamples(CameraInfo.TransTimeSamples);
            
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

        	CameraInfo.FocalLength = UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedFloatAttribute(StageActor, CameraInfo.CameraName, FString(TEXT("focalLength")), 1.0);
        	CameraInfo.FocusDistance = UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedFloatAttribute(StageActor, CameraInfo.CameraName, FString(TEXT("focusDistance")), 1.0);
        	CameraInfo.FStop = UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedFloatAttribute(StageActor, CameraInfo.CameraName, FString(TEXT("fStop")), 1.0);
        	CameraInfo.HorizontalAperture = UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedFloatAttribute(StageActor, CameraInfo.CameraName, FString(TEXT("horizontalAperture")), 1.0);
        	CameraInfo.VerticalAperture = UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedFloatAttribute(StageActor, CameraInfo.CameraName, FString(TEXT("verticalAperture")), 1.0);

        	Cameras.Add(CameraInfo);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to get necessary attributes for camera: %s"), *CameraInfo.CameraName);
        }
    }

    return Cameras;
}


void FUSDCameraFrameRangesModule::TraverseAndCollectCameras(const UE::FUsdPrim& CurrentPrim,
	TArray<UE::FSdfPath>& OutCameraPaths)
{
	if (CurrentPrim.IsA(FName(TEXT("Camera"))))
	{
		UE::FSdfPath path = CurrentPrim.GetPrimPath();
		OutCameraPaths.Add(path);
        UE_LOG(LogTemp, Log, TEXT("Camera found at path: %s"), *path.GetString());
	}

	for (UE::FUsdPrim& Child : CurrentPrim.GetChildren())
	{
		UE_LOG(LogTemp, Log, TEXT("Traversing from %s, type: %s"), *Child.GetName().ToString(), *Child.GetTypeName().ToString());
		TraverseAndCollectCameras(Child, OutCameraPaths);
	}
}


void FUSDCameraFrameRangesModule::TraverseAndCollectMaterials(const UE::FUsdPrim& CurrentPrim, TArray<FMaterialInfo>& MaterialNames)
{
	UE::FUsdStage Stage = StageActor->GetUsdStage();
	if (!CurrentPrim)
	{
		return;
	}

	
	const TCHAR* RelationshipName = TEXT("material:binding");
	if (UE::FUsdRelationship MaterialBindingRel = CurrentPrim.GetRelationship(RelationshipName))
	{
		TArray<UE::FSdfPath> TargetPaths;
		bool bTargets = MaterialBindingRel.GetTargets(TargetPaths);
		if (bTargets)
		{
			for (const UE::FSdfPath& Path : TargetPaths)
			{
				UE_LOG(LogTemp, Log, TEXT("Current prim: %s Target path: %s"), *CurrentPrim.GetName().ToString(), *Path.GetString());

				if (UE::FUsdPrim MaterialPrim = Stage.GetPrimAtPath(Path))
				{
					FString MaterialName = MaterialPrim.GetName().ToString();
					UE_LOG(LogTemp, Log, TEXT("Material found: %s"), *MaterialName);

					for (UE::FUsdPrim ChildPrim : MaterialPrim.GetChildren())
					{
						if (ChildPrim.IsA("Shader"))
						{
							FString ShaderName = ChildPrim.GetName().ToString();
							UE_LOG(LogTemp, Log, TEXT("Shader found: %s"), *ShaderName);

							FMaterialInfo MaterialInfo;
							MaterialInfo.ObjName = CurrentPrim.GetName().ToString();
							MaterialInfo.MatName = ShaderName;
							MaterialInfo.PrimPath = CurrentPrim.GetPrimPath();

							UE_LOG(LogTemp, Log, TEXT("Adding material info, ObjName: %s MatName: %s PrimPath: %s"), *MaterialInfo.ObjName,  *MaterialInfo.MatName, *MaterialInfo.PrimPath.GetString());

							
							MaterialNames.Add(MaterialInfo);
						}
					}
				}
			}
		}
	}

	for (pxr::UsdPrim ChildPrim : CurrentPrim.GetChildren())
	{
		UE::FUsdPrim UEChildPrim(ChildPrim);
		TraverseAndCollectMaterials(UEChildPrim, MaterialNames);
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUSDCameraFrameRangesModule, USDCameraFrameRanges)