// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ArchipelagoEnums.h"
#include "ArchipelagoStructs.generated.h"

#pragma region Config

USTRUCT(BlueprintType, meta = (ToolTip = "Configuration settings used to connect to an Archipelago multiworld server."))
struct FAPConfig
{
    GENERATED_BODY()

    FAPConfig() :
        ServerUrl(TEXT("archipelago.gg")),
        ServerPort(12345),
        Slot(TEXT("")),
        Password(TEXT("")),
        AutoConnectOnStartup(false)
    {
    }

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
        meta = (ToolTip = "The address of the Archipelago server to connect to, without the port number."))
    FString ServerUrl;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Config",
        meta = (
            ToolTip = "The port of the Archipelago server. Use the port provided on your multiworld room page.",
            ClampMin = "1",
            ClampMax = "65535",
            UIMin = "1",
            UIMax = "65535"
            ))
    int32 ServerPort = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Config",
        meta = (ToolTip = "The player slot name to connect to in the Archipelago multiworld."))
    FString Slot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Config",
        meta = (ToolTip = "The password required to connect to the selected player slot, if one is configured."))
    FString Password;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Config",
        meta = (ToolTip = "Automatically attempts to connect to the Archipelago server when the game starts."))
    bool AutoConnectOnStartup;
};

#pragma endregion


#pragma region Protocol

USTRUCT(BlueprintType, meta = (ToolTip = "Represents an Archipelago protocol version, including its major, minor and build numbers."))
struct FAPVersion
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Protocol",
        meta = (ToolTip = "The major version number."))
    int32 Major = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Protocol",
        meta = (ToolTip = "The minor version number."))
    int32 Minor = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Protocol",
        meta = (ToolTip = "The build version number."))
    int32 Build = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Protocol",
        meta = (ToolTip = "The version class identifier supplied by Archipelago."))
    FString Class = TEXT("Version");
};

USTRUCT(BlueprintType, meta = (ToolTip = "Contains information about an invalid packet reported by the Archipelago server."))
struct FAPInvalidPacket
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Protocol",
        meta = (ToolTip = "The type of invalid packet reported by the server."))
    FString Type;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Protocol",
        meta = (ToolTip = "The original command that caused the invalid packet response."))
    FString OriginalCommand;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Protocol",
        meta = (ToolTip = "The error message returned by the Archipelago server."))
    FString Message;
};

#pragma endregion

#pragma region Room

USTRUCT(BlueprintType, meta = (ToolTip = "Contains the permissions configured for an Archipelago multiworld room."))
struct FAPRoomPermissions
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room",
        meta = (ToolTip = "The permission level required to release items."))
    EAPPermission Release = EAPPermission::Disabled;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room",
        meta = (ToolTip = "The permission level required to retrieve remaining items."))
    EAPPermission Remaining = EAPPermission::Disabled;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room",
        meta = (ToolTip = "The permission level required to collect items."))
    EAPPermission Collect = EAPPermission::Disabled;
};

USTRUCT(BlueprintType, meta = (ToolTip = "Contains information about the connected Archipelago room, received through the RoomInfo packet."))
struct FAPRoomInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room",
        meta = (ToolTip = "Indicates whether the room requires a password to connect."))
    bool bPasswordRequired = false;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room",
        meta = (ToolTip = "The list of games present in the multiworld room."))
    TArray<FString> Games;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room",
        meta = (ToolTip = "The list of tags associated with the room."))
    TArray<FString> Tags;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room",
        meta = (ToolTip = "The version of the Archipelago server."))
    FAPVersion Version;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room",
        meta = (ToolTip = "The version of the generator used to create the multiworld."))
    FAPVersion GeneratorVersion;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room",
        meta = (ToolTip = "The permissions configured for the room."))
    FAPRoomPermissions Permissions;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room",
        meta = (ToolTip = "The number of hint points required to purchase a hint."))
    int32 HintCost = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room",
        meta = (ToolTip = "The number of location checks required to earn a hint point."))
    int32 LocationCheckPoints = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room",
        meta = (ToolTip = "Maps game names to their corresponding DataPackage checksums."))
    TMap<FString, FString> DataPackageChecksums;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room",
        meta = (ToolTip = "The name of the generated multiworld seed."))
    FString SeedName;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room",
        meta = (ToolTip = "The server time associated with the room information."))
    double Time = 0.0;
};

USTRUCT(BlueprintType, meta = (ToolTip = "Represents a location that has been checked in the Archipelago multiworld."))
struct FAPCheckedLocation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room",
        meta = (ToolTip = "The unique Archipelago location ID."))
    int64 ID = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room",
        meta = (ToolTip = "The display name of the checked location."))
    FString Name;
};

USTRUCT(BlueprintType, meta = (ToolTip = "Contains the updated room information received through an Archipelago RoomUpdate packet."))
struct FAPRoomUpdate
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room",
        meta = (ToolTip = "The current number of hint points available to the connected player."))
    int32 HintPoints = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Room",
        meta = (ToolTip = "The list of locations currently checked by the connected player."))
    TArray<FAPCheckedLocation> CheckedLocations;
};

#pragma endregion

#pragma region Players

