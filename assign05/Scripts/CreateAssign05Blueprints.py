import unreal


def ensure_path(path):
    editor_asset_lib = unreal.EditorAssetLibrary
    if not editor_asset_lib.does_directory_exist(path):
        editor_asset_lib.make_directory(path)


def create_blueprint(asset_path, asset_name, parent_class):
    full_path = f"{asset_path}/{asset_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(full_path):
        return unreal.EditorAssetLibrary.load_asset(full_path)

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("ParentClass", parent_class)
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    return tools.create_asset(asset_name, asset_path, None, factory)


def main():
    path = "/Game/Assign05/Blueprints"
    ensure_path(path)

    character_class = unreal.load_class(None, "/Script/assign05.Assign05Character")
    platform_class = unreal.load_class(None, "/Script/assign05.MovingPlatform")
    game_mode_class = unreal.load_class(None, "/Script/assign05.Assign05GameMode")
    spawn_volume_class = unreal.load_class(None, "/Script/assign05.SpawnVolume")
    pickup_item_class = unreal.load_class(None, "/Script/assign05.PickupItemBase")
    healing_item_class = unreal.load_class(None, "/Script/assign05.HealingItem")
    mine_item_class = unreal.load_class(None, "/Script/assign05.MineItem")
    stage_transition_class = unreal.load_class(None, "/Script/assign05.Assign05StageTransitionWidget")

    character_bp = create_blueprint(path, "BP_Assign05Character", character_class)
    platform_bp = create_blueprint(path, "BP_MovingPlatform", platform_class)
    game_mode_bp = create_blueprint(path, "BP_Assign05GameMode", game_mode_class)
    spawn_volume_bp = create_blueprint(path, "BP_SpawnVolume", spawn_volume_class)
    pickup_item_bp = create_blueprint(path, "BP_PickupItemBase", pickup_item_class)
    healing_item_bp = create_blueprint(path, "BP_HealingItem", healing_item_class)
    mine_item_bp = create_blueprint(path, "BP_MineItem", mine_item_class)
    stage_transition_bp = create_blueprint(path, "WBP_StageTransition", stage_transition_class)

    if game_mode_bp and stage_transition_bp:
        game_mode_cdo = unreal.get_default_object(game_mode_bp.generated_class())
        game_mode_cdo.set_editor_property("stage_transition_widget_class", stage_transition_bp.generated_class())

    if spawn_volume_bp and healing_item_bp and mine_item_bp:
        spawn_volume_cdo = unreal.get_default_object(spawn_volume_bp.generated_class())
        spawn_volume_cdo.set_editor_property(
            "spawnable_classes",
            [
                pickup_item_bp.generated_class(),
                healing_item_bp.generated_class(),
                mine_item_bp.generated_class(),
            ],
        )

    for asset in (character_bp, platform_bp, game_mode_bp, spawn_volume_bp, pickup_item_bp, healing_item_bp, mine_item_bp, stage_transition_bp):
        unreal.EditorAssetLibrary.save_loaded_asset(asset)

    unreal.log("Created Assign05 gameplay Blueprints in /Game/Assign05/Blueprints.")


main()
