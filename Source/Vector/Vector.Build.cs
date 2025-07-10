// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Vector : ModuleRules
{
	public Vector(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(["Core", "CoreUObject", "Engine"]);

		PrivateDependencyModuleNames.AddRange([
			"InputCore", "EnhancedInput", "Niagara", "ProceduralMeshComponent", "UMG", "SlateCore", "Slate"
		]);
	}
}