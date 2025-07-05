// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Vector : ModuleRules
{
	public Vector(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine"});

		PrivateDependencyModuleNames.AddRange(new string[] { "InputCore", "EnhancedInput", "Niagara", "ProceduralMeshComponent", "UMG", "SlateCore", "Slate" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");
	}
}
