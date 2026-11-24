// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

#pragma once

#include "ArchipelagoEnums.h"
#include "CoreMinimal.h"
#include "ArchipelagoStructs.generated.h"

#pragma region Config

USTRUCT(BlueprintType, Category = "Archipelago|Config", meta = (DisplayName = "Archipelago Config"))
struct FAPConfig
{
    GENERATED_BODY()

    // Default constructor
    FAPConfig():
        ServerUrl(TEXT("archipelago.gg")),
        ServerPort(12345),
        Slot(TEXT("")),
        Password(TEXT("")),
        AutoConnectOnStartup(false)
    {
    }

    // Parameterized constructor
    FAPConfig(
        const FString& InServerUrl,
        int32 InServerPort,
        const FString& InSlot,
        const FString& InPassword,
        bool bInAutoConnectOnStartup
    ) :
        ServerUrl(InServerUrl),
        ServerPort(InServerPort),
        Slot(InSlot),
        Password(InPassword),
        AutoConnectOnStartup(bInAutoConnectOnStartup)
    {
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Config",
        meta = (
            DisplayName = "Server URL",
            Tooltip = "Archipelago server address to connect to. Use the address provided on your Multiworld room page (e.g. archipelago.gg)."
            ))
    FString ServerUrl;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Config", meta = (
            DisplayName = "Server Port",
            Tooltip = "Archipelago server port to connect to. Valid range: 1–65535. Use the port provided on your Multiworld room page (e.g. 12345 in 'archipelago.gg:12345').",
            ClampMin = "1",
            ClampMax = "65535",
            UIMin = "1",
            UIMax = "65535"))
    int32 ServerPort = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Config",
        meta = (DisplayName = "Slot", Tooltip = "The Slot to connect as"))
    FString Slot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Config",
        meta = (DisplayName = "Password", Tooltip = "The Password for the Slot to connect as (encoded)"))
    FString Password;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Config",
        meta = (DisplayName = "Auto Connect on Startup", Tooltip = "Try to connect to Archipelago on Game Startup"))
    bool AutoConnectOnStartup;
};

#pragma endregion

#pragma region Archipelago

/**
 * Archipelago protocol version.
 */
USTRUCT(BlueprintType)
struct FAPVersion
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room Info")
    int32 Major = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room Info")
    int32 Minor = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room Info")
    int32 Build = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room Info")
    FString Class = TEXT("Version");
};

/**
 * Room permissions.
 */
USTRUCT(BlueprintType)
struct FAPRoomPermissions
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room Info")
    EAPPermission Release = EAPPermission::Disabled;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room Info")
    EAPPermission Remaining = EAPPermission::Disabled;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room Info")
    EAPPermission Collect = EAPPermission::Disabled;
};

/**
 * Archipelago RoomInfo packet.
 */
USTRUCT(BlueprintType)
struct FAPRoomInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room")
    bool bPasswordRequired = false;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room")
    TArray<FString> Games;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room")
    TArray<FString> Tags;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room")
    FAPVersion Version;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room")
    FAPVersion GeneratorVersion;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room")
    FAPRoomPermissions Permissions;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room")
    int32 HintCost = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room")
    int32 LocationCheckPoints = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room")
    TMap<FString, FString> DataPackageChecksums;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room")
    FString SeedName;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room")
    double Time = 0.0;
};

#pragma endregion