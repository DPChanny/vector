// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Vector : ModuleRules
{
	public Vector(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine" });

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"InputCore", "EnhancedInput", "Niagara", "ProceduralMeshComponent", "UMG", "SlateCore", "Slate"
		});
	}
}