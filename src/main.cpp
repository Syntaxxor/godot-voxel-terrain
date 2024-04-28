#include <gdextension_interface.h>

#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>

#include "voxel-terrain.hpp"
#include "voxel-data.hpp"
#include "voxel-mesh.hpp"
#include "terrain-control.hpp"

using namespace godot;

void register_types(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
    ClassDB::register_class<VoxelData>();
    ClassDB::register_class<VoxelMesh>();
    ClassDB::register_class<VoxelTerrain>();
    ClassDB::register_class<TerrainControl>();
}

void unregister_types(ModuleInitializationLevel p_level) {

}

extern "C"
{
    GDExtensionBool GDE_EXPORT voxel_terrain_init(
        GDExtensionInterfaceGetProcAddress p_get_proc_address,
        GDExtensionClassLibraryPtr p_library,
        GDExtensionInitialization *r_initialization) {
        GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

        init_obj.register_initializer(register_types);
        init_obj.register_terminator(unregister_types);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

        return init_obj.init();
    }
}
