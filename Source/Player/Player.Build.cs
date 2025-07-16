using UnrealBuildTool;

public class Player : ModuleRules
{
	public Player(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		bUseRTTI = true;

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