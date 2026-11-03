// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class silent_planet : ModuleRules
{
	public silent_planet(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
            "SlateCore"
        });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"silent_planet",
			"silent_planet/Variant_Horror",
			"silent_planet/Variant_Horror/UI",
			"silent_planet/Variant_Shooter",
			"silent_planet/Variant_Shooter/AI",
			"silent_planet/Variant_Shooter/UI",
			"silent_planet/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
