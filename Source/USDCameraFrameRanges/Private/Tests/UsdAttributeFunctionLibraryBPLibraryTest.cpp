#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "UsdAttributeFunctionLibraryBPLibrary.h"
#include "UsdStageActor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUsdAttributeFunctionLibraryBPLibraryTest, "UsdAttributeTools.UsdAttributeFunctionLibraryBPLibrary", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUsdAttributeFunctionLibraryBPLibraryTest::RunTest(const FString& Parameters)
{
    // Create a new map and spawn a mock UsdStageActor
    UWorld* MockWorld = FAutomationEditorCommonUtils::CreateNewMap();
    if (!MockWorld)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create a new map."));
        return false;
    }

    AUsdStageActor* MockStageActor = MockWorld->SpawnActor<AUsdStageActor>();
    if (!MockStageActor)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn AUsdStageActor."));
        return false;
    }

    // Path to the USD file
    FString UsdFilePath = TEXT("C:\\Users\\jack3\\OneDrive\\Documents\\Unreal Projects\\UsdAttrTest\\Plugins\\UsdAttributeTools\\TestData\\ue_harbour_test_v09_m2023.usda");
    if (!FPaths::FileExists(UsdFilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("USD file does not exist at path: %s"), *UsdFilePath);
        return false;
    }

    // Set the USD file path
    MockStageActor->SetRootLayer(UsdFilePath);

    // Test GetUsdVec3Attribute function
    FVector Vec3Result = UUsdAttributeFunctionLibraryBPLibrary::GetUsdVec3Attribute(MockStageActor, TEXT("TestPrim"), TEXT("TestVec3Attr"));
    TestNotEqual(TEXT("Vec3Attribute should not be default FVector"), Vec3Result, FVector::ZeroVector);

    // Test GetUsdFloatAttribute function
    float FloatResult = UUsdAttributeFunctionLibraryBPLibrary::GetUsdFloatAttribute(MockStageActor, TEXT("TestPrim"), TEXT("TestFloatAttr"));
    TestTrue(TEXT("FloatAttribute should be greater than zero"), FloatResult > 0.0f);

    // Test GetUsdDoubleAttribute function
    double DoubleResult = UUsdAttributeFunctionLibraryBPLibrary::GetUsdDoubleAttribute(MockStageActor, TEXT("TestPrim"), TEXT("TestDoubleAttr"));
    TestTrue(TEXT("DoubleAttribute should be greater than zero"), DoubleResult > 0.0);

    // Test GetUsdIntAttribute function
    int IntResult = UUsdAttributeFunctionLibraryBPLibrary::GetUsdIntAttribute(MockStageActor, TEXT("TestPrim"), TEXT("TestIntAttr"));
    TestTrue(TEXT("IntAttribute should be greater than zero"), IntResult > 0);

    // Test GetUsdAnimatedVec3Attribute function
    FVector AnimatedVec3Result = UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedVec3Attribute(MockStageActor, TEXT("TestPrim"), TEXT("TestAnimatedVec3Attr"), 1.0);
    TestNotEqual(TEXT("AnimatedVec3Attribute should not be default FVector"), AnimatedVec3Result, FVector::ZeroVector);

    // Test GetUsdAnimatedFloatAttribute function
    float AnimatedFloatResult = UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedFloatAttribute(MockStageActor, TEXT("TestPrim"), TEXT("TestAnimatedFloatAttr"), 1.0);
    TestTrue(TEXT("AnimatedFloatAttribute should be greater than zero"), AnimatedFloatResult > 0.0f);

    // Test GetUsdAnimatedDoubleAttribute function
    double AnimatedDoubleResult = UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedDoubleAttribute(MockStageActor, TEXT("TestPrim"), TEXT("TestAnimatedDoubleAttr"), 1.0);
    TestTrue(TEXT("AnimatedDoubleAttribute should be greater than zero"), AnimatedDoubleResult > 0.0);

    // Test GetUsdAnimatedIntAttribute function
    int AnimatedIntResult = UUsdAttributeFunctionLibraryBPLibrary::GetUsdAnimatedIntAttribute(MockStageActor, TEXT("TestPrim"), TEXT("TestAnimatedIntAttr"), 1.0);
    TestTrue(TEXT("AnimatedIntAttribute should be greater than zero"), AnimatedIntResult > 0);

    // Test ConvertToUnrealRotator function
    FVector TestVector(1.0f, 2.0f, 3.0f);
    FRotator RotatorResult = UUsdAttributeFunctionLibraryBPLibrary::ConvertToUnrealRotator(TestVector);
    TestNotEqual(TEXT("Rotator should not be default FRotator"), RotatorResult, FRotator::ZeroRotator);

    // Clean up
    MockStageActor->Destroy();

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
