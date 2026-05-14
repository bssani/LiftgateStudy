// Copyright Notice: Internal evaluation tool. GMTCK PQDQ.

using UnrealBuildTool;

public class LiftgateStudy : ModuleRules
{
	public LiftgateStudy(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// CLAUDE.md §6: 공개 API 모듈은 PublicDependency, 내부 구현용은 PrivateDependency
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
