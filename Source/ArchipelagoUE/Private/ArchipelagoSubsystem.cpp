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
    //UE_LOG(LogArchipelago, Log, TEXT("Message Sent:\n%s"), *Message);
}

FAPRoomInfo UArchipelagoSubsystem::GetRoomInfo() const
{
    return RoomInfo;
}

TArray<FAPNetworkItem> UArchipelagoSubsystem::GetReceivedItems() const
{
    return ReceivedItems;
}

void UArchipelagoSubsystem::SetupArchipelago(const FString& gameName, const FAPMessageTypeColorPalette& colorPalette)
{
    UUID = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
    GameName = gameName;
    ColorPalette = colorPalette;
}

FLinearColor UArchipelagoSubsystem::GetPrintJSONColor(const FString& Type) const
{
    if (Type == TEXT("player_id") || Type == TEXT("player_name"))
    {
        return ColorPalette.PlayerColor;
    }

    if (Type == TEXT("item_id") || Type == TEXT("item_name"))
    {
        return ColorPalette.ItemColor;
    }

    if (Type == TEXT("location_id") || Type == TEXT("location_name"))
    {
        return ColorPalette.LocationColor;
    }

    if (Type == TEXT("entrance_name"))
    {
        return ColorPalette.EntranceColor;
    }

    if (Type == TEXT("hint"))
    {
        return ColorPalette.HintColor;
    }

    return ColorPalette.TextColor;
}

bool UArchipelagoSubsystem::SendLocationCheck(int64 LocationId)
{
    return SendLocationChecks({ LocationId });
}

bool UArchipelagoSubsystem::SendLocationChecks(const TArray<int64>& LocationIds)
{
    if (!ArchipelagoWebSocket->IsConnected())
    {
        UE_LOG(LogArchipelago, Warning, TEXT("Cannot send LocationChecks: WebSocket is not connected."));
        return false;
    }

    TArray<TSharedPtr<FJsonValue>> JsonLocations;
    JsonLocations.Reserve(LocationIds.Num());

    for (int64 LocId : LocationIds)
    {
        if (LocId <= 0 || CheckedLocations.Contains(LocId))
        {
            continue;
        }

        CheckedLocations.Add(LocId);
        JsonLocations.Add(MakeShared<FJsonValueNumber>(static_cast<double>(LocId)));
    }

    if (JsonLocations.Num() == 0)
    {
        UE_LOG(LogArchipelago, Warning, TEXT("Location check skipped: All provided locations have already been checked or contain invalid location IDs."));
        return false;
    }

    TSharedPtr<FJsonObject> PacketObj = MakeShared<FJsonObject>();
    PacketObj->SetStringField(TEXT("cmd"), TEXT("LocationChecks"));
    PacketObj->SetArrayField(TEXT("locations"), JsonLocations);

    TArray<TSharedPtr<FJsonValue>> RootArray;
    RootArray.Add(MakeShared<FJsonValueObject>(PacketObj));

    FString OutputString;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(RootArray, Writer);

    ArchipelagoWebSocket->SendMessage(OutputString);

    UE_LOG(LogArchipelago, Log, TEXT("Sent LocationChecks for %d new location(s)."), JsonLocations.Num());
    return true;
}

bool UArchipelagoSubsystem::RequestLocationScout(int64 LocationId)
{
    return RequestLocationScouts({ LocationId });
}

bool UArchipelagoSubsystem::RequestLocationScouts(const TArray<int64>& LocationIds)
{
    if (!IsValid(ArchipelagoWebSocket))
    {
        UE_LOG(LogArchipelago, Warning, TEXT("Cannot request location scouts: WebSocket doesn't exist."));
        return false;
    }

    if (!ArchipelagoWebSocket->IsConnected())
    {
        UE_LOG(LogArchipelago, Warning, TEXT("Cannot request location scouts: WebSocket is not connected."));
        return false;
    }

    TArray<TSharedPtr<FJsonValue>> JsonLocations;

    for (int64 LocId : LocationIds)
    {
        if (LocId <= 0)
            continue;

        JsonLocations.Add(MakeShared<FJsonValueNumber>(static_cast<double>(LocId)));
    }

    if (JsonLocations.IsEmpty())
    {
        UE_LOG(LogArchipelago, Warning, TEXT("Location scout request skipped: No valid location IDs provided."));
        return false;
    }

    TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();

    JsonObject->SetStringField(TEXT("cmd"), TEXT("LocationScouts"));
    JsonObject->SetArrayField(TEXT("locations"), JsonLocations);
    JsonObject->SetNumberField(TEXT("create_as_hint"), 0);

    TArray<TSharedPtr<FJsonValue>> JsonArray;
    JsonArray.Add(MakeShared<FJsonValueObject>(JsonObject));

    FString OutputString;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);

    if (!FJsonSerializer::Serialize(JsonArray, Writer))
    {
        UE_LOG(LogArchipelago, Error, TEXT("Failed to serialize LocationScouts request."));
        return false;
    }

    ArchipelagoWebSocket->SendMessage(OutputString);

    UE_LOG(LogArchipelago, Log, TEXT("Requested scouts for %d location(s)."), JsonLocations.Num());

    return true;
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
        else if (Command == TEXT("PrintJSON"))
        {
            HandlePrintJSON(JsonObject);
        }
        else if (Command == TEXT("DataPackage"))
        {
            HandleDataPackage(JsonObject);
        }
        else if (Command == TEXT("LocationInfo"))
        {
            HandleLocationInfo(JsonObject);
        }
        else if (Command == TEXT("RoomUpdate"))
        {
            HandleRoomUpdate(JsonObject);
        }
        else
        {
            UE_LOG(LogArchipelago, Verbose, TEXT("Unhandled Archipelago command: %s"), *Command);
        }
    }
}

