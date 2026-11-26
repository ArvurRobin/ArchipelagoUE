// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

using UnrealBuildTool;

public class ArchipelagoUE : ModuleRules
{
    public ArchipelagoUE(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "WebSocketsSubsystem",
                "Json"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "JsonUtilities"
            }
        );
    }
}