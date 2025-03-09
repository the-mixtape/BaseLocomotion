// Mixtape's Base Locomotion All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class BaseLocomotionEditorTarget : TargetRules
{
	public BaseLocomotionEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;

		ExtraModuleNames.AddRange( new string[] { "BaseLocomotion" } );
	}
}
