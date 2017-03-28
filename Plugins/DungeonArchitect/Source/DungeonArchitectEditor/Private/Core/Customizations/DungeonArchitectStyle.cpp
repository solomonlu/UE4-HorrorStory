//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "DungeonArchitectStyle.h"
#include "IPluginManager.h"

#define BASE_PATH FPaths::GamePluginsDir() / "DungeonArchitect/Content"
#define IMAGE_PLUGIN_BRUSH( RelativePath, ... ) FSlateImageBrush( FDungeonArchitectStyle::InContent( RelativePath, ".png" ), __VA_ARGS__ )

TSharedPtr< FSlateStyleSet > FDungeonArchitectStyle::StyleInstance = nullptr;

FString FDungeonArchitectStyle::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("DungeonArchitect"))->GetContentDir();
	return (ContentDir / RelativePath) + Extension;
}

void FDungeonArchitectStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FDungeonArchitectStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FDungeonArchitectStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("DungeonArchitectStyle"));
	return StyleSetName;
}

TSharedRef< class FSlateStyleSet > FDungeonArchitectStyle::Create()
{
	const FVector2D Icon16x16(16.0f, 16.0f);
	const FVector2D Icon20x20(20.0f, 20.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);
	const FVector2D Icon48x48(48.0f, 48.0f);

	TSharedRef<FSlateStyleSet> StyleRef = MakeShareable(new FSlateStyleSet(FDungeonArchitectStyle::GetStyleSetName()));
	StyleRef->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	StyleRef->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

	FSlateStyleSet& Style = StyleRef.Get();
	// Generic styles
	{
		Style.Set("DungeonArchitect.TabIcon", new IMAGE_PLUGIN_BRUSH("Icons/icon_DungeonEd_40x", Icon40x40));
		Style.Set("DungeonArchitect.TabIcon.Small", new IMAGE_PLUGIN_BRUSH("Icons/icon_DungeonEd_40x", Icon40x40));
	}

	// Editor Mode Styles
	{
		Style.Set("DungeonArchitect.ModePaint", new IMAGE_PLUGIN_BRUSH("Icons/DungeonEditMode/icon_DungeonEdMode_Paint_40x", Icon40x40));
		Style.Set("DungeonArchitect.ModeRectangle", new IMAGE_PLUGIN_BRUSH("Icons/DungeonEditMode/icon_DungeonEdMode_Rectangle_40x", Icon40x40));
		Style.Set("DungeonArchitect.ModeBorder", new IMAGE_PLUGIN_BRUSH("Icons/DungeonEditMode/icon_DungeonEdMode_Border_40x", Icon40x40));
		Style.Set("DungeonArchitect.ModeSelect", new IMAGE_PLUGIN_BRUSH("Icons/DungeonEditMode/icon_DungeonEdMode_Select_40x", Icon40x40));
	}

	return StyleRef;
}


#undef IMAGE_PLUGIN_BRUSH

const ISlateStyle& FDungeonArchitectStyle::Get()
{
	return *StyleInstance;
}
