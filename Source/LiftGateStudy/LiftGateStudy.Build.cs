// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

using UnrealBuildTool;

public class LiftGateStudy : ModuleRules
{
	public LiftGateStudy(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// CLAUDE.md §6 / ADR-005: C++ widget + ISDK input + HMD location 사용
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"UMG",
			"Slate",
			"SlateCore",
			"HeadMountedDisplay"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			// ISDK / Meta XR API 를 직접 호출하게 되면 여기 추가:
			// "OculusXRHMD", "MetaXRInteraction" 등 (ADR-001, R5)
		});
	}
}
