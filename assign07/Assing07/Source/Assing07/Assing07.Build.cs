// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Assing07 : ModuleRules
{
	public Assing07(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
