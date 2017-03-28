//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "AssetToolsModule.h"
#include "PropertyEditorModule.h"
#include "LevelEditor.h"
#include "LevelEditorActions.h"
#include "AssetEditorToolkit.h"
#include "ModuleManager.h"
#include "IAssetTools.h"

#include "Core/Dungeon.h"
#include "Core/DungeonArchitectEditor.h"
#include "Core/DungeonArchitectCommands.h"
#include "Core/Asset/DungeonThemeAssetTypeActions.h"
#include "Core/Graph/EdGraphNode_DungeonActorBase.h"
#include "Core/Widgets/SGraphNode_DungeonActor.h"
#include "Core/Widgets/GraphPanelNodeFactory_DungeonProp.h"
#include "Core/Widgets/SDungeonEditorViewportToolbar.h"
#include "Core/EditorMode/DungeonEdMode.h"
#include "Core/EditorMode/DungeonEdModeHandlerFactory.h"
#include "Core/Customizations/DungeonArchitectStyle.h"
#include "Core/Customizations/DungeonArchitectEditorCustomization.h"
#include "Core/Customizations/DungeonArchitectGraphNodeCustomization.h"
#include "Core/Utils/DungeonEditorService.h"

#include "Builders/Grid/Customizations/DAGridSpatialConstraintCustomization.h"
#include "Builders/Snap/Asset/SnapDoorAssetTypeActions.h"
#include "Builders/Snap/DoorEditor/SnapDoorEditorCommands.h"
#include "Builders/Snap/Customizations/SnapEditorCustomization.h"
#include "Builders/Snap/DoorEditor/Preview3D/SSnapDoorPreview3DViewportToolbar.h"

#define LOCTEXT_NAMESPACE "DungeonArchitectEditorModule" 


