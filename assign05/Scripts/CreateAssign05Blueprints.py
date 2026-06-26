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


def set_first_existing_property(obj, property_names, value, warning_label=None):
    for property_name in property_names:
        try:
            obj.set_editor_property(property_name, value)
            return True
        except Exception:
            pass

    if warning_label:
        unreal.log_warning(f"Could not set {warning_label}. Tried: {', '.join(property_names)}")
    return False


def main():
    path = "/Game/Assign05/Blueprints"
    ui_path = "/Game/Assign05/UI"
    ensure_path(path)
    ensure_path(ui_path)

    character_class = unreal.load_class(None, "/Script/assign05.Assign05Character")
    platform_class = unreal.load_class(None, "/Script/assign05.MovingPlatform")
    game_mode_class = unreal.load_class(None, "/Script/assign05.Assign05GameMode")
    spawn_volume_class = unreal.load_class(None, "/Script/assign05.SpawnVolume")
    rising_obstacle_class = unreal.load_class(None, "/Script/assign05.RisingObstacle")
    fall_death_trigger_class = unreal.load_class(None, "/Script/assign05.FallDeathTrigger")
    pickup_item_class = unreal.load_class(None, "/Script/assign05.PickupItemBase")
    healing_item_class = unreal.load_class(None, "/Script/assign05.HealingItem")
    mine_item_class = unreal.load_class(None, "/Script/assign05.MineItem")
    hud_class = unreal.load_class(None, "/Script/assign05.Assign05HUDWidget")
    stage_transition_class = unreal.load_class(None, "/Script/assign05.Assign05StageTransitionWidget")

    character_bp = create_blueprint(path, "BP_Assign05Character", character_class)
    platform_bp = create_blueprint(path, "BP_MovingPlatform", platform_class)
    game_mode_bp = create_blueprint(path, "BP_Assign05GameMode", game_mode_class)
    spawn_volume_bp = create_blueprint(path, "BP_SpawnVolume", spawn_volume_class)
    rising_obstacle_bp = create_blueprint(path, "BP_RisingObstacle", rising_obstacle_class)
    fall_death_trigger_bp = create_blueprint(path, "BP_FallDeathTrigger", fall_death_trigger_class)
    pickup_item_bp = create_blueprint(path, "BP_PickupItemBase", pickup_item_class)
    healing_item_bp = create_blueprint(path, "BP_HealingItem", healing_item_class)
    mine_item_bp = create_blueprint(path, "BP_MineItem", mine_item_class)
    hud_bp = create_blueprint(ui_path, "WBP_HUD", hud_class)
    stage_transition_bp = create_blueprint(ui_path, "WBP_StageTransition", stage_transition_class)
    hp_bp = unreal.EditorAssetLibrary.load_asset(f"{ui_path}/WBP_HP")
    end_game_bp = unreal.EditorAssetLibrary.load_asset(f"{ui_path}/WBP_EndGame")

    if character_bp and hp_bp:
        character_cdo = unreal.get_default_object(character_bp.generated_class())
        set_first_existing_property(
            character_cdo,
            ["hp_widget_class"],
            hp_bp.generated_class(),
            "HP widget class on BP_Assign05Character",
        )

    if game_mode_bp:
        game_mode_cdo = unreal.get_default_object(game_mode_bp.generated_class())
        if hud_bp:
            game_mode_cdo.set_editor_property("hud_widget_class", hud_bp.generated_class())
        if stage_transition_bp:
            game_mode_cdo.set_editor_property("stage_transition_widget_class", stage_transition_bp.generated_class())
        if end_game_bp:
            game_mode_cdo.set_editor_property("end_game_widget_class", end_game_bp.generated_class())
        set_first_existing_property(
            game_mode_cdo,
            ["travel_to_optional_map_names", "b_travel_to_optional_map_names"],
            True,
            "map travel option on BP_Assign05GameMode",
        )

    if spawn_volume_bp and healing_item_bp and mine_item_bp:
        spawn_volume_cdo = unreal.get_default_object(spawn_volume_bp.generated_class())
        set_first_existing_property(
            spawn_volume_cdo,
            ["required_pickup_class"],
            pickup_item_bp.generated_class(),
            "required pickup class on BP_SpawnVolume",
        )
        spawn_volume_cdo.set_editor_property(
            "spawnable_classes",
            [
                pickup_item_bp.generated_class(),
                healing_item_bp.generated_class(),
                mine_item_bp.generated_class(),
            ],
        )

    if rising_obstacle_bp:
        rising_obstacle_cdo = unreal.get_default_object(rising_obstacle_bp.generated_class())
        set_first_existing_property(
            rising_obstacle_cdo,
            ["toggle_interval"],
            5.0,
            "toggle interval on BP_RisingObstacle",
        )
        set_first_existing_property(
            rising_obstacle_cdo,
            ["local_rise_offset"],
            unreal.Vector(0.0, 0.0, 180.0),
            "rise offset on BP_RisingObstacle",
        )
        set_first_existing_property(
            rising_obstacle_cdo,
            ["move_speed"],
            260.0,
            "move speed on BP_RisingObstacle",
        )

    if pickup_item_bp:
        pickup_item_cdo = unreal.get_default_object(pickup_item_bp.generated_class())
        set_first_existing_property(
            pickup_item_cdo,
            ["counts_toward_wave_goal", "b_counts_toward_wave_goal"],
            True,
            "wave goal count on BP_PickupItemBase",
        )
        set_first_existing_property(
            pickup_item_cdo,
            ["score_value"],
            100,
            "score value on BP_PickupItemBase",
        )

    if healing_item_bp:
        healing_item_cdo = unreal.get_default_object(healing_item_bp.generated_class())
        set_first_existing_property(
            healing_item_cdo,
            ["counts_toward_wave_goal", "b_counts_toward_wave_goal"],
            False,
            "wave goal count on BP_HealingItem",
        )

    if mine_item_bp:
        mine_item_cdo = unreal.get_default_object(mine_item_bp.generated_class())
        set_first_existing_property(
            mine_item_cdo,
            ["counts_toward_wave_goal", "b_counts_toward_wave_goal"],
            False,
            "wave goal count on BP_MineItem",
        )
        set_first_existing_property(
            mine_item_cdo,
            ["destroy_on_pickup", "b_destroy_on_pickup"],
            True,
            "destroy on pickup on BP_MineItem",
        )
        set_first_existing_property(
            mine_item_cdo,
            ["damage_amount"],
            20.0,
            "damage amount on BP_MineItem",
        )
        set_first_existing_property(
            mine_item_cdo,
            ["camera_reverse_duration"],
            5.0,
            "camera reverse duration on BP_MineItem",
        )

    for asset in (character_bp, platform_bp, game_mode_bp, spawn_volume_bp, rising_obstacle_bp, fall_death_trigger_bp, pickup_item_bp, healing_item_bp, mine_item_bp, hud_bp, hp_bp, stage_transition_bp, end_game_bp):
        if asset:
            try:
                unreal.KismetEditorUtilities.compile_blueprint(asset)
            except Exception:
                pass
            unreal.EditorAssetLibrary.save_loaded_asset(asset)

    unreal.log("Created Assign05 gameplay Blueprints in /Game/Assign05/Blueprints.")


main()
