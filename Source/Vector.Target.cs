// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VectorTarget : TargetRules
{
	public VectorTarget(TargetInfo target) : base(target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		ExtraModuleNames.Add("Vector");
		RegisterModulesCreatedByRider();
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
	}

	private void RegisterModulesCreatedByRider()
	{
		ExtraModuleNames.AddRange(["Voxel", "Player"]);
	}
}