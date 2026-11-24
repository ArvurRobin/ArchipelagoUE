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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAPConnectionClosed,int32, StatusCode, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAPConnectionError, const FString&, Error);

UCLASS()
class ARCHIPELAGOUE_API UArchipelagoSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

public:

    UPROPERTY(BlueprintAssignable, Category = "Archipelago|Events")
    FOnAPConnectionStarted OnConnectionStarted;

    UPROPERTY(BlueprintAssignable, Category = "Archipelago|Events")
    FOnAPConnected OnConnected;

    UPROPERTY(BlueprintAssignable, Category = "Archipelago|Events")
    FOnAPConnectionClosed OnConnectionClosed;

    UPROPERTY(BlueprintAssignable, Category = "Archipelago|Events")
    FOnAPConnectionError OnConnectionError;

#pragma region Config

    // Get the configuration file path
    UFUNCTION(BlueprintPure, Category = "Archipelago|Config", meta = (DisplayName = "Get Config Path", ToolTip = "Returns the absolute Path to where the Archipelago Config File should be located on the file-system"))
    FString GetConfigPath() const;

    // Access the loaded configuration
    UFUNCTION(BlueprintPure, Category = "Archipelago|Config", meta = (DisplayName = "Get Archipelago Config", ToolTip = "Returns the read-only Archipelago Config currently loaded"))
    const FAPConfig& GetConfig() const
    {
        return Config;
    }

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Config", meta = (DisplayName = "Override Config", ToolTip = "Overrides the currently loaded Archipelago Config with a new one"))
    bool OverrideConfig(const FAPConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Config", meta = (DisplayName = "Create New Config", ToolTip = "Creates a new Archipelago Config with default values and saves it, optionally overriding an existing Config file"))
    EAPConfigCreateResult CreateNewConfig(bool bOverrideExisting = false);

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Config", meta = (DisplayName = "Load Config", ToolTip = "Loads an existing Archipelago Config file and sets it as the currently loaded Config"))
    EAPConfigLoadResult LoadConfig();

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Config", meta = (DisplayName = "Save Config", ToolTip = "Saves the currently loaded Config to the Archipelago Config File"))
    EAPConfigSaveResult SaveConfig();

private:

    FAPConfig Config;

    static FString EncodePassword(const FString& Password);
    static FString DecodePassword(const FString& EncodedPassword);

#pragma endregion

#pragma region WebSocket Connection

public:
    UFUNCTION(BlueprintPure, Category = "Archipelago|WebSocket", meta = (DisplayName = "Get WebSocket Name", ToolTip = "Returns the Name to use for the Archipelago WebSocket on the WebSocketsSubsystem"))
    static FName GetWebSocketName();

    UFUNCTION(BlueprintCallable, Category = "Archipelago|WebSocket", meta = (DisplayName = "Init WebSocket", ToolTip = "Inits a WebSocket to the Server defined in the Archipelago Config, but doesn't connect to it"))
    EAPWebSocketInitResult InitWebSocket();

    UFUNCTION(BlueprintCallable, Category = "Archipelago|WebSocket", meta = (DisplayName = "Connect WebSocket", ToolTip = "Establishs a WebSocket-Connection that needs to initialized before"))
    EAPWebSocketConnectionResult ConnectWebSocket();

    UFUNCTION(BlueprintCallable, Category = "Archipelago|WebSocket", meta = (DisplayName = "Destroy WebSocket", ToolTip = "Destroys the WebSocket-Connection that needs to exist"))
    EAPWebSocketDestroyResult DestroyWebSocket();

private:
    UWebSocket* ArchipelagoWebSocket;

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

    UFUNCTION(BlueprintPure, Category = "Archipelago|Room")
    FAPRoomInfo GetRoomInfo() const;

private:

    FAPRoomInfo RoomInfo;

    /**
     * Processes incoming Archipelago server messages.
     * Identifies the command and dispatches it to the appropriate handler.
     */
    void ProcessIncomingMessage(const FString& Message);

    // Handlers for incoming JSON Info through Archipelago WebSocket

    void HandleRoomInfo(const TSharedPtr<FJsonObject>& JsonObject);
    void HandleConnected(const TSharedPtr<FJsonObject>& JsonObject);
    void HandleConnectionRefused(const TSharedPtr<FJsonObject>& JsonObject);
    void HandleReceivedItems(const TSharedPtr<FJsonObject>& JsonObject);

    void ParseVersion(const TSharedPtr<FJsonObject>& JsonObject, FAPVersion& OutVersion);

#pragma endregion

};

