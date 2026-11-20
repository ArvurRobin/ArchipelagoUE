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
