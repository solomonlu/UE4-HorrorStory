

namespace UnrealBuildTool.Rules
{
	public class DungeonArchitectRuntime : ModuleRules
	{
		public DungeonArchitectRuntime(TargetInfo Target)
		{
            //bFasterWithoutUnity = true;
			PublicIncludePaths.AddRange(
				new string[] {
					// ... add public include paths required here ...
				}
				);

			PrivateIncludePaths.AddRange(
				new string[] {
                    "DungeonArchitectRuntime/Private",
                    "DungeonArchitectRuntime/Public",
					// ... add other private include paths required here ...
				}
				);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
				    "Engine",
                    "Foliage"
					// ... add other public dependencies that you statically link with here ...
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					// ... add private dependencies that you statically link with here ...
				}
				);

			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
					// ... add any modules that your module loads dynamically here ...
				}
				);
		}
	}
}