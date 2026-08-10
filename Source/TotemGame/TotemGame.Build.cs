using UnrealBuildTool;

public class TotemGame : ModuleRules
{
	public TotemGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", "CoreUObject", "Engine", "InputCore", "NetCore",
			"Totem",
		});
	}
}
