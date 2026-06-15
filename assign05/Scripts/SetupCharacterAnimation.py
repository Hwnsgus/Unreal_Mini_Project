import unreal


def find_component(actor_cdo, component_class, preferred_names=None):
    preferred_names = preferred_names or []
    components = actor_cdo.get_components_by_class(component_class)

    for name in preferred_names:
        for component in components:
            if component.get_name() == name:
                return component

    return components[0] if components else None


def set_property_or_call(component, property_name, value, fallback_method_name=None):
    try:
        component.set_editor_property(property_name, value)
        return True
    except Exception:
        pass

    if fallback_method_name and hasattr(component, fallback_method_name):
        getattr(component, fallback_method_name)(value)
        return True

    return False


def main():
    character_bp_path = "/Game/Assign05/Blueprints/BP_Assign05Character"
    skeletal_mesh_path = "/Game/Characters/Mannequins/Meshes/SKM_Manny"
    anim_class_path = "/Game/Characters/Mannequins/Animations/ABP_Manny.ABP_Manny_C"

    character_bp = unreal.EditorAssetLibrary.load_asset(character_bp_path)
    skeletal_mesh = unreal.EditorAssetLibrary.load_asset(skeletal_mesh_path)
    anim_class = unreal.load_class(None, anim_class_path)

    if character_bp is None:
        unreal.log_error(f"Could not load {character_bp_path}")
        return

    if skeletal_mesh is None:
        unreal.log_error(f"Could not load {skeletal_mesh_path}")
        return

    if anim_class is None:
        unreal.log_error(f"Could not load {anim_class_path}")
        return

    character_cdo = unreal.get_default_object(character_bp.generated_class())
    mesh_component = find_component(
        character_cdo,
        unreal.SkeletalMeshComponent,
        ["CharacterMesh0", "Mesh"],
    )

    if mesh_component is None:
        unreal.log_error("Could not find the character SkeletalMeshComponent.")
        return

    set_property_or_call(mesh_component, "skeletal_mesh_asset", skeletal_mesh, "set_skeletal_mesh")
    mesh_component.set_editor_property("animation_mode", unreal.AnimationMode.ANIMATION_BLUEPRINT)
    set_property_or_call(mesh_component, "anim_class", anim_class, "set_anim_instance_class")

    mesh_component.set_relative_location(unreal.Vector(0.0, 0.0, -90.0), False, False)
    mesh_component.set_relative_rotation(unreal.Rotator(0.0, -90.0, 0.0), False, False)

    unreal.EditorAssetLibrary.save_loaded_asset(character_bp)
    unreal.log("BP_Assign05Character now uses SKM_Manny with ABP_Manny animation.")


main()