void UArchipelagoSubsystem::HandleRoomInfo(const TSharedPtr<FJsonObject>& JsonObject)
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
                RoomInfo.DataPackageChecksums.Add(Pair.Key, Pair.Value->AsString());
            }
        }
    }

    UE_LOG(LogArchipelago,Log,TEXT("RoomInfo processed successfully. Games: %d | Tags: %d | Seed: %s"), RoomInfo.Games.Num(), RoomInfo.Tags.Num(), *RoomInfo.SeedName);

    // Send Archipelago authentication
    SendConnectPacket();
}

void UArchipelagoSubsystem::HandleConnected(const TSharedPtr<FJsonObject>& JsonObject)
{
    if (!JsonObject.IsValid())
    {
        UE_LOG(LogArchipelago, Error, TEXT("Invalid Connected JSON object."));
        return;
    }

    UE_LOG(LogArchipelago, Log, TEXT("Processing Connected..."));

    JsonObject->TryGetNumberField(TEXT("team"), ConnectedTeam);
    JsonObject->TryGetNumberField(TEXT("slot"), ConnectedSlot);
    JsonObject->TryGetNumberField(TEXT("hint_points"), HintPoints);

    // Missing locations
    MissingLocations.Empty();

    const TArray<TSharedPtr<FJsonValue>>* MissingArray = nullptr;

    if (JsonObject->TryGetArrayField(TEXT("missing_locations"), MissingArray))
    {
        for (const TSharedPtr<FJsonValue>& Value : *MissingArray)
        {
            MissingLocations.Add(static_cast<int64>(Value->AsNumber()));
        }
    }

    // Checked locations
    CheckedLocations.Empty();

    const TArray<TSharedPtr<FJsonValue>>* CheckedArray = nullptr;

    if (JsonObject->TryGetArrayField(TEXT("checked_locations"), CheckedArray))
    {
        for (const TSharedPtr<FJsonValue>& Value : *CheckedArray)
        {
            CheckedLocations.Add(static_cast<int64>(Value->AsNumber()));
        }
    }

    // Players
    Players.Empty();

    const TArray<TSharedPtr<FJsonValue>>* PlayersArray = nullptr;

    if (JsonObject->TryGetArrayField(TEXT("players"), PlayersArray))
    {
        for (const TSharedPtr<FJsonValue>& Value : *PlayersArray)
        {
            const TSharedPtr<FJsonObject> PlayerObject = Value->AsObject();

            if (!PlayerObject.IsValid())
                continue;

            FAPPlayer Player;

            PlayerObject->TryGetNumberField(TEXT("team"), Player.Team);
            PlayerObject->TryGetNumberField(TEXT("slot"), Player.Slot);
            PlayerObject->TryGetStringField(TEXT("alias"), Player.Alias);
            PlayerObject->TryGetStringField(TEXT("name"), Player.Name);
            PlayerObject->TryGetStringField(TEXT("class"), Player.PlayerClass);

            Players.Add(MoveTemp(Player));
        }
    }

    // Slot Info
    SlotInfo.Empty();

    const TSharedPtr<FJsonObject>* SlotInfoObject = nullptr;

    if (JsonObject->TryGetObjectField(TEXT("slot_info"), SlotInfoObject) && SlotInfoObject && SlotInfoObject->IsValid())
    {
        for (const TPair<FString, TSharedPtr<FJsonValue>>& SlotPair : (*SlotInfoObject)->Values)
        {
            const TSharedPtr<FJsonObject> SlotObject = SlotPair.Value->AsObject();

            if (!SlotObject.IsValid())
                continue;

            const int32 Slot = FCString::Atoi(*SlotPair.Key);

            FAPSlotInfo Info;

            SlotObject->TryGetStringField(TEXT("name"), Info.Name);
            SlotObject->TryGetStringField(TEXT("game"), Info.Game);
            SlotObject->TryGetNumberField(TEXT("type"), Info.Type);

            SlotInfo.Add(Slot, MoveTemp(Info));

            UE_LOG(LogArchipelago, Log,
                TEXT("Slot: %d | Name: %s | Game: %s | Type: %d"),
                Slot,
                *SlotInfo[Slot].Name,
                *SlotInfo[Slot].Game,
                SlotInfo[Slot].Type);
        }
    }

    // Slot Data
    const TSharedPtr<FJsonObject>* SlotDataObject = nullptr;

    if (JsonObject->TryGetObjectField(TEXT("slot_data"), SlotDataObject))
    {
        SlotData = *SlotDataObject;
    }

    UE_LOG(LogArchipelago, Log, TEXT("Connected successfully! Team: %d | Slot: %d | Players: %d"), ConnectedTeam, ConnectedSlot, Players.Num());
    OnConnected.Broadcast();

    // Get DataPackages for all games in this Archipelago session.
    TArray<FString> GameNames;

    for (const TPair<int32, FAPSlotInfo>& SlotPair : SlotInfo)
    {
        if (!SlotPair.Value.Game.IsEmpty())
        {
            GameNames.AddUnique(SlotPair.Value.Game);
        }
    }

    GameNames.AddUnique(TEXT("Archipelago"));

    if (!GameNames.IsEmpty())
    {
        RequestDataPackage(GameNames);
    }
}

