// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ArchipelagoStructs.h"
#include "ArchipelagoLog.h"
#include "WebSocket.h"
#include "Dom/JsonObject.h"
#include "ArchipelagoSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAPConnectionStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAPConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAPConnectionClosed, int32, StatusCode, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAPConnectionError, const FString&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAPPrintJSON, const FAPPrintJSONMessage&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAPReceivedItems, const TArray<FAPResolvedNetworkItem>&, ReceivedItems);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAPLocationInfoReceived, const TArray<FAPResolvedNetworkItem>&, LocationInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAPRoomUpdated, const FAPRoomUpdate&, RoomUpdate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAPPrintReceived, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAPInvalidPacketReceived, const FAPInvalidPacket&, InvalidPacket);

UCLASS()
class ARCHIPELAGOUE_API UArchipelagoSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

#pragma region Events

    UPROPERTY(BlueprintAssignable, Category = "Archipelago|Events",
        meta = (ToolTip = "Broadcast when an Archipelago WebSocket connection attempt is started."))
    FOnAPConnectionStarted OnConnectionStarted;

    UPROPERTY(BlueprintAssignable, Category = "Archipelago|Events",
        meta = (ToolTip = "Broadcast when the client successfully connects and is accepted into the Archipelago room."))
    FOnAPConnected OnConnected;

    UPROPERTY(BlueprintAssignable, Category = "Archipelago|Events",
        meta = (ToolTip = "Broadcast when the Archipelago WebSocket connection is closed, including the close status code and reason."))
    FOnAPConnectionClosed OnConnectionClosed;

    UPROPERTY(BlueprintAssignable, Category = "Archipelago|Events",
        meta = (ToolTip = "Broadcast when an error occurs during the WebSocket connection."))
    FOnAPConnectionError OnConnectionError;

    UPROPERTY(BlueprintAssignable, Category = "Archipelago|Events",
        meta = (ToolTip = "Broadcast when a PrintJSON message is received, including its individual text segments and resolved colors."))
    FOnAPPrintJSON OnPrintJSON;

    UPROPERTY(BlueprintAssignable, Category = "Archipelago|Events",
        meta = (ToolTip = "Broadcast when a legacy Print message is received from the Archipelago server."))
    FOnAPPrintReceived OnPrint;

    UPROPERTY(BlueprintAssignable, Category = "Archipelago|Events",
        meta = (ToolTip = "Broadcast when new items are received and resolved into their corresponding names and classifications."))
    FOnAPReceivedItems OnReceivedItems;

    UPROPERTY(BlueprintAssignable, Category = "Archipelago|Events",
        meta = (ToolTip = "Broadcast when location scout information is received from the Archipelago server."))
    FOnAPLocationInfoReceived OnReceivedLocationInfo;

    UPROPERTY(BlueprintAssignable, Category = "Archipelago|Events",
        meta = (ToolTip = "Broadcast when updated room information is received, including hint points and checked locations."))
    FOnAPRoomUpdated OnRoomUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Archipelago|Events",
        meta = (ToolTip = "Broadcast when the Archipelago server reports an invalid packet."))
    FOnAPInvalidPacketReceived OnInvalidPacketReceived;

#pragma endregion


#pragma region Config

    UFUNCTION(BlueprintPure, Category = "Archipelago|Config",
        meta = (
            DisplayName = "Get Config Path",
            ToolTip = "Returns the absolute file-system path where the Archipelago configuration file is located."
            ))
    FString GetConfigPath() const;

    UFUNCTION(BlueprintPure, Category = "Archipelago|Config",
        meta = (
            DisplayName = "Get Archipelago Config",
            ToolTip = "Returns a read-only reference to the currently loaded Archipelago configuration."
            ))
    const FAPConfig& GetConfig() const
    {
        return Config;
    }

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Config",
        meta = (
            DisplayName = "Override Config",
            ToolTip = "Replaces the currently loaded Archipelago configuration with the supplied configuration."
            ))
    bool OverrideConfig(const FAPConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Config",
        meta = (
            DisplayName = "Create New Config",
            ToolTip = "Creates a new Archipelago configuration with default values and saves it. Optionally overwrites an existing configuration file."
            ))
    EAPConfigCreateResult CreateNewConfig(bool bOverrideExisting = false);

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Config",
        meta = (
            DisplayName = "Load Config",
            ToolTip = "Loads the Archipelago configuration file and sets it as the currently loaded configuration."
            ))
    EAPConfigLoadResult LoadConfig();

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Config",
        meta = (
            DisplayName = "Save Config",
            ToolTip = "Saves the currently loaded Archipelago configuration to the configuration file."
            ))
    EAPConfigSaveResult SaveConfig();

