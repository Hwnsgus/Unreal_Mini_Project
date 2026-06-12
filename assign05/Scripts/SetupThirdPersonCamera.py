import unreal


def find_component(actor_cdo, component_name):
    components = actor_cdo.get_components_by_class(unreal.ActorComponent)
    for component in components:
        if component.get_name() == component_name:
            return component
    return None


def main():
    asset_path = "/Game/Assign05/Blueprints/BP_Assign05Character"
    character_bp = unreal.EditorAssetLibrary.load_asset(asset_path)
    if character_bp is None:
        unreal.log_error(f"Could not load {asset_path}")
        return

    generated_class = character_bp.generated_class()
    character_cdo = unreal.get_default_object(generated_class)

    camera_boom = find_component(character_cdo, "CameraBoom")
    follow_camera = find_component(character_cdo, "FollowCamera")

    if camera_boom is None:
        unreal.log_error("CameraBoom component was not found on BP_Assign05Character.")
        return

    if follow_camera is None:
        unreal.log_error("FollowCamera component was not found on BP_Assign05Character.")
        return

    camera_boom.set_editor_property("target_arm_length", 500.0)
    camera_boom.set_editor_property("use_pawn_control_rotation", True)
    camera_boom.set_editor_property("do_collision_test", True)
    camera_boom.set_relative_location(unreal.Vector(0.0, 0.0, 75.0), False, False)
    camera_boom.set_relative_rotation(unreal.Rotator(0.0, 0.0, 0.0), False, False)

    follow_camera.set_editor_property("use_pawn_control_rotation", False)
    follow_camera.set_relative_location(unreal.Vector(0.0, 0.0, 0.0), False, False)
    follow_camera.set_relative_rotation(unreal.Rotator(0.0, 0.0, 0.0), False, False)

    character_cdo.set_editor_property("use_controller_rotation_pitch", False)
    character_cdo.set_editor_property("use_controller_rotation_yaw", False)
    character_cdo.set_editor_property("use_controller_rotation_roll", False)
    character_cdo.set_editor_property("auto_possess_player", unreal.AutoReceiveInput.PLAYER0)

    unreal.EditorAssetLibrary.save_loaded_asset(character_bp)
    unreal.log("BP_Assign05Character third-person CameraBoom/FollowCamera defaults saved.")


main()
