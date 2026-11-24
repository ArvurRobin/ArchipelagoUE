// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

#pragma once

#pragma region Config

UENUM(BlueprintType, DisplayName = "Config Load Result", Category = "Archipelago|Config")
enum class EAPConfigLoadResult : uint8
{
    Success                 UMETA(DisplayName = "Success"),
    FileNotFound            UMETA(DisplayName = "File Not Found"),
    FileReadFailed          UMETA(DisplayName = "File Read Failed"),
    JsonParsingFailed       UMETA(DisplayName = "JSON Parsing Failed"),
    JsonConversionFailed    UMETA(DisplayName = "JSON Conversion Failed")
};

UENUM(BlueprintType, DisplayName = "Config Create Result", Category = "Archipelago|Config")
enum class EAPConfigCreateResult : uint8
{
    Success                 UMETA(DisplayName = "Success"),
    AlreadyExists           UMETA(DisplayName = "Already Exists"),
    DirectoryCreationFailed UMETA(DisplayName = "Directory Creation Failed"),
    JsonConversionFailed    UMETA(DisplayName = "JSON Conversion Failed"),
    FileWriteFailed         UMETA(DisplayName = "File Write Failed")
};

UENUM(BlueprintType, DisplayName = "Config Save Result", Category = "Archipelago|Config")
enum class EAPConfigSaveResult : uint8
{
    Success                 UMETA(DisplayName = "Success"),
    DirectoryCreationFailed UMETA(DisplayName = "Directory Creation Failed"),
    JsonConversionFailed    UMETA(DisplayName = "JSON Conversion Failed"),
    FileWriteFailed         UMETA(DisplayName = "File Write Failed")
};

#pragma endregion

#pragma region WebSocket Connection

UENUM(BlueprintType, DisplayName = "WebSocket Initialization Result", Category = "Archipelago|WebSocket")
enum class EAPWebSocketInitResult : uint8
{
    Success                 UMETA(DisplayName = "Success"),
    AlreadyInitialized      UMETA(DisplayName = "Already Initialized"),
    InvalidURL              UMETA(DisplayName = "Invalid URL"),
    InvalidPort             UMETA(DisplayName = "Invalid Port"),
    CreationFailed          UMETA(DisplayName = "Creation Failed"),
    SubsystemNotAccessible  UMETA(DisplayName = "Subsystem Not Accessible")
};

UENUM(BlueprintType, DisplayName = "WebSocket Connection Result", Category = "Archipelago|WebSocket")
enum class EAPWebSocketConnectionResult : uint8
{
    Success                 UMETA(DisplayName = "Success"),
    ConnectionStarted       UMETA(DisplayName = "Connection Started"),
    AlreadyConnected        UMETA(DisplayName = "Already Connected"),
    NoNetworkConnection     UMETA(DisplayName = "No Network Connection"),
    WebSocketNotFound       UMETA(DisplayName = "WebSocket Not Found"),
    WebSocketNotInitialized UMETA(DisplayName = "WebSocket Not Initialized"),
    HostNotFound            UMETA(DisplayName = "Host Not Found"),
    ConnectionRefused       UMETA(DisplayName = "Connection Refused"),
    ConnectionTimedOut      UMETA(DisplayName = "Connection Timed Out"),
    HandshakeFailed         UMETA(DisplayName = "Handshake Failed"),
    SSLCertificateFailed    UMETA(DisplayName = "SSL Certificate Failed"),
    ConnectionFailed        UMETA(DisplayName = "Connection Failed"),
    Cancelled               UMETA(DisplayName = "Cancelled"),
    SubsystemNotAccessible  UMETA(DisplayName = "Subsystem Not Accessible")
};

UENUM(BlueprintType, DisplayName = "WebSocket Initialization Result", Category = "Archipelago|WebSocket")
enum class EAPWebSocketDestroyResult : uint8
{
    Success                 UMETA(DisplayName = "Success"),
    SubsystemNotAccessible  UMETA(DisplayName = "Subsystem Not Accessible"),
    WebSocketNotFound       UMETA(DisplayName = "WebSocket Not Found"),
    WebSocketNotDestroyed   UMETA(DisplayName = "WebSocket Not Destroyed")
};

#pragma endregion

#pragma region Archipelago

/**
 * Archipelago permission levels.
 * Values correspond directly to the protocol bitmask.
 */
UENUM(BlueprintType)
enum class EAPPermission : uint8
{
    Disabled = 0            UMETA(DisplayName = "Disabled"),
    Goal = 1                UMETA(DisplayName = "Goal"),
    Auto = 2                UMETA(DisplayName = "Auto"),
    AutoAndGoal = 3         UMETA(DisplayName = "Auto + Goal"),
    BeforeGoal = 4          UMETA(DisplayName = "Before Goal"),
    BeforeGoalAndGoal = 5   UMETA(DisplayName = "Before Goal + Goal"),
    BeforeGoalAndAuto = 6   UMETA(DisplayName = "Before Goal + Auto"),
    All = 7                 UMETA(DisplayName = "All")
};

#pragma endregion