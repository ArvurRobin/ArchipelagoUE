// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

#include "ArchipelagoSubsystem.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/Base64.h"
#include "JsonObjectConverter.h"
#include "WebSocketsGameInstanceSubsystem.h"
#include "WebSocket.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/GameInstance.h"

void UArchipelagoSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogArchipelago, Log, TEXT("Initializing..."));

    const EAPConfigLoadResult ConfigLoadResult = LoadConfig();

    switch (ConfigLoadResult)
    {
    case EAPConfigLoadResult::Success:
        UE_LOG(LogArchipelago, Log, TEXT("Configuration loaded successfully."));
        break;

    case EAPConfigLoadResult::FileNotFound:
    {
        UE_LOG(LogArchipelago, Warning, TEXT("Configuration file does not exist. Creating a new default configuration..."));

        const EAPConfigCreateResult ConfigCreateResult = CreateNewConfig(false);

        switch (ConfigCreateResult)
        {
        case EAPConfigCreateResult::Success:
            UE_LOG(LogArchipelago, Log, TEXT("Default configuration created successfully."));
            break;

        case EAPConfigCreateResult::AlreadyExists:
            UE_LOG(LogArchipelago, Warning, TEXT("Configuration file already exists. Skipping creation."));
            break;

        case EAPConfigCreateResult::DirectoryCreationFailed:
            UE_LOG(LogArchipelago, Error, TEXT("Failed to create the configuration directory."));
            break;

        case EAPConfigCreateResult::JsonConversionFailed:
            UE_LOG(LogArchipelago, Error, TEXT("Failed to convert the default configuration to JSON."));
            break;

        case EAPConfigCreateResult::FileWriteFailed:
            UE_LOG(LogArchipelago, Error, TEXT("Failed to write the configuration file."));
            break;

        default:
            UE_LOG(LogArchipelago, Error, TEXT("Unknown error while creating the configuration file."));
            break;
        }

        break;
    }

    case EAPConfigLoadResult::FileReadFailed:
        UE_LOG(LogArchipelago, Error, TEXT("Could not read configuration file."));
        break;

    case EAPConfigLoadResult::JsonConversionFailed:
        UE_LOG(LogArchipelago, Error, TEXT("JSON configuration conversion failed."));
        break;

    default:
        UE_LOG(LogArchipelago, Fatal, TEXT("Unknown error loading configuration!"));
        break;
    }

    // Init WebSocket
    const EAPWebSocketInitResult InitResult = InitWebSocket();

    if (InitResult == EAPWebSocketInitResult::Success ||
        InitResult == EAPWebSocketInitResult::AlreadyInitialized)
    {
        UWebSocketsSubsystem* WebSocketsSubsystem =
            GetGameInstance()->GetSubsystem<UWebSocketsSubsystem>();

        if (IsValid(WebSocketsSubsystem))
        {
            ArchipelagoWebSocket =
                WebSocketsSubsystem->GetWebSocket(GetWebSocketName());
        }

        SetupWebSocketDelegates();

        if (GetConfig().AutoConnectOnStartup)
        {
            ConnectWebSocket();
        }
    }
}

void UArchipelagoSubsystem::Deinitialize()
{
    DestroyWebSocket();

    Super::Deinitialize();
}

FString UArchipelagoSubsystem::GetConfigPath() const
{
    return FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("APConfig.json"));
}

bool UArchipelagoSubsystem::OverrideConfig(const FAPConfig& NewConfig)
{
    Config = NewConfig;
    return true;
}

EAPConfigCreateResult UArchipelagoSubsystem::CreateNewConfig(bool bOverrideExisting)
{
    const FString ConfigFilePath = GetConfigPath();
    const FString ConfigDirectory = FPaths::GetPath(ConfigFilePath);

    // Check whether the config already exists.
    if (FPaths::FileExists(ConfigFilePath) && !bOverrideExisting)
    {
        return EAPConfigCreateResult::AlreadyExists;
    }

    // Ensure the Config directory exists.
    if (!IFileManager::Get().DirectoryExists(*ConfigDirectory))
    {
        if (!IFileManager::Get().MakeDirectory(*ConfigDirectory, true))
        {
            return EAPConfigCreateResult::DirectoryCreationFailed;
        }
    }

    // Generate default config JSON.
    const FAPConfig DefaultConfig;

    FString JsonString;
    if (!FJsonObjectConverter::UStructToJsonObjectString(
        FAPConfig::StaticStruct(),
        &DefaultConfig,
        JsonString,
        0,
        0))
    {
        return EAPConfigCreateResult::JsonConversionFailed;
    }

    // Write the config file.
    if (!FFileHelper::SaveStringToFile(JsonString, *ConfigFilePath))
    {
        return EAPConfigCreateResult::FileWriteFailed;
    }

    // Update the in-memory config.
    Config = DefaultConfig;

    return EAPConfigCreateResult::Success;
}

