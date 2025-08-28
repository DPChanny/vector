using UnrealBuildTool;

public class Menu : ModuleRules
{
	public Menu(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange([
			"Core"
		]);

		PrivateDependencyModuleNames.AddRange([
			"CoreUObject",
			"Engine",

			"Slate",
			"SlateCore",
			"UMG"
		]);
	}
}