// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

using UnrealBuildTool;

public class ArchipelagoUEEditor : ModuleRules
{
    public ArchipelagoUEEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "ArchipelagoUE"
            }
        );
    }
}