EAPConfigLoadResult UArchipelagoSubsystem::LoadConfig()
{
    const FString ConfigFilePath = GetConfigPath();

    FString JsonString;

    if (!FPaths::FileExists(ConfigFilePath))
    {
        return EAPConfigLoadResult::FileNotFound;
    }

    if (!FFileHelper::LoadFileToString(JsonString, *ConfigFilePath))
    {
        return EAPConfigLoadResult::FileReadFailed;
    }

    FAPConfig LoadedConfig;

    if (!FJsonObjectConverter::JsonObjectStringToUStruct<FAPConfig>(
        JsonString,
        &LoadedConfig))
    {
        return EAPConfigLoadResult::JsonConversionFailed;
    }

    LoadedConfig.Password = DecodePassword(LoadedConfig.Password);

    Config = MoveTemp(LoadedConfig);

    return EAPConfigLoadResult::Success;
}

EAPConfigSaveResult UArchipelagoSubsystem::SaveConfig()
{
    const FString ConfigFilePath = GetConfigPath();
    const FString ConfigDirectory = FPaths::GetPath(ConfigFilePath);

    // Ensure the Config directory exists.
    if (!IFileManager::Get().DirectoryExists(*ConfigDirectory))
    {
        if (!IFileManager::Get().MakeDirectory(*ConfigDirectory, true))
        {
            return EAPConfigSaveResult::DirectoryCreationFailed;
        }
    }

    // Encode the Password before saving it
    FAPConfig ConfigToSave = Config;
    ConfigToSave.Password = EncodePassword(Config.Password);

    FString JsonString;

    if (!FJsonObjectConverter::UStructToJsonObjectString(
        FAPConfig::StaticStruct(),
        &ConfigToSave,
        JsonString,
        0,
        0))
    {
        return EAPConfigSaveResult::JsonConversionFailed;
    }

    // Write the JSON to disk.
    if (!FFileHelper::SaveStringToFile(JsonString, *ConfigFilePath))
    {
        return EAPConfigSaveResult::FileWriteFailed;
    }

    return EAPConfigSaveResult::Success;
}

FString UArchipelagoSubsystem::EncodePassword(const FString& Password)
{
    const FTCHARToUTF8 Converted(*Password);

    return FBase64::Encode(
        reinterpret_cast<const uint8*>(Converted.Get()),
        Converted.Length()
    );
}

FString UArchipelagoSubsystem::DecodePassword(const FString& EncodedPassword)
{
    TArray<uint8> DecodedData;

    if (!FBase64::Decode(EncodedPassword, DecodedData))
    {
        return FString();
    }

    FUTF8ToTCHAR Converted(
        reinterpret_cast<const ANSICHAR*>(DecodedData.GetData()),
        DecodedData.Num()
    );

    return FString(Converted.Length(), Converted.Get());
}

FName UArchipelagoSubsystem::GetWebSocketName()
{
    static const FName WebSocketName(TEXT("Archipelago"));
    return WebSocketName;
}

