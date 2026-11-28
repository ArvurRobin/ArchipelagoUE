// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ArchipelagoSubsystem.h"
#include "ArchipelagoFunctionLibrary.generated.h"

UCLASS()
class ARCHIPELAGOUE_API UArchipelagoFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    // Subsystem Access
    static UArchipelagoSubsystem* GetArchipelagoSubsystem(const UObject* WorldContextObject);

#pragma region Setup

    UFUNCTION(BlueprintCallable, Category = "Archipelago",
        meta = (WorldContext = "WorldContextObject"))
    static bool SetupArchipelago(
        const UObject* WorldContextObject,
        const FString& GameName,
        const FAPMessageTypeColorPalette& ColorPalette
    );

#pragma endregion

#pragma region Config

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Config",
        meta = (WorldContext = "WorldContextObject"))
    static bool OverrideConfig(
        const UObject* WorldContextObject,
        const FAPConfig& NewConfig
    );

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Config",
        meta = (WorldContext = "WorldContextObject"))
    static EAPConfigCreateResult CreateNewConfig(
        const UObject* WorldContextObject,
        bool bOverrideExisting = false
    );

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Config",
        meta = (WorldContext = "WorldContextObject"))
    static EAPConfigLoadResult LoadConfig(
        const UObject* WorldContextObject
    );

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Config",
        meta = (WorldContext = "WorldContextObject"))
    static EAPConfigSaveResult SaveConfig(
        const UObject* WorldContextObject
    );

#pragma endregion

#pragma region WebSocket

    UFUNCTION(BlueprintCallable, Category = "Archipelago|WebSocket",
        meta = (WorldContext = "WorldContextObject", DisplayName = "Connect Archipelago WebSocket"))
    static EAPWebSocketConnectionResult ConnectWebSocket(
        const UObject* WorldContextObject
    );

    UFUNCTION(BlueprintCallable, Category = "Archipelago|WebSocket",
        meta = (WorldContext = "WorldContextObject", DisplayName = "Destroy Archipelago WebSocket"))
    static EAPWebSocketDestroyResult DestroyWebSocket(
        const UObject* WorldContextObject
    );

#pragma endregion

#pragma region Locations

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Locations",
        meta = (WorldContext = "WorldContextObject"))
    static bool SendLocationCheck(
        const UObject* WorldContextObject,
        int64 LocationId
    );

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Locations",
        meta = (WorldContext = "WorldContextObject"))
    static bool SendLocationChecks(
        const UObject* WorldContextObject,
        const TArray<int64>& LocationIds
    );

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Locations",
        meta = (WorldContext = "WorldContextObject"))
    static bool RequestLocationScout(
        const UObject* WorldContextObject,
        int64 LocationId
    );

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Locations",
        meta = (WorldContext = "WorldContextObject"))
    static bool RequestLocationScouts(
        const UObject* WorldContextObject,
        const TArray<int64>& LocationIds
    );

#pragma endregion

#pragma region Items

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Items",
        meta = (WorldContext = "WorldContextObject"))
    static bool RequestSync(
        const UObject* WorldContextObject
    );

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Items",
        meta = (WorldContext = "WorldContextObject"))
    static void ApplyItem(
        const UObject* WorldContextObject,
        int64 ItemID
    );

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Items",
        meta = (WorldContext = "WorldContextObject"))
    static void MarkItemAsApplied(
        const UObject* WorldContextObject,
        int32 ItemIndex
    );

#pragma endregion

#pragma region Player

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Player",
        meta = (WorldContext = "WorldContextObject"))
    static bool SendStatusUpdate(
        const UObject* WorldContextObject,
        EAPClientStatus Status
    );

#pragma endregion

#pragma region Chat

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Chat",
        meta = (WorldContext = "WorldContextObject"))
    static bool SendChatMessage(
        const UObject* WorldContextObject,
        const FString& Message
    );

#pragma endregion

#pragma region Save

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Save",
        meta = (WorldContext = "WorldContextObject"))
    static bool SaveArchipelagoSave(
        const UObject* WorldContextObject,
        const FString& OriginalSavePath
    );

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Save",
        meta = (WorldContext = "WorldContextObject"))
    static bool LoadArchipelagoSave(
        const UObject* WorldContextObject,
        const FString& OriginalSavePath
    );

#pragma endregion
};