private:

    UPROPERTY(Transient,
        meta = (ToolTip = "The currently loaded Archipelago configuration."))
    FAPConfig Config;

    static FString EncodePassword(const FString& Password);
    static FString DecodePassword(const FString& EncodedPassword);

#pragma endregion


#pragma region WebSocket Connection

public:

    UFUNCTION(BlueprintPure, Category = "Archipelago|WebSocket",
        meta = (
            DisplayName = "Get WebSocket Name",
            ToolTip = "Returns the name used to identify the Archipelago WebSocket in the WebSocket subsystem."
            ))
    static FName GetWebSocketName();

    UFUNCTION(BlueprintCallable, Category = "Archipelago|WebSocket",
        meta = (
            DisplayName = "Init WebSocket",
            ToolTip = "Initializes a WebSocket using the server address and port defined in the Archipelago configuration. Does not establish a connection."
            ))
    EAPWebSocketInitResult InitWebSocket();

    UFUNCTION(BlueprintCallable, Category = "Archipelago|WebSocket",
        meta = (
            DisplayName = "Connect WebSocket",
            ToolTip = "Attempts to establish a connection using an initialized Archipelago WebSocket."
            ))
    EAPWebSocketConnectionResult ConnectWebSocket();

    UFUNCTION(BlueprintCallable, Category = "Archipelago|WebSocket",
        meta = (
            DisplayName = "Destroy WebSocket",
            ToolTip = "Destroys the currently registered Archipelago WebSocket."
            ))
    EAPWebSocketDestroyResult DestroyWebSocket();

private:

    UPROPERTY(Transient,
        meta = (ToolTip = "Reference to the WebSocket used for communication with the Archipelago server."))
    UWebSocket* ArchipelagoWebSocket = nullptr;

    void SetupWebSocketDelegates();

    UFUNCTION()
    void HandleWebSocketConnected();

    UFUNCTION()
    void HandleWebSocketConnectionError(const FString& Error);

    UFUNCTION()
    void HandleWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean);

    UFUNCTION()
    void HandleWebSocketMessageReceived(const FString& Message);

    UFUNCTION()
    void HandleWebSocketMessageSent(const FString& Message);

#pragma endregion


#pragma region Archipelago

public:

    UFUNCTION(BlueprintPure, Category = "Archipelago|Room",
        meta = (ToolTip = "Returns the information received about the currently connected Archipelago room."))
    FAPRoomInfo GetRoomInfo() const;

    UFUNCTION(BlueprintPure, Category = "Archipelago|Items",
        meta = (ToolTip = "Returns the complete history of network items received by this client."))
    TArray<FAPNetworkItem> GetReceivedItems() const;

    UFUNCTION(BlueprintCallable, Category = "Archipelago")
    void SetupArchipelago(
        UPARAM(DisplayName = "Game Name") const FString& InGameName,
        UPARAM(DisplayName = "Color Palette") const FAPMessageTypeColorPalette& InColorPalette
    );

    UFUNCTION(BlueprintPure, Category = "Archipelago|Messages",
        meta = (ToolTip = "Returns the display color assigned to the specified PrintJSON segment type."))
    FLinearColor GetPrintJSONColor(const FString& Type) const;

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Locations",
        meta = (ToolTip = "Sends a location check for a single location ID to the Archipelago server."))
    bool SendLocationCheck(int64 LocationId);

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Locations",
        meta = (ToolTip = "Sends location checks for multiple location IDs to the Archipelago server. Already checked and invalid locations are ignored."))
    bool SendLocationChecks(const TArray<int64>& LocationIds);

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Locations",
        meta = (ToolTip = "Requests scout information for a single location from the Archipelago server."))
    bool RequestLocationScout(int64 LocationId);

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Locations",
        meta = (ToolTip = "Requests scout information for multiple locations from the Archipelago server."))
    bool RequestLocationScouts(const TArray<int64>& LocationIds);

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Items",
        meta = (ToolTip = "Requests synchronization of the received item history with the Archipelago server."))
    bool RequestSync();

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Player",
        meta = (ToolTip = "Updates the client's gameplay status on the Archipelago server."))
    bool SendStatusUpdate(EAPClientStatus Status);

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Chat",
        meta = (ToolTip = "Sends a chat message to the connected Archipelago room."))
    bool SendChatMessage(const FString& Message);

    UFUNCTION(BlueprintPure, Category = "Archipelago|Items",
        meta = (ToolTip = "Resolves a network item into its display names and item classifications using the available game data."))
    FAPResolvedNetworkItem ResolveNetworkItem(const FAPNetworkItem& NetworkItem) const;

