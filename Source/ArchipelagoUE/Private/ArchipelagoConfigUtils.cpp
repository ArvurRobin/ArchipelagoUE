// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

#include "ArchipelagoConfigUtils.h"
#include "ArchipelagoStructs.h"
#include "Dom/JsonObject.h"
#include "JsonObjectConverter.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

FString FArchipelagoConfigUtils::GetConfigPath()
{
    return FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("APConfig.json"));
}

EAPConfigCreateResult FArchipelagoConfigUtils::CreateConfigFile(bool bOverrideExisting)
{
    const FString ConfigFilePath = GetConfigPath();
    const FString ConfigDirectory = FPaths::GetPath(ConfigFilePath);

    if (FPaths::FileExists(ConfigFilePath) && !bOverrideExisting)
    {
        return EAPConfigCreateResult::AlreadyExists;
    }

    if (!IFileManager::Get().DirectoryExists(*ConfigDirectory))
    {
        if (!IFileManager::Get().MakeDirectory(*ConfigDirectory, true))
        {
            return EAPConfigCreateResult::DirectoryCreationFailed;
        }
    }

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

    if (!FFileHelper::SaveStringToFile(JsonString, *ConfigFilePath))
    {
        return EAPConfigCreateResult::FileWriteFailed;
    }

    return EAPConfigCreateResult::Success;
}