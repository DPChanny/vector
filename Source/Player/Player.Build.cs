using UnrealBuildTool;

public class Player : ModuleRules
{
	public Player(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		bUseRTTI = true;

		PublicDependencyModuleNames.AddRange([
				"Core"
			]
		);

		PrivateDependencyModuleNames.AddRange([
				"InputCore",
				"EnhancedInput",
				"CoreUObject",
				"Engine",
				"Voxel"
			]
		);
	}
}