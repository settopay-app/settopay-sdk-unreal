// Copyright Setto. All Rights Reserved.

using UnrealBuildTool;

public class SettoSDK : ModuleRules
{
    public SettoSDK(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "HTTP",
                "Json",
                "JsonUtilities"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Slate",
                "SlateCore"
            }
        );

        // Platform-specific dependencies
        if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            PublicFrameworks.AddRange(
                new string[]
                {
                    "SafariServices",
                    "UIKit"
                }
            );
        }

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PrivateDependencyModuleNames.Add("Launch");

            // Android Custom Tabs
            AdditionalPropertiesForReceipt.Add(
                "AndroidPlugin",
                System.IO.Path.Combine(ModuleDirectory, "SettoSDK_UPL.xml")
            );
        }
    }
}
