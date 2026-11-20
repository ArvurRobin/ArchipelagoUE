// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

#pragma once

#include "ArchipelagoEnums.h"
#include "ArchipelagoStructs.generated.h"

#pragma region Config

USTRUCT(BlueprintType, Category = "Archipelago|Config", meta = (DisplayName = "Archipelago Config"))
struct FAPConfig
{
    GENERATED_BODY()

    // Default constructor
    FAPConfig()
        : ServerUrlAndPort(TEXT("archipelago.gg:38281"))
        , Slot(TEXT(""))
        , Password(TEXT(""))
        , AutoConnectOnStartup(false)
    {
    }

    // Parameterized constructor
    FAPConfig(
        const FString& InServerUrlAndPort,
        const FString& InSlot,
        const FString& InPassword,
        bool bInAutoConnectOnStartup
    )
        : ServerUrlAndPort(InServerUrlAndPort)
        , Slot(InSlot)
        , Password(InPassword)
        , AutoConnectOnStartup(bInAutoConnectOnStartup)
    {
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Config",
        meta = (DisplayName = "Server URL & Port", Tooltip = "Archipelago server address and port. Use the address and port provided on your Multiworld room page (e.g. archipelago.gg:12345)."))
    FString ServerUrlAndPort;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Config",
        meta = (DisplayName = "Slot", Tooltip = "The Slot to connect as"))
    FString Slot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Config",
        meta = (DisplayName = "Password", Tooltip = "The Password for the Slot to connect as (encoded)"))
    FString Password;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archipelago|Config",
        meta = (DisplayName = "Auto Connect on Startup", Tooltip = "Try to connect to Archipelago on Game Startup"))
    bool AutoConnectOnStartup;
};

#pragma endregion