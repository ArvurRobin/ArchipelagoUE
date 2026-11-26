// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

#pragma once

#pragma region Config

UENUM(BlueprintType, meta = (
    DisplayName = "Config Load Result",
    ToolTip = "Describes the result of attempting to load an Archipelago configuration file.",
    Category = "Archipelago|Config"
    ))
    enum class EAPConfigLoadResult : uint8
{
    Success                 UMETA(DisplayName = "Success", ToolTip = "The configuration file was loaded successfully."),
    FileNotFound            UMETA(DisplayName = "File Not Found", ToolTip = "The configuration file could not be found."),
    FileReadFailed          UMETA(DisplayName = "File Read Failed", ToolTip = "The configuration file could not be read."),
    JsonParsingFailed       UMETA(DisplayName = "JSON Parsing Failed", ToolTip = "The configuration file could not be parsed as valid JSON."),
    JsonConversionFailed    UMETA(DisplayName = "JSON Conversion Failed", ToolTip = "The parsed JSON data could not be converted into the configuration structure.")
};

UENUM(BlueprintType, meta = (
    DisplayName = "Config Create Result",
    ToolTip = "Describes the result of attempting to create an Archipelago configuration file.",
    Category = "Archipelago|Config"
    ))
    enum class EAPConfigCreateResult : uint8
{
    Success                 UMETA(DisplayName = "Success", ToolTip = "The configuration file was created successfully."),
    AlreadyExists           UMETA(DisplayName = "Already Exists", ToolTip = "A configuration file already exists at the target path."),
    DirectoryCreationFailed UMETA(DisplayName = "Directory Creation Failed", ToolTip = "The directory for the configuration file could not be created."),
    JsonConversionFailed    UMETA(DisplayName = "JSON Conversion Failed", ToolTip = "The configuration data could not be converted into JSON."),
    FileWriteFailed         UMETA(DisplayName = "File Write Failed", ToolTip = "The configuration file could not be written to disk.")
};

UENUM(BlueprintType, meta = (
    DisplayName = "Config Save Result",
    ToolTip = "Describes the result of attempting to save an Archipelago configuration file.",
    Category = "Archipelago|Config"
    ))
    enum class EAPConfigSaveResult : uint8
{
    Success                 UMETA(DisplayName = "Success", ToolTip = "The configuration file was saved successfully."),
    DirectoryCreationFailed UMETA(DisplayName = "Directory Creation Failed", ToolTip = "The directory for the configuration file could not be created."),
    JsonConversionFailed    UMETA(DisplayName = "JSON Conversion Failed", ToolTip = "The configuration data could not be converted into JSON."),
    FileWriteFailed         UMETA(DisplayName = "File Write Failed", ToolTip = "The configuration file could not be written to disk.")
};

#pragma endregion

#pragma region WebSocket Connection

UENUM(BlueprintType, meta = (
    DisplayName = "WebSocket Initialization Result",
    ToolTip = "Describes the result of attempting to initialize a WebSocket.",
    Category = "Archipelago|WebSocket"
    ))
    enum class EAPWebSocketInitResult : uint8
{
    Success                 UMETA(DisplayName = "Success", ToolTip = "The WebSocket was initialized successfully."),
    AlreadyInitialized      UMETA(DisplayName = "Already Initialized", ToolTip = "The WebSocket has already been initialized."),
    InvalidURL              UMETA(DisplayName = "Invalid URL", ToolTip = "The supplied server URL is invalid."),
    InvalidPort             UMETA(DisplayName = "Invalid Port", ToolTip = "The supplied server port is outside the valid range."),
    CreationFailed          UMETA(DisplayName = "Creation Failed", ToolTip = "The WebSocket could not be created."),
    SubsystemNotAccessible  UMETA(DisplayName = "Subsystem Not Accessible", ToolTip = "The WebSocket subsystem could not be accessed.")
};

