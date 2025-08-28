// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Vector : ModuleRules
{
	public Vector(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(["Core"]);

		PrivateDependencyModuleNames.AddRange([
			"CoreUObject",
			"Engine",

			"EnhancedInput",

			"VectorCore",
			"Voxel"
		]);
	}
}