void UArchipelagoSubsystem::HandleConnectionRefused(const TSharedPtr<FJsonObject>& JsonObject)
{
    if (!JsonObject.IsValid())
    {
        UE_LOG(LogArchipelago, Error, TEXT("ConnectionRefused received an invalid JSON object."));
        return;
    }

    const TArray<TSharedPtr<FJsonValue>>* ErrorsArray = nullptr;

    if (!JsonObject->TryGetArrayField(TEXT("errors"), ErrorsArray) || !ErrorsArray)
    {
        UE_LOG(LogArchipelago, Error, TEXT("Connection refused without an error description."));
        return;
    }

    for (const TSharedPtr<FJsonValue>& Value : *ErrorsArray)
    {
        if (!Value.IsValid() || Value->Type != EJson::String)
            continue;

        const FString Error = Value->AsString();

        if (Error == TEXT("InvalidSlot"))
        {
            UE_LOG(LogArchipelago, Error, TEXT("Connection refused: Invalid slot! Check your slot name."));
        }
        else if (Error == TEXT("InvalidGame"))
        {
            UE_LOG(LogArchipelago, Error, TEXT("Connection refused: Invalid game! Check your GameName!"));
        }
        else if (Error == TEXT("IncompatibleVersion"))
        {
            UE_LOG(LogArchipelago, Error, TEXT("Connection refused: Incompatible Archipelago version!"));
        }
        else if (Error == TEXT("InvalidPassword"))
        {
            UE_LOG(LogArchipelago, Error, TEXT("Connection refused: Invalid password! Check your password."));
        }
        else if (Error == TEXT("InvalidItemsHandling"))
        {
            UE_LOG(LogArchipelago, Error, TEXT("Connection refused: Invalid items_handling value!"));
        }
        else if (Error == TEXT("SlotAlreadyTaken"))
        {
            UE_LOG(LogArchipelago, Error, TEXT("Connection refused: This slot is already connected by another client."));
        }
        else
        {
            UE_LOG(LogArchipelago, Error, TEXT("Connection refused: Unknown error code: %s"), *Error);
        }
    }
}

void UArchipelagoSubsystem::HandleReceivedItems(const TSharedPtr<FJsonObject>& JsonObject)
{
    if (!JsonObject.IsValid())
    {
        UE_LOG(LogArchipelago, Error, TEXT("Invalid ReceivedItems JSON object."));
        return;
    }

    int32 Index = 0;
    JsonObject->TryGetNumberField(TEXT("index"), Index);

    const TArray<TSharedPtr<FJsonValue>>* ItemsArray = nullptr;

    if (!JsonObject->TryGetArrayField(TEXT("items"), ItemsArray))
    {
        UE_LOG(LogArchipelago, Warning, TEXT("ReceivedItems has no items array."));
        return;
    }

    // Store only the newly received items.
    TArray<FAPNetworkItem> NewItems;

    // Index 0 means this is the initial item history.
    if (Index == 0)
    {
        ReceivedItems.Empty();
    }

    for (const TSharedPtr<FJsonValue>& Value : *ItemsArray)
    {
        const TSharedPtr<FJsonObject> ItemObject = Value->AsObject();

        if (!ItemObject.IsValid())
        {
            continue;
        }

        FAPNetworkItem Item;

        ItemObject->TryGetNumberField(TEXT("item"), Item.Item);
        ItemObject->TryGetNumberField(TEXT("location"), Item.Location);
        ItemObject->TryGetNumberField(TEXT("player"), Item.Player);
        ItemObject->TryGetNumberField(TEXT("flags"), Item.Flags);

        NewItems.Add(Item);
        ReceivedItems.Add(MoveTemp(Item));
    }

    ReceivedItemsIndex = ReceivedItems.Num();

    UE_LOG(LogArchipelago, Log, TEXT("Received %d items. Total item history: %d"), NewItems.Num(), ReceivedItems.Num());

    if (GameData.Find(GameName))
    {
        TArray<FAPResolvedNetworkItem> NewResolvedItems;

        // Resolve only newly received items.
        for (const FAPNetworkItem& Item : NewItems)
        {
            FAPResolvedNetworkItem ResolvedItem = ResolveNetworkItem(Item);

            UE_LOG(LogArchipelago, Log, TEXT("Item: %s | Location: %s | ID: %lld"),
                *ResolvedItem.ItemName,
                *ResolvedItem.LocationName,
                ResolvedItem.ItemID);

            NewResolvedItems.Add(ResolvedItem);
        }

        // Append newly resolved items to the complete history.
        ReceivedItemsResolved.Append(NewResolvedItems);

        // Only broadcast OnReceivedItems after the GameData was locally processed.
        OnReceivedItems.Broadcast(NewResolvedItems);
    }
}