UENUM(BlueprintType, meta = (
    DisplayName = "WebSocket Connection Result",
    ToolTip = "Describes the result of attempting to establish a WebSocket connection.",
    Category = "Archipelago|WebSocket"
    ))
    enum class EAPWebSocketConnectionResult : uint8
{
    Success                 UMETA(DisplayName = "Success", ToolTip = "The WebSocket connection is established."),
    ConnectionStarted       UMETA(DisplayName = "Connection Started", ToolTip = "The WebSocket connection attempt has started."),
    AlreadyConnected        UMETA(DisplayName = "Already Connected", ToolTip = "The WebSocket is already connected."),
    NoNetworkConnection     UMETA(DisplayName = "No Network Connection", ToolTip = "No network connection is available."),
    WebSocketNotFound       UMETA(DisplayName = "WebSocket Not Found", ToolTip = "The requested WebSocket could not be found."),
    WebSocketNotInitialized UMETA(DisplayName = "WebSocket Not Initialized", ToolTip = "The WebSocket has not been initialized."),
    HostNotFound            UMETA(DisplayName = "Host Not Found", ToolTip = "The server host could not be resolved."),
    ConnectionRefused       UMETA(DisplayName = "Connection Refused", ToolTip = "The server refused the connection."),
    ConnectionTimedOut      UMETA(DisplayName = "Connection Timed Out", ToolTip = "The connection attempt timed out."),
    HandshakeFailed         UMETA(DisplayName = "Handshake Failed", ToolTip = "The WebSocket handshake failed."),
    SSLCertificateFailed    UMETA(DisplayName = "SSL Certificate Failed", ToolTip = "The SSL certificate could not be validated."),
    ConnectionFailed        UMETA(DisplayName = "Connection Failed", ToolTip = "The connection attempt failed for an unspecified reason."),
    Cancelled               UMETA(DisplayName = "Cancelled", ToolTip = "The connection attempt was cancelled."),
    SubsystemNotAccessible  UMETA(DisplayName = "Subsystem Not Accessible", ToolTip = "The WebSocket subsystem could not be accessed.")
};

UENUM(BlueprintType, meta = (
    DisplayName = "WebSocket Destroy Result",
    ToolTip = "Describes the result of attempting to destroy a WebSocket.",
    Category = "Archipelago|WebSocket"
    ))
    enum class EAPWebSocketDestroyResult : uint8
{
    Success                 UMETA(DisplayName = "Success", ToolTip = "The WebSocket was destroyed successfully."),
    SubsystemNotAccessible  UMETA(DisplayName = "Subsystem Not Accessible", ToolTip = "The WebSocket subsystem could not be accessed."),
    WebSocketNotFound       UMETA(DisplayName = "WebSocket Not Found", ToolTip = "The requested WebSocket could not be found."),
    WebSocketNotDestroyed   UMETA(DisplayName = "WebSocket Not Destroyed", ToolTip = "The WebSocket could not be destroyed.")
};

#pragma endregion

#pragma region Archipelago

UENUM(BlueprintType, meta = (
    ToolTip = "Defines the permission levels used by Archipelago rooms. Values correspond directly to the protocol permission bitmask.",
    Category = "Archipelago|Protocol"
    ))
    enum class EAPPermission : uint8
{
    Disabled = 0            UMETA(DisplayName = "Disabled", ToolTip = "The action is disabled."),
    Goal = 1                UMETA(DisplayName = "Goal", ToolTip = "The action is permitted once the player reaches their goal."),
    Auto = 2                UMETA(DisplayName = "Auto", ToolTip = "The action is permitted automatically."),
    AutoAndGoal = 3         UMETA(DisplayName = "Auto + Goal", ToolTip = "The action is permitted automatically and after reaching the goal."),
    BeforeGoal = 4          UMETA(DisplayName = "Before Goal", ToolTip = "The action is permitted before reaching the goal."),
    BeforeGoalAndGoal = 5   UMETA(DisplayName = "Before Goal + Goal", ToolTip = "The action is permitted both before and after reaching the goal."),
    BeforeGoalAndAuto = 6   UMETA(DisplayName = "Before Goal + Auto", ToolTip = "The action is permitted before the goal and automatically."),
    All = 7                 UMETA(DisplayName = "All", ToolTip = "The action is permitted under all conditions.")
};

UENUM(BlueprintType, meta = (
    ToolTip = "Defines the classifications that can be assigned to Archipelago items.",
    Category = "Archipelago|Protocol"
    ))
    enum class EAPItemClassification : uint8
{
    Advancement             UMETA(DisplayName = "Advancement", ToolTip = "An item required for progression or completion."),
    NeverExclude            UMETA(DisplayName = "Never Exclude", ToolTip = "An item that should never be excluded from relevant item pools."),
    Useful                  UMETA(DisplayName = "Useful", ToolTip = "An item that provides a useful benefit but is not required for progression."),
    Trap                    UMETA(DisplayName = "Trap", ToolTip = "An item that has a negative or disruptive effect.")
};

UENUM(BlueprintType, meta = (
    ToolTip = "Represents the client's current gameplay status reported to the Archipelago server.",
    Category = "Archipelago|Protocol"
    ))
    enum class EAPClientStatus : uint8
{
    Unknown = 0             UMETA(DisplayName = "Unknown", ToolTip = "The client's current status is unknown."),
    Ready = 10              UMETA(DisplayName = "Ready", ToolTip = "The player is ready to begin playing."),
    Playing = 20            UMETA(DisplayName = "Playing", ToolTip = "The player is currently playing."),
    Goal = 30               UMETA(DisplayName = "Goal", ToolTip = "The player has completed their goal.")
};

#pragma endregion
