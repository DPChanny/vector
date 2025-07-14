// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VectorEditorTarget : TargetRules
{
	public VectorEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.Add("Vector");
		RegisterModulesCreatedByRider();
	}

	private void RegisterModulesCreatedByRider()
	{
		ExtraModuleNames.AddRange(new[] { "Voxel", "Player" });
	}
}