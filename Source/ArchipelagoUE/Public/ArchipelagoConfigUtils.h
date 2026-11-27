// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ArchipelagoEnums.h"

class ARCHIPELAGOUE_API FArchipelagoConfigUtils
{
public:
    static FString GetConfigPath();

    static EAPConfigCreateResult CreateConfigFile(bool bOverrideExisting);
};