void UArchipelagoSubsystem::HandlePrintJSON(const TSharedPtr<FJsonObject>& JsonObject)
{
    if (!JsonObject.IsValid())
    {
        UE_LOG(LogArchipelago, Warning, TEXT("PrintJSON received an invalid JSON object."));
        return;
    }

    const TArray<TSharedPtr<FJsonValue>>* DataArray = nullptr;

    if (!JsonObject->TryGetArrayField(TEXT("data"), DataArray) || !DataArray)
    {
        UE_LOG(LogArchipelago, Warning, TEXT("PrintJSON is missing its data array."));
        return;
    }

    FAPPrintJSONMessage Message;

    for (const TSharedPtr<FJsonValue>& Value : *DataArray)
    {
        if (!Value.IsValid() || Value->Type != EJson::Object)
        {
            continue;
        }

        const TSharedPtr<FJsonObject> DataObject = Value->AsObject();

        if (!DataObject.IsValid())
        {
            continue;
        }

        FAPPrintJSONSegment Segment;

        // Read the segment type.
        if (!DataObject->TryGetStringField(TEXT("type"), Segment.Type))
        {
            Segment.Type = TEXT("text");
        }

        // Read the actual text.
        if (!DataObject->TryGetStringField(TEXT("text"), Segment.Text))
        {
            continue;
        }

        int32 SegmentPlayer = 0;
        DataObject->TryGetNumberField(TEXT("player"), SegmentPlayer);

        Segment.Color = GetPrintJSONColor(Segment.Type);
        Message.Segments.Add(ParsePrintJSONSegment(Segment, SegmentPlayer));
    }

    if (Message.Segments.IsEmpty())
    {
        UE_LOG(LogArchipelago, Warning, TEXT("PrintJSON contained no valid text segments."));
        return;
    }

    // Construct Message.PlainText from the resolved Segments.
    Message.PlainText.Empty();

    for (const FAPPrintJSONSegment& Segment : Message.Segments)
    {
        Message.PlainText += Segment.Text;
    }

    UE_LOG(LogArchipelago, Log, TEXT("%s"), *Message.PlainText);
    OnPrintJSON.Broadcast(Message);
}

void UArchipelagoSubsystem::HandleDataPackage(const TSharedPtr<FJsonObject>& JsonObject)
{
    if (!JsonObject.IsValid())
    {
        UE_LOG(LogArchipelago, Warning, TEXT("DataPackage received an invalid JSON object."));
        return;
    }

    // Get the data object.
    const TSharedPtr<FJsonObject>* DataObject = nullptr;

    if (!JsonObject->TryGetObjectField(TEXT("data"), DataObject) || !DataObject || !DataObject->IsValid())
    {
        UE_LOG(LogArchipelago, Warning, TEXT("DataPackage is missing its data object."));
        return;
    }

    // Get the games object.
    const TSharedPtr<FJsonObject>* GamesObject = nullptr;

    FString DebugJson;

    const TSharedRef<TJsonWriter<>> Writer =
        TJsonWriterFactory<>::Create(&DebugJson);

    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    UE_LOG(LogArchipelago, Warning,
        TEXT("Received DataPackage JSON:\n%s"),
        *DebugJson);

    if (!(*DataObject)->TryGetObjectField(TEXT("games"), GamesObject) || !GamesObject || !GamesObject->IsValid())
    {
        UE_LOG(LogArchipelago, Warning, TEXT("DataPackage is missing its games object."));
        return;
    }

    // Process every game.
    for (const TPair<FString, TSharedPtr<FJsonValue>>& GamePair : (*GamesObject)->Values)
    {
        const FString& Name = GamePair.Key;
        const TSharedPtr<FJsonObject> GameObject = GamePair.Value->AsObject();

        if (!GameObject.IsValid())
        {
            continue;
        }

        FAPGameData& CurrentGameData = GameData.FindOrAdd(Name);

        // ------------------------------------------------
        // Parse Item Names
        // ------------------------------------------------

        const TSharedPtr<FJsonObject>* ItemsObject = nullptr;

        if (GameObject->TryGetObjectField(TEXT("item_name_to_id"), ItemsObject) && ItemsObject && ItemsObject->IsValid())
        {
            CurrentGameData.ItemNames.Empty();

            for (const TPair<FString, TSharedPtr<FJsonValue>>& ItemPair : (*ItemsObject)->Values)
            {
                const int64 ItemID = static_cast<int64>(ItemPair.Value->AsNumber());

                CurrentGameData.ItemNames.Add(ItemID, ItemPair.Key);
            }
        }

        // ------------------------------------------------
        // Parse Location Names
        // ------------------------------------------------

        const TSharedPtr<FJsonObject>* LocationsObject = nullptr;

        if (GameObject->TryGetObjectField(TEXT("location_name_to_id"), LocationsObject) && LocationsObject && LocationsObject->IsValid())
        {
            CurrentGameData.LocationNames.Empty();

            for (const TPair<FString, TSharedPtr<FJsonValue>>& LocationPair : (*LocationsObject)->Values)
            {
                const int64 LocationID = static_cast<int64>(LocationPair.Value->AsNumber());

                CurrentGameData.LocationNames.Add(LocationID, LocationPair.Key);
            }
        }

        UE_LOG(LogArchipelago, Log, TEXT("DataPackage processed for %s | Items: %d | Locations: %d"), *Name, CurrentGameData.ItemNames.Num(), CurrentGameData.LocationNames.Num());
    }

    UE_LOG(LogArchipelago, Warning,
        TEXT("DataPackage contains %d games."),
        (*GamesObject)->Values.Num());

    UE_LOG(LogArchipelago, Log, TEXT("DataPackage processing completed. Games: %d"), GameData.Num());
    RequestSync();
}

