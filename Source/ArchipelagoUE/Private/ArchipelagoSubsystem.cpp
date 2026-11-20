// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

#include "ArchipelagoSubsystem.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/Base64.h"
#include "JsonObjectConverter.h"

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
}

void UArchipelagoSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

FString UArchipelagoSubsystem::GetConfigPath() const
{
    return FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("APConfig.json"));
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
