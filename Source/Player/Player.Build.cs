using UnrealBuildTool;

public class Player : ModuleRules
{
	public Player(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"InputCore",
				"EnhancedInput",
				"CoreUObject",
				"Engine",
				"Voxel"
			}
		);
	}
}