void UArchipelagoSubsystem::HandleLocationInfo(const TSharedPtr<FJsonObject>& JsonObject)
{
    if (!JsonObject.IsValid())
    {
        UE_LOG(LogArchipelago, Error, TEXT("LocationInfo received an invalid JSON object."));
        return;
    }

    const TArray<TSharedPtr<FJsonValue>>* LocationsArray = nullptr;

    if (!JsonObject->TryGetArrayField(TEXT("locations"), LocationsArray) || !LocationsArray)
    {
        UE_LOG(LogArchipelago, Warning, TEXT("LocationInfo packet contains no locations array."));
        return;
    }

    TArray<FAPResolvedNetworkItem> ResolvedLocations;

    for (const TSharedPtr<FJsonValue>& Value : *LocationsArray)
    {
        if (!Value.IsValid() || Value->Type != EJson::Object)
            continue;

        const TSharedPtr<FJsonObject> LocationObject = Value->AsObject();

        if (!LocationObject.IsValid())
            continue;

        double ItemValue = 0.0;
        double LocationValue = 0.0;
        double PlayerValue = 0.0;
        double FlagsValue = 0.0;

        if (!LocationObject->TryGetNumberField(TEXT("item"), ItemValue) ||
            !LocationObject->TryGetNumberField(TEXT("location"), LocationValue) ||
            !LocationObject->TryGetNumberField(TEXT("player"), PlayerValue) ||
            !LocationObject->TryGetNumberField(TEXT("flags"), FlagsValue))
        {
            UE_LOG(LogArchipelago, Warning, TEXT("LocationInfo contains an entry with missing or invalid fields."));
            continue;
        }

        const int64 ItemID = static_cast<int64>(ItemValue);
        const int64 LocationID = static_cast<int64>(LocationValue);
        const int32 PlayerSlot = static_cast<int32>(PlayerValue);
        const int32 Flags = static_cast<int32>(FlagsValue);

        FAPResolvedNetworkItem Result;

        Result.ItemID = ItemID;
        Result.LocationID = LocationID;
        Result.PlayerSlot = PlayerSlot;

        // Resolve the item name using the recipient's game.
        FString RecipientGame;

        if (PlayerSlot == 0)
        {
            RecipientGame = TEXT("Archipelago");
            Result.PlayerName = TEXT("Archipelago Server");
            Result.bPlayerNameFound = true;
        }
        else
        {
            if (const FAPSlotInfo* RecipientSlotInfo = SlotInfo.Find(PlayerSlot))
            {
                RecipientGame = RecipientSlotInfo->Game;
            }

            for (const FAPPlayer& Player : Players)
            {
                if (Player.Slot == PlayerSlot)
                {
                    Result.PlayerName = !Player.Alias.IsEmpty() ? Player.Alias : Player.Name;
                    Result.bPlayerNameFound = !Result.PlayerName.IsEmpty();
                    break;
                }
            }
        }

        if (!RecipientGame.IsEmpty())
        {
            if (const FAPGameData* RecipientGameData = GameData.Find(RecipientGame))
            {
                if (const FString* ItemName = RecipientGameData->ItemNames.Find(ItemID))
                {
                    Result.ItemName = *ItemName;
                    Result.bItemNameFound = true;
                }
            }
        }

        // The scouted location belongs to the connected player's game.
        if (const FAPGameData* LocalGameData = GameData.Find(GameName))
        {
            if (const FString* LocationName = LocalGameData->LocationNames.Find(LocationID))
            {
                Result.LocationName = *LocationName;
                Result.bLocationNameFound = true;
            }
        }

        // Decode item classification flags.
        if (Flags & 1)
            Result.Classifications.Add(EAPItemClassification::Advancement);

        if (Flags & 2)
            Result.Classifications.Add(EAPItemClassification::NeverExclude);

        if (Flags & 4)
            Result.Classifications.Add(EAPItemClassification::Useful);

        if (Flags & 8)
            Result.Classifications.Add(EAPItemClassification::Trap);

        ResolvedLocations.Add(MoveTemp(Result));
    }

    if (ResolvedLocations.IsEmpty())
    {
        UE_LOG(LogArchipelago, Warning, TEXT("LocationInfo contained no valid location entries."));
        return;
    }

    UE_LOG(LogArchipelago, Log, TEXT("Received scout information for %d location(s)."), ResolvedLocations.Num());
    OnReceivedLocationInfo.Broadcast(ResolvedLocations);
}

