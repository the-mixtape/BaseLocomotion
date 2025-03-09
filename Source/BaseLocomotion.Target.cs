// Mixtape's Base Locomotion All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class BaseLocomotionTarget : TargetRules
{
	public BaseLocomotionTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;

		ExtraModuleNames.AddRange( new string[] { "BaseLocomotion" } );
	}
}
