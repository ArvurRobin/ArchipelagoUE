// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ArchipelagoStructs.h"
#include "ArchipelagoLog.h"
#include "ArchipelagoSubsystem.generated.h"

UCLASS()
class ARCHIPELAGOUE_API UArchipelagoSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Get the configuration file path
    UFUNCTION(BlueprintPure, Category = "Archipelago|Config", meta = (DisplayName = "Get Config Path"))
    FString GetConfigPath() const;

    // Access the loaded configuration
    UFUNCTION(BlueprintPure, Category = "Archipelago|Config", meta = (DisplayName = "Get Archipelago Config"))
    const FAPConfig& GetConfig() const
    {
        return Config;
    }

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Config", meta = (DisplayName = "Create New Config"))
    EAPConfigCreateResult CreateNewConfig(bool bOverrideExisting = false);

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Config", meta = (DisplayName = "Load Config"))
    EAPConfigLoadResult LoadConfig();

    UFUNCTION(BlueprintCallable, Category = "Archipelago|Config", meta = (DisplayName = "Save Config"))
    EAPConfigSaveResult SaveConfig();

private:

    FAPConfig Config;

    static FString EncodePassword(const FString& Password);
    static FString DecodePassword(const FString& EncodedPassword);
};