void UArchipelagoSubsystem::HandleRoomUpdate(const TSharedPtr<FJsonObject>& JsonObject)
{
    if (!JsonObject.IsValid())
    {
        UE_LOG(LogArchipelago, Error, TEXT("Invalid RoomUpdate JSON object."));
        return;
    }

    UE_LOG(LogArchipelago, Log, TEXT("Processing RoomUpdate..."));

    // Hint Points
    JsonObject->TryGetNumberField(TEXT("hint_points"), HintPoints);

    // Checked Locations
    const TArray<TSharedPtr<FJsonValue>>* CheckedArray = nullptr;

    if (JsonObject->TryGetArrayField(TEXT("checked_locations"), CheckedArray) && CheckedArray)
    {
        for (const TSharedPtr<FJsonValue>& Value : *CheckedArray)
        {
            if (!Value.IsValid() || Value->Type != EJson::Number)
                continue;

            const int64 LocationID = static_cast<int64>(Value->AsNumber());

            if (LocationID <= 0)
                continue;

            CheckedLocations.Add(LocationID);
            MissingLocations.Remove(LocationID);
        }
    }

    // Players
    const TArray<TSharedPtr<FJsonValue>>* PlayersArray = nullptr;

    if (JsonObject->TryGetArrayField(TEXT("players"), PlayersArray) && PlayersArray)
    {
        Players.Empty();

        for (const TSharedPtr<FJsonValue>& Value : *PlayersArray)
        {
            if (!Value.IsValid() || Value->Type != EJson::Object)
                continue;

            const TSharedPtr<FJsonObject> PlayerObject = Value->AsObject();

            if (!PlayerObject.IsValid())
                continue;

            FAPPlayer Player;

            PlayerObject->TryGetNumberField(TEXT("team"), Player.Team);
            PlayerObject->TryGetNumberField(TEXT("slot"), Player.Slot);
            PlayerObject->TryGetStringField(TEXT("alias"), Player.Alias);
            PlayerObject->TryGetStringField(TEXT("name"), Player.Name);
            PlayerObject->TryGetStringField(TEXT("class"), Player.PlayerClass);

            Players.Add(MoveTemp(Player));
        }
    }

    UE_LOG(LogArchipelago, Log, TEXT("RoomUpdate processed. Hint Points: %d | Players: %d | Checked Locations: %d"), HintPoints, Players.Num(), CheckedLocations.Num());
    
    FAPRoomUpdate Update;
    Update.HintPoints = HintPoints;

    for (const int64 LocationID : CheckedLocations)
    {
        FAPCheckedLocation Location;
        Location.ID = LocationID;

        if (const FAPGameData* LocalGameData = GameData.Find(GameName))
        {
            if (const FString* LocationName = LocalGameData->LocationNames.Find(LocationID))
            {
                Location.Name = *LocationName;
            }
        }

        Update.CheckedLocations.Add(MoveTemp(Location));
    }

    OnRoomUpdated.Broadcast(Update);
}

void UArchipelagoSubsystem::ParseVersion(const TSharedPtr<FJsonObject>& JsonObject, FAPVersion& OutVersion)
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