#pragma endregion


#pragma region Internal State

private:

    // The name of the game associated with this Archipelago client.
    UPROPERTY(Transient, meta = (ToolTip = "The name of the game associated with this Archipelago client."))
    FString GameName;

    // The unique identifier generated for this client connection.
    UPROPERTY(Transient, meta = (ToolTip = "The unique identifier generated for this client connection."))
    FString UUID;

    // The color palette used to display Archipelago PrintJSON messages.
    UPROPERTY(Transient, meta = (ToolTip = "The color palette used to display Archipelago PrintJSON messages."))
    FAPMessageTypeColorPalette ColorPalette;

    // The team ID assigned to this client after successfully connecting.
    UPROPERTY(Transient, meta = (ToolTip = "The team ID assigned to this client after successfully connecting."))
    int32 ConnectedTeam = INDEX_NONE;

    // The player slot ID assigned to this client after successfully connecting.
    UPROPERTY(Transient, meta = (ToolTip = "The player slot ID assigned to this client after successfully connecting."))
    int32 ConnectedSlot = INDEX_NONE;

    // The players currently known to the connected Archipelago room.
    UPROPERTY(Transient, meta = (ToolTip = "The players currently known to the connected Archipelago room."))
    TArray<FAPPlayer> Players;

    // The location IDs that have not yet been checked by the connected player.
    UPROPERTY(Transient, meta = (ToolTip = "The location IDs that have not yet been checked by the connected player."))
    TArray<int64> MissingLocations;

    // The location IDs that have already been checked by the connected player.
    UPROPERTY(Transient, meta = (ToolTip = "The location IDs that have already been checked by the connected player."))
    TSet<int64> CheckedLocations;

    // The current number of hint points available to the connected player.
    UPROPERTY(Transient, meta = (ToolTip = "The current number of hint points available to the connected player."))
    int32 HintPoints = 0;

    // Maps player slot IDs to their corresponding slot information.
    UPROPERTY(Transient, meta = (ToolTip = "Maps player slot IDs to their corresponding slot information."))
    TMap<int32, FAPSlotInfo> SlotInfo;

    // The complete history of network items received from the Archipelago server.
    UPROPERTY(Transient, meta = (ToolTip = "The complete history of network items received from the Archipelago server."))
    TArray<FAPNetworkItem> ReceivedItems;

    // The complete received item history with item, location and player names resolved.
    UPROPERTY(Transient, meta = (ToolTip = "The complete received item history with item, location and player names resolved."))
    TArray<FAPResolvedNetworkItem> ReceivedItemsResolved;

    // The current item history index used when synchronizing received items with the server.
    UPROPERTY(Transient, meta = (ToolTip = "The current item history index used when synchronizing received items with the server."))
    int32 ReceivedItemsIndex = 0;

    // The slot data received from the Archipelago server.
    TSharedPtr<FJsonObject> SlotData;

    // Information about the connected Archipelago room.
    FAPRoomInfo RoomInfo;

    // Game data indexed by game name, including item and location names.
    TMap<FString, FAPGameData> GameData;

#pragma endregion


#pragma region Internal Message Processing

    void ProcessIncomingMessage(const FString& Message);

    // Handlers for incoming Archipelago server commands.

    void HandleRoomInfo(const TSharedPtr<FJsonObject>& JsonObject);
    void HandleConnected(const TSharedPtr<FJsonObject>& JsonObject);
    void HandleConnectionRefused(const TSharedPtr<FJsonObject>& JsonObject);
    void HandleReceivedItems(const TSharedPtr<FJsonObject>& JsonObject);
    void HandlePrintJSON(const TSharedPtr<FJsonObject>& JsonObject);
    void HandlePrint(const TSharedPtr<FJsonObject>& JsonObject);
    void HandleDataPackage(const TSharedPtr<FJsonObject>& JsonObject);
    void HandleLocationInfo(const TSharedPtr<FJsonObject>& JsonObject);
    void HandleRoomUpdate(const TSharedPtr<FJsonObject>& JsonObject);
    void HandleInvalidPacket(const TSharedPtr<FJsonObject>& JsonObject);

    // Parsers and helpers.

    void ParseVersion(const TSharedPtr<FJsonObject>& JsonObject, FAPVersion& OutVersion);

    FAPPrintJSONSegment ParsePrintJSONSegment(const FAPPrintJSONSegment& Segment, int32 PlayerID);

    // Outgoing Archipelago messages.

    void SendConnectPacket();

    void RequestDataPackage(const TArray<FString>& GameNames);

#pragma endregion

};