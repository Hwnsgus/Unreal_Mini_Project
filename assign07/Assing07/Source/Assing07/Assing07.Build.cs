// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Assing07 : ModuleRules
{
	public Assing07(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
		
		PublicIncludePaths.AddRange(new string[]
		{
			"Assing07", // 모듈 루트 아래의 헤더를 찾기 위한 프로젝트 include 경로입니다.
		});
	}
}