FAPPrintJSONSegment UArchipelagoSubsystem::ParsePrintJSONSegment(const FAPPrintJSONSegment Segment, int32 PlayerID)
{
    FAPPrintJSONSegment Out = Segment;

    auto GetTargetGameData = [this, PlayerID]() -> const FAPGameData*
        {
            if (PlayerID == 0)
            {
                return GameData.Find(TEXT("Archipelago"));
            }

            if (const FAPPlayer* Owner = Players.FindByPredicate([PlayerID](const FAPPlayer& P) { return P.Slot == PlayerID; }))
            {
                if (!Owner->PlayerClass.IsEmpty())
                {
                    if (const FAPGameData* OwnerGame = GameData.Find(Owner->PlayerClass))
                    {
                        return OwnerGame;
                    }
                }
            }

            return GameData.Find(GameName);
        };

    if (Out.Type == TEXT("player_id"))
    {
        int32 Slot = 0;
        if (LexTryParseString(Slot, *Out.Text))
        {
            if (Slot == 0)
            {
                Out.Text = TEXT("Archipelago Server");
            }
            else if (const FAPPlayer* Player = Players.FindByPredicate([Slot](const FAPPlayer& P) { return P.Slot == Slot; }))
            {
                Out.Text = !Player->Alias.IsEmpty() ? Player->Alias : Player->Name;
            }
        }
    }
    else if (Out.Type == TEXT("item_id"))
    {
        int64 ItemId = 0;
        if (LexTryParseString(ItemId, *Out.Text))
        {
            if (const FAPGameData* TargetGame = GetTargetGameData())
            {
                if (const FString* ItemName = TargetGame->ItemNames.Find(ItemId))
                {
                    Out.Text = *ItemName;
                }
            }
        }
    }
    else if (Out.Type == TEXT("location_id"))
    {
        int64 LocationId = 0;
        if (LexTryParseString(LocationId, *Out.Text))
        {
            if (const FAPGameData* TargetGame = GetTargetGameData())
            {
                if (const FString* LocationName = TargetGame->LocationNames.Find(LocationId))
                {
                    Out.Text = *LocationName;
                }
            }
        }
    }

    return Out;
}

void UArchipelagoSubsystem::SendConnectPacket()
{
    if (!IsValid(ArchipelagoWebSocket))
    {
        UE_LOG(LogArchipelago, Error, TEXT("Cannot send Connect packet: WebSocket is invalid."));
        return;
    }

    // Create Connect object
    TSharedPtr<FJsonObject> ConnectObject = MakeShared<FJsonObject>();

    ConnectObject->SetStringField(TEXT("cmd"), TEXT("Connect"));

    // Password
    if (Config.Password.IsEmpty())
    {
        ConnectObject->SetField(TEXT("password"), MakeShared<FJsonValueNull>());
    }
    else
    {
        ConnectObject->SetStringField(TEXT("password"), Config.Password);
    }

    // Player information
    ConnectObject->SetStringField(TEXT("game"), GameName);
    ConnectObject->SetStringField(TEXT("name"), Config.Slot);
    ConnectObject->SetStringField(TEXT("uuid"), UUID);

    // Client protocol version
    TSharedPtr<FJsonObject> VersionObject = MakeShared<FJsonObject>();

    VersionObject->SetNumberField(TEXT("major"), 0);
    VersionObject->SetNumberField(TEXT("minor"), 6);
    VersionObject->SetNumberField(TEXT("build"), 8);
    VersionObject->SetStringField(TEXT("class"), TEXT("Version"));

    ConnectObject->SetObjectField(TEXT("version"), VersionObject);

    // Item handling
    ConnectObject->SetNumberField(TEXT("items_handling"), 7);

    // Tags
    TArray<TSharedPtr<FJsonValue>> Tags;
    ConnectObject->SetArrayField(TEXT("tags"), Tags);

    // Request slot data
    ConnectObject->SetBoolField(TEXT("slot_data"), true);

    // Archipelago expects an array of commands.
    TArray<TSharedPtr<FJsonValue>> RootArray;
    RootArray.Add(MakeShared<FJsonValueObject>(ConnectObject));

    // Serialize JSON
    FString SerializedMessage;

    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&SerializedMessage);

    if (!FJsonSerializer::Serialize(RootArray, Writer))
    {
        UE_LOG(LogArchipelago, Error, TEXT("Failed to serialize Connect packet."));
        return;
    }

    // Send message
    ArchipelagoWebSocket->SendMessage(SerializedMessage);

    UE_LOG(LogArchipelago, Log, TEXT("Archipelago Connect packet sent."));
}

void UArchipelagoSubsystem::RequestDataPackage(const TArray<FString>& GameNames)
{
    TSharedPtr<FJsonObject> RequestObject = MakeShared<FJsonObject>();

    RequestObject->SetStringField(TEXT("cmd"), TEXT("GetDataPackage"));

    TArray<TSharedPtr<FJsonValue>> GamesArray;

    for (const FString& Game : GameNames)
    {
        GamesArray.Add(MakeShared<FJsonValueString>(Game));
    }

    RequestObject->SetArrayField(TEXT("games"), GamesArray);

    TArray<TSharedPtr<FJsonValue>> Packets;
    Packets.Add(MakeShared<FJsonValueObject>(RequestObject));

    FString Output;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);

    FJsonSerializer::Serialize(Packets, Writer);

    ArchipelagoWebSocket->SendMessage(Output);
}

