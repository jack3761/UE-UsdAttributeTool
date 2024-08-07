#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "USDCameraFrameRanges.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "USDStageActor.h"
#include "CineCameraActor.h"
#include "CineCameraComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

// Test for FindUsdStageActor
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUSDCameraFrameRangesModuleTest, "UsdAttributeTools.USDCameraFrameRangesModule.FindUsdStageActor", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUSDCameraFrameRangesModuleTest::RunTest(const FString& Parameters)
{
    UWorld* MockWorld = FAutomationEditorCommonUtils::CreateNewMap();
    
    FUSDCameraFrameRangesModule Module;

    TObjectPtr<AUsdStageActor> FoundStageActor = Module.FindUsdStageActor();
    TestNull(TEXT("Should return nullptr"), FoundStageActor.Get());

    AUsdStageActor* MockStageActor = MockWorld->SpawnActor<AUsdStageActor>();

    FoundStageActor = Module.FindUsdStageActor();
    TestNotNull(TEXT("Should find an AUsdStageActor"), FoundStageActor.Get());

    TestEqual(TEXT("FoundStageActor should be the same as MockStageActor"), FoundStageActor.Get(), MockStageActor);

    MockStageActor->Destroy();

    return true;
}

// Test for GetCamerasFromUSDStage

// Test for OnDuplicateButtonClicked

//Test for OnMaterialSwapButtonClicked

//Test for OnAttributeExportButtonClicked

//Test for TraverseAndCollectCameras

//Test for TraverseAndCollectMaterials

//Test for GetAllMaterials

//Test for AddCameraToLevelSequence

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUSDCameraFrameRangesModuleDisableManualFocusTest, "UsdAttributeTools.USDCameraFrameRangesModule.DisableManualFocus", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUSDCameraFrameRangesModuleDisableManualFocusTest::RunTest(const FString& Parameters)
{
    UWorld* MockWorld = FAutomationEditorCommonUtils::CreateNewMap();

    ACineCameraActor* MockCameraActor = MockWorld->SpawnActor<ACineCameraActor>();
    TestNotNull(TEXT("Should spawn a CineCameraActor"), MockCameraActor);

    FUSDCameraFrameRangesModule Module;
    Module.DisableManualFocus(MockCameraActor);

    FCameraFocusSettings FocusSettings = MockCameraActor->GetCineCameraComponent()->FocusSettings;
    TestEqual(TEXT("FocusMethod should be DoNotOverride"), FocusSettings.FocusMethod, ECameraFocusMethod::DoNotOverride);

    MockCameraActor->Destroy();

    return true;
}




#endif // WITH_DEV_AUTOMATION_TESTS
