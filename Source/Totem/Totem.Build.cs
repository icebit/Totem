// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Totem : ModuleRules
{
	public Totem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "NetCore" });
	}
}
