#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/physics_direct_space_state3d.hpp>
#include <godot_cpp/classes/physics_ray_query_parameters3d.hpp>
#include <godot_cpp/classes/world3d.hpp>

#include "terrain-control.hpp"


void TerrainControl::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_terrain", "terrain"), &TerrainControl::set_terrain);
    ClassDB::bind_method(D_METHOD("get_terrain"), &TerrainControl::get_terrain);

    ClassDB::bind_method(D_METHOD("set_edit_shape", "edit_shape"), &TerrainControl::set_edit_shape);
    ClassDB::bind_method(D_METHOD("get_edit_shape"), &TerrainControl::get_edit_shape);

    ClassDB::bind_method(D_METHOD("set_edit_tool", "edit_tool"), &TerrainControl::set_edit_tool);
    ClassDB::bind_method(D_METHOD("get_edit_tool"), &TerrainControl::get_edit_tool);

    ClassDB::bind_method(D_METHOD("set_target_location", "target_location"), &TerrainControl::set_target_location);
    ClassDB::bind_method(D_METHOD("get_target_location"), &TerrainControl::get_target_location);

    ClassDB::bind_method(D_METHOD("set_size", "size"), &TerrainControl::set_size);
    ClassDB::bind_method(D_METHOD("get_size"), &TerrainControl::get_size);

    ClassDB::bind_method(D_METHOD("set_strength", "strength"), &TerrainControl::set_strength);
    ClassDB::bind_method(D_METHOD("get_strength"), &TerrainControl::get_strength);

    ClassDB::bind_method(D_METHOD("update_target_location", "r0", "rd"), &TerrainControl::update_target_location);
    ClassDB::bind_method(D_METHOD("get_weight_at", "off"), &TerrainControl::get_weight_at);
    ClassDB::bind_method(D_METHOD("apply_edit"), &TerrainControl::apply_edit);

    ADD_SIGNAL(MethodInfo("edit_shape_changed"));

    BIND_CONSTANT(SHAPE_SPHERE);
    BIND_CONSTANT(SHAPE_BOX);
    BIND_CONSTANT(SHAPE_CYLINDER);
}


TerrainControl::TerrainControl() {
    terrain = nullptr;
    target_location = Vector3(0.0, 0.0, 0.0);
    size = 4.0;
    strength = 0;
    default_dist = 10.0;
    edit_shape = 0;
    edit_tool = 0;
}


TerrainControl::~TerrainControl() {

}


void TerrainControl::set_terrain(VoxelTerrain *terrain) {
    this->terrain = terrain;
}

VoxelTerrain *TerrainControl::get_terrain() {
    return this->terrain;
}


void TerrainControl::set_edit_shape(uint32_t edit_shape) {
    this->edit_shape = edit_shape;
    emit_signal("edit_shape_changed");
}

uint32_t TerrainControl::get_edit_shape() const {
    return edit_shape;
}


void TerrainControl::set_edit_tool(uint32_t edit_tool) {
    this->edit_tool = edit_tool;
    UtilityFunctions::print(edit_tool);
}

uint32_t TerrainControl::get_edit_tool() const {
    return edit_tool;
}


void TerrainControl::set_target_location(Vector3 target_location) {
    this->target_location = target_location;
}

Vector3 TerrainControl::get_target_location() const {
    return target_location;
}


void TerrainControl::set_size(float size) {
    if(size > 0.0){
        this->size = size;
    }
}

float TerrainControl::get_size() const {
    return size;
}


void TerrainControl::set_strength(float strength) {
    this->strength = Math::clamp(strength, -1.0f, 1.0f);
}

float TerrainControl::get_strength() const {
    return strength;
}

void TerrainControl::update_target_location(Vector3 r0, Vector3 rd) {
    if(terrain){
        // Cast a ray to find the next target distance.
        PhysicsDirectSpaceState3D *space_state = terrain->get_world_3d()->get_direct_space_state();

        PhysicsRayQueryParameters3D query_parameters;
        query_parameters.set_from(r0);
        query_parameters.set_to(r0 + rd * 4096);

        Dictionary intersection = space_state->intersect_ray(&query_parameters);

        if(intersection.is_empty()) {
            // If nothing is found, use a default distance based on previous intersections.
            // This will keep the control consistent if it suddenly loses collisions.
            target_location = r0 + (rd * default_dist);
        } else {
            Vector3 intersection_point = intersection.get("position", Vector3(0.0, 0.0, 0.0));
            default_dist = r0.distance_to(intersection_point); // Set the default distance.
            target_location = intersection_point;
        }
    }
}