USTRUCT(BlueprintType, meta = (ToolTip = "Contains the identity and information of a player in the Archipelago multiworld."))
struct FAPPlayer
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Players",
        meta = (ToolTip = "The team ID this player belongs to."))
    int32 Team = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Players",
        meta = (ToolTip = "The unique player slot ID within the multiworld."))
    int32 Slot = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Players",
        meta = (ToolTip = "The player's currently displayed alias."))
    FString Alias;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Players",
        meta = (ToolTip = "The player's original slot name."))
    FString Name;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Players",
        meta = (ToolTip = "The player classification, such as a normal player or group."))
    FString PlayerClass;
};

USTRUCT(BlueprintType, meta = (ToolTip = "Contains information about a player slot, including its name, game and slot type."))
struct FAPSlotInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Players",
        meta = (ToolTip = "The name of the player slot."))
    FString Name;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Players",
        meta = (ToolTip = "The game associated with this player slot."))
    FString Game;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Players",
        meta = (ToolTip = "The slot type reported by Archipelago."))
    int32 Type = 0;
};

#pragma endregion

#pragma region Items

USTRUCT(BlueprintType, meta = (ToolTip = "Represents an item received from or sent through the Archipelago multiworld network."))
struct FAPNetworkItem
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Items",
        meta = (ToolTip = "The unique ID of the item."))
    int64 Item = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Items",
        meta = (ToolTip = "The location ID where the item originated."))
    int64 Location = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Items",
        meta = (ToolTip = "The player slot ID that sent the item."))
    int32 Player = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Items",
        meta = (ToolTip = "The item classification flags represented as a bitfield."))
    int32 Flags = 0;
};

USTRUCT(BlueprintType, meta = (ToolTip = "Contains a network item together with its resolved names and item classifications."))
struct FAPResolvedNetworkItem
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Items",
        meta = (ToolTip = "The resolved display name of the item."))
    FString ItemName;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Items",
        meta = (ToolTip = "The resolved display name of the location where the item originated."))
    FString LocationName;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Items",
        meta = (ToolTip = "The resolved display name of the player who sent the item."))
    FString PlayerName;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Items",
        meta = (ToolTip = "The unique ID of the item."))
    int64 ItemID = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Items",
        meta = (ToolTip = "The ID of the location where the item originated."))
    int64 LocationID = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Items",
        meta = (ToolTip = "The player slot ID that sent the item."))
    int32 PlayerSlot = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Items",
        meta = (ToolTip = "The classifications assigned to the item, such as Advancement, Useful or Trap."))
    TSet<EAPItemClassification> Classifications;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Items",
        meta = (ToolTip = "Indicates whether the item ID was successfully resolved into an item name."))
    bool bItemNameFound = false;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Items",
        meta = (ToolTip = "Indicates whether the location ID was successfully resolved into a location name."))
    bool bLocationNameFound = false;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Items",
        meta = (ToolTip = "Indicates whether the player slot ID was successfully resolved into a player name."))
    bool bPlayerNameFound = false;
};

#pragma endregion

#pragma region GameData

USTRUCT(BlueprintType, meta = (ToolTip = "Contains the item and location name mappings for a specific Archipelago game."))
struct FAPGameData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Game Data",
        meta = (ToolTip = "Maps item IDs to their corresponding display names."))
    TMap<int64, FString> ItemNames;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Game Data",
        meta = (ToolTip = "Maps location IDs to their corresponding display names."))
    TMap<int64, FString> LocationNames;
};

#pragma endregion

#pragma region Messages

USTRUCT(BlueprintType, meta = (ToolTip = "Represents an individual segment of an Archipelago PrintJSON message."))
struct FAPPrintJSONSegment
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Messages",
        meta = (ToolTip = "The text content of this message segment."))
    FString Text;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Messages",
        meta = (ToolTip = "The original segment type supplied by Archipelago."))
    FString Type;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Messages",
        meta = (ToolTip = "The display color assigned to this message segment by the client."))
    FLinearColor Color = FLinearColor::White;
};

USTRUCT(BlueprintType, meta = (ToolTip = "Contains a complete Archipelago PrintJSON message, including its individual segments and combined plain text."))
struct FAPPrintJSONMessage
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Messages",
        meta = (ToolTip = "The individual text segments that make up this message."))
    TArray<FAPPrintJSONSegment> Segments;

    UPROPERTY(BlueprintReadOnly, Category = "Archipelago|Messages",
        meta = (ToolTip = "All message segments combined into a single plain text string."))
    FString PlainText;
};

USTRUCT(BlueprintType, meta = (ToolTip = "Defines the colors used to display different types of Archipelago messages."))
struct FAPMessageTypeColorPalette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Messages|Colors",
        meta = (ToolTip = "The color used for regular message text."))
    FLinearColor TextColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Messages|Colors",
        meta = (ToolTip = "The color used for player names."))
    FLinearColor PlayerColor = FLinearColor(0.35f, 0.75f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Messages|Colors",
        meta = (ToolTip = "The color used for item names."))
    FLinearColor ItemColor = FLinearColor(1.0f, 0.75f, 0.2f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Messages|Colors",
        meta = (ToolTip = "The color used for location names."))
    FLinearColor LocationColor = FLinearColor(0.35f, 0.9f, 0.45f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Messages|Colors",
        meta = (ToolTip = "The color used for entrance names."))
    FLinearColor EntranceColor = FLinearColor(0.75f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Messages|Colors",
        meta = (ToolTip = "The color used for hint-related messages."))
    FLinearColor HintColor = FLinearColor(1.0f, 0.6f, 0.2f);
};

#pragma endregion