// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

using UnrealBuildTool;
using System.Collections.Generic;

public class LiftgateStudyTarget : TargetRules
{
	public LiftgateStudyTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("LiftgateStudy");
	}
}