EAPWebSocketInitResult UArchipelagoSubsystem::InitWebSocket()
{
    // Get WebSockets Subsystem
    UWebSocketsSubsystem* WebSocketsSubsystem = GetGameInstance()->GetSubsystem<UWebSocketsSubsystem>();
    if (!IsValid(WebSocketsSubsystem))
    {
        return EAPWebSocketInitResult::SubsystemNotAccessible;
    }

    // Check if WebSocket is already initialized
    if (IsValid(WebSocketsSubsystem->GetWebSocket(GetWebSocketName())))
    {
        return EAPWebSocketInitResult::AlreadyInitialized;
    }

    FString ServerUrl = GetConfig().ServerUrl;
    const int32 ServerPort = GetConfig().ServerPort;

    // Check if URL is empty
    if (ServerUrl.IsEmpty())
    {
        return EAPWebSocketInitResult::InvalidURL;
    }

    // Ensure the URL has a WebSocket protocol.
    // Default to secure WebSocket (WSS) if no protocol is specified.
    if (!ServerUrl.StartsWith(TEXT("ws://"), ESearchCase::IgnoreCase) &&
        !ServerUrl.StartsWith(TEXT("wss://"), ESearchCase::IgnoreCase))
    {
        ServerUrl.InsertAt(0, TEXT("wss://"));
    }

    // Check port
    if (ServerPort <= 0 || ServerPort > 65535)
    {
        return EAPWebSocketInitResult::InvalidPort;
    }

    // Combine server URL and port
    const FString ServerAndPort = FString::Printf(
        TEXT("%s:%d"),
        *ServerUrl,
        ServerPort
    );

    UE_LOG(
        LogArchipelago,
        Warning,
        TEXT(">>> TARGET URL: %s <<<"),
        *ServerAndPort
    );

    // Create WebSocket
    UWebSocket* WebSocket;
    FString OutMessage;

    const bool bSuccess = WebSocketsSubsystem->CreateWebSocket(
        GetWebSocketName(),
        ServerAndPort,
        WebSocket,
        OutMessage
    );

    if (!bSuccess || !IsValid(WebSocket))
    {
        // Handle creation failure
        WebSocket = nullptr;
        return EAPWebSocketInitResult::CreationFailed;
    }

    return EAPWebSocketInitResult::Success;
}

EAPWebSocketConnectionResult UArchipelagoSubsystem::ConnectWebSocket()
{
    UWebSocketsSubsystem* WebSocketsSubsystem =
        GetGameInstance()->GetSubsystem<UWebSocketsSubsystem>();

    if (!IsValid(WebSocketsSubsystem))
    {
        UE_LOG(LogArchipelago, Error, TEXT("WebSocketsSubsystem is not accessible."));
        return EAPWebSocketConnectionResult::SubsystemNotAccessible;
    }

    if (!IsValid(ArchipelagoWebSocket))
    {
        UE_LOG(LogArchipelago, Error, TEXT("Archipelago WebSocket is invalid."));
        return EAPWebSocketConnectionResult::WebSocketNotFound;
    }

    if (!ArchipelagoWebSocket->IsInitialized())
    {
        UE_LOG(LogArchipelago, Error, TEXT("Archipelago WebSocket is not initialized."));
        return EAPWebSocketConnectionResult::WebSocketNotInitialized;
    }

    UE_LOG(LogArchipelago, Log, TEXT("Attempting WebSocket connection..."));

    ArchipelagoWebSocket->Connect();
    OnConnectionStarted.Broadcast();

    UE_LOG(LogArchipelago, Log, TEXT("WebSocket connection request submitted."));

    return EAPWebSocketConnectionResult::Success;
}

EAPWebSocketDestroyResult UArchipelagoSubsystem::DestroyWebSocket()
{
    // Get WebSockets Subsystem
    UWebSocketsSubsystem* WebSocketsSubsystem = GetGameInstance()->GetSubsystem<UWebSocketsSubsystem>();
    if (!IsValid(WebSocketsSubsystem))
    {
        return EAPWebSocketDestroyResult::SubsystemNotAccessible;
    }

    // Is WebSocket initialized?
    if (!IsValid(ArchipelagoWebSocket))
    {
        return EAPWebSocketDestroyResult::WebSocketNotFound;
    }

    FString DestroyMessage;

    if (WebSocketsSubsystem->DestroyWebSocket(GetWebSocketName(), DestroyMessage))
    {
        ArchipelagoWebSocket = nullptr;
        return EAPWebSocketDestroyResult::Success;
    }

    return EAPWebSocketDestroyResult::WebSocketNotDestroyed;
}

void UArchipelagoSubsystem::SetupWebSocketDelegates()
{
    if (!IsValid(ArchipelagoWebSocket))
    {
        return;
    }

    // Setup Delegates
    ArchipelagoWebSocket->OnWebSocketConnected.AddUniqueDynamic(
        this, &UArchipelagoSubsystem::HandleWebSocketConnected);

    ArchipelagoWebSocket->OnWebSocketConnectionError.AddUniqueDynamic(
        this, &UArchipelagoSubsystem::HandleWebSocketConnectionError);

    ArchipelagoWebSocket->OnWebSocketClosed.AddUniqueDynamic(
        this, &UArchipelagoSubsystem::HandleWebSocketClosed);

    ArchipelagoWebSocket->OnWebSocketMessageReceived.AddUniqueDynamic(
        this, &UArchipelagoSubsystem::HandleWebSocketMessageReceived);

    ArchipelagoWebSocket->OnWebSocketMessageSent.AddUniqueDynamic(
        this, &UArchipelagoSubsystem::HandleWebSocketMessageSent);
}

