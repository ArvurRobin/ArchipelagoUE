// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

#include "ArchipelagoUEEditor.h"
#include "ArchipelagoConfigUtils.h"
#include "ArchipelagoStructs.h"
#include "ArchipelagoEnums.h"

#define LOCTEXT_NAMESPACE "FArchipelagoUEEditorModule"

void FArchipelagoUEEditorModule::StartupModule()
{
    const FString ConfigPath = FArchipelagoConfigUtils::GetConfigPath();

    if (FPaths::FileExists(ConfigPath))
    {
        return;
    }

    const EAPConfigCreateResult Result = FArchipelagoConfigUtils::CreateConfigFile(false);
}

void FArchipelagoUEEditorModule::ShutdownModule()
{
    // Called when the Editor module is unloaded.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FArchipelagoUEEditorModule, ArchipelagoUEEditor)