void UArchipelagoSubsystem::RequestSync()
{
    TSharedPtr<FJsonObject> SyncObject = MakeShared<FJsonObject>();

    SyncObject->SetStringField(TEXT("cmd"), TEXT("Sync"));

    TArray<TSharedPtr<FJsonValue>> PacketArray;

    PacketArray.Add(MakeShared<FJsonValueObject>(SyncObject));

    FString JsonString;

    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);

    FJsonSerializer::Serialize(PacketArray, Writer);

    // Send through WebSocket
    ArchipelagoWebSocket->SendMessage(JsonString);
}

bool UArchipelagoSubsystem::SendChatMessage(const FString& Message)
{
    if (!IsValid(ArchipelagoWebSocket))
    {
        UE_LOG(LogArchipelago, Warning, TEXT("Cannot send chat message: WebSocket doesn't exist."));
        return false;
    }

    if (!ArchipelagoWebSocket->IsConnected())
    {
        UE_LOG(LogArchipelago, Warning, TEXT("Cannot send chat message: WebSocket is not connected."));
        return false;
    }

    if (Message.TrimStartAndEnd().IsEmpty())
    {
        UE_LOG(LogArchipelago, Warning, TEXT("Cannot send chat message: Message is empty."));
        return false;
    }

    TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();

    JsonObject->SetStringField(TEXT("cmd"), TEXT("Say"));
    JsonObject->SetStringField(TEXT("text"), Message);

    TArray<TSharedPtr<FJsonValue>> JsonArray;
    JsonArray.Add(MakeShared<FJsonValueObject>(JsonObject));

    FString OutputString;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);

    if (!FJsonSerializer::Serialize(JsonArray, Writer))
    {
        UE_LOG(LogArchipelago, Error, TEXT("Failed to serialize Say request."));
        return false;
    }

    ArchipelagoWebSocket->SendMessage(OutputString);

    UE_LOG(LogArchipelago, Log, TEXT("Sent chat message: %s"), *Message);
    return true;
}

FAPResolvedNetworkItem UArchipelagoSubsystem::ResolveNetworkItem(const FAPNetworkItem& NetworkItem) const
{
    FAPResolvedNetworkItem Result;

    Result.ItemID = NetworkItem.Item;
    Result.LocationID = NetworkItem.Location;
    Result.PlayerSlot = NetworkItem.Player;

    // Resolve the correct game's DataPackage.
    const FAPGameData* LocationGameData = nullptr;

    if (NetworkItem.Player == 0)
    {
        // Player 0 represents the Archipelago Server.
        LocationGameData = GameData.Find(TEXT("Archipelago"));

        Result.PlayerName = TEXT("Archipelago Server");
        Result.bPlayerNameFound = true;
    }
    else
    {
        // Find the player who originally sent/owns the item.
        const FAPPlayer* LocationOwner = Players.FindByPredicate(
            [&NetworkItem](const FAPPlayer& Player)
            {
                return Player.Slot == NetworkItem.Player;
            }
        );

        if (LocationOwner)
        {
            // Resolve the sender's game through SlotInfo.
            if (const FAPSlotInfo* OwnerSlotInfo = SlotInfo.Find(LocationOwner->Slot))
            {
                LocationGameData = GameData.Find(OwnerSlotInfo->Game);
            }

            // Resolve player name.
            Result.PlayerName = !LocationOwner->Alias.IsEmpty() ? LocationOwner->Alias : LocationOwner->Name;
            Result.bPlayerNameFound = true;
        }
    }

    // Resolve item name using the receiving player's own game DataPackage.
    if (const FAPGameData* ReceivingGameData = GameData.Find(GameName))
    {
        if (const FString* ItemName = ReceivingGameData->ItemNames.Find(NetworkItem.Item))
        {
            Result.ItemName = *ItemName;
            Result.bItemNameFound = true;
        }
    }

    // Resolve location name using the sending slot's game DataPackage.
    if (LocationGameData)
    {
        if (const FString* LocationName = LocationGameData->LocationNames.Find(NetworkItem.Location))
        {
            Result.LocationName = *LocationName;
            Result.bLocationNameFound = true;
        }
    }

    // Resolve item and location names.
    /*
    if (LocationGameData)
    {
        if (const FString* ItemName =
            LocationGameData->ItemNames.Find(NetworkItem.Item))
        {
            Result.ItemName = *ItemName;
            Result.bItemNameFound = true;
        }
    }
    */

    // Decode item classifications.
    if ((NetworkItem.Flags & 1) != 0)
        Result.Classifications.Add(EAPItemClassification::Advancement);

    if ((NetworkItem.Flags & 2) != 0)
        Result.Classifications.Add(EAPItemClassification::NeverExclude);

    if ((NetworkItem.Flags & 4) != 0)
        Result.Classifications.Add(EAPItemClassification::Useful);

    if ((NetworkItem.Flags & 8) != 0)
        Result.Classifications.Add(EAPItemClassification::Trap);

    return Result;
}