void UArchipelagoSubsystem::HandleWebSocketConnected()
{
    // WebSocket successfully connected
    UE_LOG(LogArchipelago, Log, TEXT("WebSocket connected!"));

    OnConnected.Broadcast();
}

void UArchipelagoSubsystem::HandleWebSocketConnectionError(const FString& Error)
{
    // Handle connection error
    UE_LOG(LogArchipelago, Error, TEXT("WebSocket Connection Error:\n%s"), *Error);

    OnConnectionError.Broadcast(Error);
}

void UArchipelagoSubsystem::HandleWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
    UE_LOG(
        LogArchipelago,
        Warning,
        TEXT("WebSocket Closed | Code: %d | Reason: '%s' | Clean: %s"),
        StatusCode,
        *Reason,
        bWasClean ? TEXT("true") : TEXT("false")
    );

    OnConnectionClosed.Broadcast(StatusCode, Reason);
}

void UArchipelagoSubsystem::HandleWebSocketMessageReceived(const FString& Message)
{
    // Handle incoming message
    //UE_LOG(LogArchipelago, Log, TEXT("Message Received:\n%s"), *Message);
    ProcessIncomingMessage(Message);
}

void UArchipelagoSubsystem::HandleWebSocketMessageSent(const FString& Message)
{
    // Handle outgoing message
    UE_LOG(LogArchipelago, Log, TEXT("Message Sent:\n%s"), *Message);
}

FAPRoomInfo UArchipelagoSubsystem::GetRoomInfo() const
{
    return RoomInfo;
}

void UArchipelagoSubsystem::ProcessIncomingMessage(const FString& Message)
{
    // Parse incoming JSON.
    TArray<TSharedPtr<FJsonValue>> JsonArray;

    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);

    if (!FJsonSerializer::Deserialize(Reader, JsonArray))
    {
        UE_LOG(LogArchipelago, Error, TEXT("Failed to parse incoming Archipelago message."));
        return;
    }

    // Archipelago messages are JSON arrays.
    for (const TSharedPtr<FJsonValue>& JsonValue : JsonArray)
    {
        if (!JsonValue.IsValid() || JsonValue->Type != EJson::Object)
        {
            UE_LOG(LogArchipelago, Warning, TEXT("Received invalid Archipelago message entry."));
            continue;
        }

        const TSharedPtr<FJsonObject> JsonObject = JsonValue->AsObject();

        if (!JsonObject.IsValid())
        {
            continue;
        }

        // Identify the incoming command.
        FString Command;

        if (!JsonObject->TryGetStringField(TEXT("cmd"), Command))
        {
            UE_LOG(LogArchipelago, Warning, TEXT("Received Archipelago message without a command."));
            continue;
        }

        UE_LOG(LogArchipelago, Log, TEXT("Received Archipelago command: %s"), *Command);

        // Dispatch command.
        if (Command == TEXT("RoomInfo"))
        {
            HandleRoomInfo(JsonObject);
        }
        else if (Command == TEXT("Connected"))
        {
            HandleConnected(JsonObject);
        }
        else if (Command == TEXT("ConnectionRefused"))
        {
            HandleConnectionRefused(JsonObject);
        }
        else if (Command == TEXT("ReceivedItems"))
        {
            HandleReceivedItems(JsonObject);
        }
        else
        {
            UE_LOG(LogArchipelago, Verbose, TEXT("Unhandled Archipelago command: %s"), *Command);
        }
    }
}