float TerrainControl::get_weight_at(Vector3i off) {
    if(edit_shape == SHAPE_SPHERE) {
        if(off.length() >= size) {
            return 0.0;
        }
        return (size - off.length()) / size;
    } else if(edit_shape == SHAPE_BOX) {
        int min = Math::min(off.x, Math::min(off.y, off.z));
        return (size - min) / size;
    } else if(edit_shape == SHAPE_CYLINDER) {
        Vector3i c_off = Vector3i(off.x, 0.0, off.z);
        if(c_off.length() >= size) {
            return 0.0;
        }
        return (size - c_off.length()) / size;
    }
    return 0.0;
}

void TerrainControl::apply_edit() {
    // Use to_local to make sure the terrain can be translated/scaled/rotated and remain consistent.
    Vector3 edit_location = terrain->to_local(target_location);
    Vector3i voxel_location = static_cast<Vector3i>(edit_location);
    // Iterate over our size.
    for(int z = -size; z <= size; z++) {
        for(int y = -size; y <= size; y++) {
            for(int x = -size; x <= size; x++) {
                Vector3i off = Vector3i(x, y, z);
                float weight = get_weight_at(off);
                if(weight != 0.0) {
                    if(edit_tool == TOOL_DRAW) {
                        edit_draw(voxel_location + off, weight * strength);
                    } else if(edit_tool == TOOL_SCULPT) {
                        edit_sculpt(voxel_location + off, weight * strength);
                    } else if(edit_tool == TOOL_SMOOTH) {
                        edit_smooth(voxel_location + off, weight * strength);
                    }
                }
            }
        }
    }
}

void TerrainControl::edit_draw(Vector3i location, float weight) {
    int32_t cur = terrain->get_voxel(location);
    cur += weight * 255;
    cur = Math::clamp(cur, 0, 255);
    terrain->set_voxel(location, cur);
}

void TerrainControl::edit_sculpt(Vector3i location, float weight) {
    int32_t cur = terrain->get_voxel(location);
    bool should_sculpt = false;
    if(weight > 0.0) {
        if(cur > 127) {
            should_sculpt = true;
        } else {
            // Check surrounding voxels
            for(int x = -1; x <= 1; x++) {
                for(int y = -1; y <= 1; y++) {
                    for(int z = -1; z <= 1; z++) {
                        if(terrain->get_voxel(location + Vector3i(x, y, z)) == 255) {
                            should_sculpt = true;
                            goto do_sculpting;
                        }
                    }
                }
            }
        }
    } else {
        if(cur < 128) {
            should_sculpt = true;
        } else {
            // Check surrounding voxels
            for(int x = -1; x <= 1; x++) {
                for(int y = -1; y <= 1; y++) {
                    for(int z = -1; z <= 1; z++) {
                        if(terrain->get_voxel(location + Vector3i(x, y, z)) == 0) {
                            should_sculpt = true;
                            goto do_sculpting;
                        }
                    }
                }
            }
        }
    }
    do_sculpting:
    if(should_sculpt) {
        cur += weight * 255;
        cur = Math::clamp(cur, 0, 255);
        terrain->set_voxel(location, cur);
    }
}


void TerrainControl::edit_smooth(Vector3i location, float weight) {
    int32_t average = 0;
    for(int x = -1; x <= 1; x++) {
        for(int y = -1; y <= 1; y++) {
            for(int z = -1; z <= 1; z++) {
                if(x != 0 || y != 0 || z != 0){
                    average += terrain->get_voxel(location + Vector3i(x, y, z));
                }
            }
        }
    }
    average /= 26;
    int32_t cur = terrain->get_voxel(location);
    cur = Math::move_toward(cur, average, weight);
    cur = Math::clamp(cur, 0, 255);
    terrain->set_voxel(location, cur);
}
