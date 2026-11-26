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

USTRUCT(BlueprintType)
struct FAPPlayer
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago")
    int32 Team = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago")
    int32 Slot = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago")
    FString Alias;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago")
    FString Name;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago")
    FString PlayerClass;
};

USTRUCT(BlueprintType)
struct FAPSlotInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago")
    FString Name;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago")
    FString Game;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago")
    int32 Type = 0;
};

USTRUCT(BlueprintType)
struct FAPNetworkItem
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago")
    int64 Item = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago")
    int64 Location = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago")
    int32 Player = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago")
    int32 Flags = 0;
};

USTRUCT(BlueprintType)
struct FAPResolvedNetworkItem
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Items")
    FString ItemName;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Items")
    FString LocationName;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Items")
    FString PlayerName;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Items")
    int64 ItemID = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Items")
    int64 LocationID = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Items")
    int32 PlayerSlot = 0;

    // Can contain multiple classifications.
    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Items")
    TSet<EAPItemClassification> Classifications;

    // Indicates whether the IDs could be resolved into names.
    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Items")
    bool bItemNameFound = false;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Items")
    bool bLocationNameFound = false;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Items")
    bool bPlayerNameFound = false;
};

USTRUCT(BlueprintType)
struct FAPGameData
{
    GENERATED_BODY()

    // Item ID -> Item Name
    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Game Data")
    TMap<int64, FString> ItemNames;

    // Location ID -> Location Name
    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Game Data")
    TMap<int64, FString> LocationNames;
};

USTRUCT(BlueprintType)
struct FAPPrintJSONSegment
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Messages")
    FString Text;

    // Original type supplied by Archipelago.
    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Messages")
    FString Type;

    // Color selected by our client for display.
    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Messages")
    FLinearColor Color = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FAPPrintJSONMessage
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Messages")
    TArray<FAPPrintJSONSegment> Segments;

    // Convenience field for logging or simple text displays.
    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Messages")
    FString PlainText;
};

USTRUCT(BlueprintType)
struct FAPMessageTypeColorPalette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Messages|Colors")
    FLinearColor TextColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Messages|Colors")
    FLinearColor PlayerColor = FLinearColor(0.35f, 0.75f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Messages|Colors")
    FLinearColor ItemColor = FLinearColor(1.0f, 0.75f, 0.2f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Messages|Colors")
    FLinearColor LocationColor = FLinearColor(0.35f, 0.9f, 0.45f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Messages|Colors")
    FLinearColor EntranceColor = FLinearColor(0.75f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Messages|Colors")
    FLinearColor HintColor = FLinearColor(1.0f, 0.6f, 0.2f);
};

USTRUCT(BlueprintType)
struct FAPCheckedLocation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room")
    int64 ID = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room")
    FString Name;
};

USTRUCT(BlueprintType)
struct FAPRoomUpdate
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room")
    int32 HintPoints = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room")
    TArray<FAPCheckedLocation> CheckedLocations;
};

#pragma endregion