void UArchipelagoSubsystem::HandleRoomInfo(
    const TSharedPtr<FJsonObject>& JsonObject)
{
    if (!JsonObject.IsValid())
    {
        UE_LOG(LogArchipelago, Error, TEXT("Invalid RoomInfo JSON object."));
        return;
    }

    UE_LOG(LogArchipelago, Log, TEXT("Processing RoomInfo..."));

    // Clean Room Info
    RoomInfo = FAPRoomInfo();

    // Password required
    JsonObject->TryGetBoolField(TEXT("password"), RoomInfo.bPasswordRequired);

    // Games
    const TArray<TSharedPtr<FJsonValue>>* GamesArray = nullptr;

    if (JsonObject->TryGetArrayField(TEXT("games"), GamesArray))
    {
        for (const TSharedPtr<FJsonValue>& Value : *GamesArray)
        {
            if (Value.IsValid())
            {
                RoomInfo.Games.Add(Value->AsString());
            }
        }
    }

    // Tags
    const TArray<TSharedPtr<FJsonValue>>* TagsArray = nullptr;

    if (JsonObject->TryGetArrayField(TEXT("tags"), TagsArray))
    {
        for (const TSharedPtr<FJsonValue>& Value : *TagsArray)
        {
            if (Value.IsValid())
            {
                RoomInfo.Tags.Add(Value->AsString());
            }
        }
    }

    // Seed name
    JsonObject->TryGetStringField(TEXT("seed_name"), RoomInfo.SeedName);

    // Time
    JsonObject->TryGetNumberField(TEXT("time"), RoomInfo.Time);

    // Hint cost
    JsonObject->TryGetNumberField(TEXT("hint_cost"), RoomInfo.HintCost);

    // Location check points
    JsonObject->TryGetNumberField(
        TEXT("location_check_points"),
        RoomInfo.LocationCheckPoints);

    // Version
    const TSharedPtr<FJsonObject>* VersionObject = nullptr;

    if (JsonObject->TryGetObjectField(TEXT("version"), VersionObject))
    {
        ParseVersion(*VersionObject, RoomInfo.Version);
    }

    // Generator version
    const TSharedPtr<FJsonObject>* GeneratorVersionObject = nullptr;

    if (JsonObject->TryGetObjectField(TEXT("generator_version"), GeneratorVersionObject))
    {
        ParseVersion(*GeneratorVersionObject, RoomInfo.GeneratorVersion);
    }

    // Permissions
    const TSharedPtr<FJsonObject>* PermissionsObject = nullptr;

    if (JsonObject->TryGetObjectField(TEXT("permissions"), PermissionsObject))
    {
        int32 PermissionValue = 0;

        if ((*PermissionsObject)->TryGetNumberField(TEXT("release"), PermissionValue))
        {
            RoomInfo.Permissions.Release = static_cast<EAPPermission>(PermissionValue);
        }

        if ((*PermissionsObject)->TryGetNumberField(TEXT("remaining"), PermissionValue))
        {
            RoomInfo.Permissions.Remaining = static_cast<EAPPermission>(PermissionValue);
        }

        if ((*PermissionsObject)->TryGetNumberField(TEXT("collect"), PermissionValue))
        {
            RoomInfo.Permissions.Collect = static_cast<EAPPermission>(PermissionValue);
        }
    }

    // Data package checksums
    const TSharedPtr<FJsonObject>* ChecksumsObject = nullptr;

    if (JsonObject->TryGetObjectField(TEXT("datapackage_checksums"), ChecksumsObject))
    {
        for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : (*ChecksumsObject)->Values)
        {
            if (Pair.Value.IsValid())
            {
                RoomInfo.DataPackageChecksums.Add(
                    Pair.Key,
                    Pair.Value->AsString());
            }
        }
    }

    UE_LOG(
        LogArchipelago,
        Log,
        TEXT("RoomInfo processed successfully. Games: %d | Tags: %d | Seed: %s"),
        RoomInfo.Games.Num(),
        RoomInfo.Tags.Num(),
        *RoomInfo.SeedName);
}

void UArchipelagoSubsystem::HandleConnected(const TSharedPtr<FJsonObject>& JsonObject)
{
    UE_LOG(LogArchipelago, Log, TEXT("Processing Connected."));
}

void UArchipelagoSubsystem::HandleConnectionRefused(const TSharedPtr<FJsonObject>& JsonObject)
{
    UE_LOG(LogArchipelago, Warning, TEXT("Processing ConnectionRefused."));
}

void UArchipelagoSubsystem::HandleReceivedItems(const TSharedPtr<FJsonObject>& JsonObject)
{
    UE_LOG(LogArchipelago, Log, TEXT("Processing ReceivedItems."));
}

void UArchipelagoSubsystem::ParseVersion(
    const TSharedPtr<FJsonObject>& JsonObject,
    FAPVersion& OutVersion)
{
    if (!JsonObject.IsValid())
    {
        return;
    }

    int32 Major = 0;
    int32 Minor = 0;
    int32 Build = 0;

    JsonObject->TryGetNumberField(TEXT("major"), Major);
    JsonObject->TryGetNumberField(TEXT("minor"), Minor);
    JsonObject->TryGetNumberField(TEXT("build"), Build);
    JsonObject->TryGetStringField(TEXT("class"), OutVersion.Class);

    OutVersion.Major = Major;
    OutVersion.Minor = Minor;
    OutVersion.Build = Build;
}