class FDungeonArchitectEditorModule : public IDungeonArchitectEditorModule
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override {
		FDungeonEditorThumbnailPool::Create();
		FDungeonArchitectStyle::Initialize();     

		FDungeonArchitectCommands::Register();
		FDungeonEditorViewportCommands::Register();
		FSnapDoorEditorCommands::Register();
		FSnapDoorEditorViewportCommands::Register();

		// Add a category for the dungeon architect assets
		{
			IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
			DungeonAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Dungeon Architect")), LOCTEXT("DungeonArchitectAssetCategory", "Dungeon Architect"));
		}
		
		// Register the details customization
		{
			FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
			PropertyEditorModule.RegisterCustomClassLayout("Dungeon", FOnGetDetailCustomizationInstance::CreateStatic(&FDungeonArchitectEditorCustomization::MakeInstance));
			PropertyEditorModule.RegisterCustomClassLayout("DungeonVolume", FOnGetDetailCustomizationInstance::CreateStatic(&FDungeonArchitectVolumeCustomization::MakeInstance));
			PropertyEditorModule.RegisterCustomClassLayout("EdGraphNode_DungeonActorBase", FOnGetDetailCustomizationInstance::CreateStatic(&FDungeonArchitectVisualGraphNodeCustomization::MakeInstance));
			PropertyEditorModule.RegisterCustomClassLayout("DungeonEditorViewportProperties", FOnGetDetailCustomizationInstance::CreateStatic(&FDungeonEditorViewportPropertiesCustomization::MakeInstance));
			PropertyEditorModule.RegisterCustomClassLayout("SnapDoorMeshInfo", FOnGetDetailCustomizationInstance::CreateStatic(&FSnapDoorMeshInfoCustomization::MakeInstance));
			PropertyEditorModule.RegisterCustomClassLayout("DungeonDebug", FOnGetDetailCustomizationInstance::CreateStatic(&FDungeonDebugCustomization::MakeInstance));

			PropertyEditorModule.RegisterCustomPropertyTypeLayout("GridSpatialConstraint3x3Data", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FDAGridConstraintCustomization3x3::MakeInstance));
			PropertyEditorModule.RegisterCustomPropertyTypeLayout("GridSpatialConstraint2x2Data", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FDAGridConstraintCustomization2x2::MakeInstance));
			PropertyEditorModule.RegisterCustomPropertyTypeLayout("GridSpatialConstraintEdgeData", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FDAGridConstraintCustomizationEdge::MakeInstance));


			PropertyEditorModule.NotifyCustomizationModuleChanged();
		}

		// Register the editor mode handlers for the dungeon builders
		FDungeonEdModeHandlerFactory::Register();

		//RegisterToolbar();

		// Register asset types
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		RegisterAssetTypeAction(AssetTools, MakeShareable(new FDungeonThemeAssetTypeActions));
		RegisterAssetTypeAction(AssetTools, MakeShareable(new FDungeonThemeAssetTypeActionsV1));
		RegisterAssetTypeAction(AssetTools, MakeShareable(new FSnapDoorAssetTypeActions));

		// Register custom graph nodes
		GraphPanelNodeFactory = MakeShareable(new FGraphPanelNodeFactory_DungeonProp);
		FEdGraphUtilities::RegisterVisualNodeFactory(GraphPanelNodeFactory);
		//SGraphNode_DungeonPropMesh

		// Register the dungeon draw editor mode
		FEditorModeRegistry::Get().RegisterMode<FEdModeDungeon>(
			FEdModeDungeon::EM_Dungeon,
			NSLOCTEXT("EditorModes", "DungeonDrawMode", "Draw Dungeon"),
			FSlateIcon(FDungeonArchitectStyle::GetStyleSetName(), "DungeonArchitect.TabIcon", "DungeonArchitect.TabIcon.Small"),
			true, 400
			);


		// Track dungeon actor property change events to handle advanced dungeon details
		DungeonPropertyChangeListener = MakeShareable(new FDungeonPropertyChangeListener);
		DungeonPropertyChangeListener->Initialize();

		// Create and editor service, so the runtime module can access it
		IDungeonEditorService::Set(MakeShareable(new FDungeonEditorService));
	}

	class Local {
	public:
		static void AddToolBarCommands(FToolBarBuilder& ToolBarBuilder) {
			ToolBarBuilder.AddToolBarButton(FDungeonArchitectCommands::Get().OpenDungeonEditor);
		}
	};

	virtual void ShutdownModule() override {
		// Unregister all the asset types that we registered
		if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
		{
			IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
			for (int32 Index = 0; Index < CreatedAssetTypeActions.Num(); ++Index)
			{
				AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions[Index].ToSharedRef());
			}
		}
		CreatedAssetTypeActions.Empty();

		FEditorModeRegistry::Get().UnregisterMode(FEdModeDungeon::EM_Dungeon);

		FDungeonArchitectStyle::Shutdown();
	}

	static void OpenDungeonEditor() {
	}

	/** Creates a new dungeon editor */
	virtual TSharedRef<IDungeonArchitectEditor> CreateDungeonEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost) override {
		TSharedRef<FDungeonArchitectEditor> NewDungeonEditor(new FDungeonArchitectEditor());
		//NewDungeonEditor->InitDungeonEditor(Mode, InitToolkitHost);
		return NewDungeonEditor;
	}

	void RegisterToolbar() {
		// register the level editor commands

		// bind the level editor commands
		GlobalLevelEditorActions = TSharedPtr<FUICommandList>(new FUICommandList);
		FUICommandList& ActionList = *GlobalLevelEditorActions;
		ActionList.MapAction(FDungeonArchitectCommands::Get().OpenDungeonEditor, FExecuteAction::CreateStatic(&FDungeonArchitectEditorModule::OpenDungeonEditor));

		// Register the menu extenders
		{
			FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
			TSharedRef<FExtender> ToolbarExtender(new FExtender);
			ToolbarExtender->AddToolBarExtension(
				"Game",
				EExtensionHook::After,
				GlobalLevelEditorActions,
				FToolBarExtensionDelegate::CreateStatic(&Local::AddToolBarCommands));
			LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
		}
	}

	virtual EAssetTypeCategories::Type GetDungeonAssetCategoryBit() const {
		return DungeonAssetCategoryBit;
	}

public:
	TSharedPtr<FUICommandList> GlobalLevelEditorActions;
	TSharedPtr<FGraphPanelNodeFactory> GraphPanelNodeFactory;
	TSharedPtr<FDungeonPropertyChangeListener> DungeonPropertyChangeListener;


private:
	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
	{
		AssetTools.RegisterAssetTypeActions(Action);
		CreatedAssetTypeActions.Add(Action);
	}

	/** All created asset type actions.  Cached here so that we can unregister them during shutdown. */
	TArray< TSharedPtr<IAssetTypeActions> > CreatedAssetTypeActions;
	EAssetTypeCategories::Type DungeonAssetCategoryBit;
};

IMPLEMENT_MODULE(FDungeonArchitectEditorModule, DungeonArchitectEditor)


#undef LOCTEXT_NAMESPACE