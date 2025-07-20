// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using UnrealBuildTool;

public class VectorEditorTarget : TargetRules
{
	public VectorEditorTarget(TargetInfo target) : base(target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		ExtraModuleNames.Add("Vector");
		RegisterModulesCreatedByRider();
	}

	private void RegisterModulesCreatedByRider()
	{
		ExtraModuleNames.AddRange(["Voxel", "Player"]);
	}
}