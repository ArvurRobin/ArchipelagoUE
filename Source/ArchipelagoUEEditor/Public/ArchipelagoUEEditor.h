// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FArchipelagoUEEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};