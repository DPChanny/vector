using UnrealBuildTool;

public class Voxel : ModuleRules
{
	public Voxel(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		bUseRTTI = true;

		PublicDependencyModuleNames.AddRange([
				"Core"
			]
		);

		PrivateDependencyModuleNames.AddRange([
				"Niagara",
				"UMG",
				"ProceduralMeshComponent",
				"CoreUObject",
				"Engine",
				"VectorCore"
			]
		);
	}
}