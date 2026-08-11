using UnrealBuildTool;

public class Temporary : ModuleRules
{
	public Temporary(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.Add("Core");
	}
}
