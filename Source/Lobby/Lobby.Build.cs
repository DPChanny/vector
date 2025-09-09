using UnrealBuildTool;

public class Lobby : ModuleRules
{
	public Lobby(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange([
			"Core"
		]);

		PrivateDependencyModuleNames.AddRange([
			"CoreUObject",
			"Engine",

			"UMG",
			"CommonUI",

			"Vector",
			"VectorCore"
		]);
	}
}