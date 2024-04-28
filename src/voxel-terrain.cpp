#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/worker_thread_pool.hpp>

#include "voxel-mesh.hpp"
#include "voxel-terrain.hpp"
#include "voxel-data.hpp"

void VoxelTerrain::_bind_methods() {
    ClassDB::bind_method(D_METHOD("update_terrain"), &VoxelTerrain::update_terrain);

    ClassDB::bind_method(D_METHOD("set_chunks", "chunks"), &VoxelTerrain::set_chunks);
    ClassDB::bind_method(D_METHOD("get_chunks"), &VoxelTerrain::get_chunks);

    ClassDB::bind_method(D_METHOD("set_terrain_material", "terrain_material"), &VoxelTerrain::set_terrain_material);
    ClassDB::bind_method(D_METHOD("get_terrain_material"), &VoxelTerrain::get_terrain_material);

    ClassDB::bind_method(D_METHOD("set_voxel", "position", "value"), &VoxelTerrain::set_voxel);
    ClassDB::bind_method(D_METHOD("get_voxel", "position"), &VoxelTerrain::get_voxel);

    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "terrain_material", PROPERTY_HINT_RESOURCE_TYPE, "Material"), "set_terrain_material", "get_terrain_material");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "chunks"), "set_chunks", "get_chunks");
}


VoxelTerrain::VoxelTerrain() {
    chunks = Dictionary();
    terrain_material = Ref<Material>();
}


VoxelTerrain::~VoxelTerrain() {

}


void VoxelTerrain::_ready() {
    update_terrain();
}

void VoxelTerrain::_process(double delta) {
    // Only update if something has changed.
    // This should be light-weight enough to have negligible performance impact at runtime.
    if (dirty) {
        update_terrain();
    }
}


void VoxelTerrain::update_terrain() {
    dirty = false;
    std::vector<int64_t> threads;
    // No way to iterate for each value. Iterate with index instead.
    for (int64_t i = 0; i < chunks.keys().size(); i++) {
        Variant key = chunks.keys()[i];
        // There's basically no way it won't be the right type. Doesn't hurt to check though, right?
        if (key.get_type() == Variant::VECTOR3I) {
            Vector3i position = static_cast<Vector3i>(key);
            VoxelData *data = get_chunk(position);
            if(data) {
                if(data->get_dirty()) {
                    data->set_dirty(false);
                    // Clear empty chunks.
                    if(data->is_empty()) {
                        remove_chunk(position);
                        i--;
                        continue;
                    }

                    // Create a mesh if there's not one already.
                    if(!meshes.has(position)) {
                        VoxelMesh *mesh = memnew(VoxelMesh);
                        mesh->set_chunk_position(position);
                        mesh->set_mesh_material(terrain_material);
                        meshes[position] = mesh;
                        add_child(mesh);
                    }

                    // Add the update to the thread pool singleton.
                    Callable fn = meshes[position].get("update_mesh");
                    threads.push_back(WorkerThreadPool::get_singleton()->add_task(fn.bind(this), true));
                }
            }
        }
    }

    // Join the threads.
    for(int i = 0; i < threads.size(); i++) {
        WorkerThreadPool::get_singleton()->wait_for_task_completion(threads[i]);
    }
}


void VoxelTerrain::set_chunks(Dictionary chunks) {
    this->chunks = chunks;
}


Dictionary VoxelTerrain::get_chunks() const {
    return this->chunks;
}


void VoxelTerrain::set_terrain_material(Ref<Material> terrain_material) {
    this->terrain_material = terrain_material;
    for(int i = 0; i < meshes.size(); i++) {
        meshes.values()[i].call("set_mesh_material", terrain_material);
    }
}


Ref<Material> VoxelTerrain::get_terrain_material() const {
    return this->terrain_material;
}


void VoxelTerrain::set_voxel(Vector3i position, uint8_t value) {
    dirty = true;
    Vector3i voxel_pos = position;
    Vector3i chunk_pos(0, 0, 0);
    // Bring chunk and voxel position into the correct range.
    while(voxel_pos.x < 0) {
        voxel_pos.x += 16;
        chunk_pos.x -= 1;
    }
    while(voxel_pos.x >= 16) {
        voxel_pos.x -= 16;
        chunk_pos.x += 1;
    }
    while(voxel_pos.y < 0) {
        voxel_pos.y += 16;
        chunk_pos.y -= 1;
    }
    while(voxel_pos.y >= 16) {
        voxel_pos.y -= 16;
        chunk_pos.y += 1;
    }
    while(voxel_pos.z < 0) {
        voxel_pos.z += 16;
        chunk_pos.z -= 1;
    }
    while(voxel_pos.z >= 16) {
        voxel_pos.z -= 16;
        chunk_pos.z += 1;
    }

    // Add a new chunk data if there isn't one already.
    if(!chunks.has(chunk_pos)) {
        VoxelData *data(memnew(VoxelData));
        data->set_local_to_scene(true);
        data->set_name(UtilityFunctions::str("chunk_", chunk_pos.x, "-", chunk_pos.y, "-", chunk_pos.z));
        chunks[chunk_pos] = data;
    }

    VoxelData *data = get_chunk(chunk_pos);
    if(data) {
        data->set_voxel(voxel_pos, value);
        // Attempt to dirty surrounding chunks, to ensure chunk generation works properly.
        if(voxel_pos.x <= 1) {
            VoxelData *other = get_chunk(chunk_pos - Vector3i(1, 0, 0));
            if(other) {
                other->set_dirty(true);
            }
        } else if(voxel_pos.x >= 14) {
            VoxelData *other = get_chunk(chunk_pos + Vector3i(1, 0, 0));
            if(other) {
                other->set_dirty(true);
            }
        }
        if(voxel_pos.y <= 1) {
            VoxelData *other = get_chunk(chunk_pos - Vector3i(0, 1, 0));
            if(other) {
                other->set_dirty(true);
            }
        } else if(voxel_pos.y >= 14) {
            VoxelData *other = get_chunk(chunk_pos + Vector3i(0, 1, 0));
            if(other) {
                other->set_dirty(true);
            }
        }
        if(voxel_pos.z <= 1) {
            VoxelData *other = get_chunk(chunk_pos - Vector3i(0, 0, 1));
            if(other) {
                other->set_dirty(true);
            }
        } else if(voxel_pos.z >= 14) {
            VoxelData *other = get_chunk(chunk_pos + Vector3i(0, 0, 1));
            if(other) {
                other->set_dirty(true);
            }
        }
    }
}


uint8_t VoxelTerrain::get_voxel(Vector3i position) const {
    Vector3i voxel_pos = position;
    Vector3i chunk_pos(0, 0, 0);
    // Bring chunk and voxel position into the correct range.
    while(voxel_pos.x < 0) {
        voxel_pos.x += 16;
        chunk_pos.x -= 1;
    }
    while(voxel_pos.x >= 16) {
        voxel_pos.x -= 16;
        chunk_pos.x += 1;
    }
    while(voxel_pos.y < 0) {
        voxel_pos.y += 16;
        chunk_pos.y -= 1;
    }
    while(voxel_pos.y >= 16) {
        voxel_pos.y -= 16;
        chunk_pos.y += 1;
    }
    while(voxel_pos.z < 0) {
        voxel_pos.z += 16;
        chunk_pos.z -= 1;
    }
    while(voxel_pos.z >= 16) {
        voxel_pos.z -= 16;
        chunk_pos.z += 1;
    }
    VoxelData *data = get_chunk(chunk_pos);
    if(data) {
        return data->get_voxel(voxel_pos);
    } else {
        return 0;
    }
}

float VoxelTerrain::get_voxel_float(Vector3i position) const {
    return ((float)get_voxel(position) - 127.5) / 128.0; // This normalizes it in such a way that it should NEVER be fully 0, -1, or 1.
}

VoxelData *VoxelTerrain::get_chunk(Vector3i position) const {
    if(!chunks.has(position)) {
        return nullptr; // No chunk at position.
    }
    Variant var = chunks[position];
    if(var.get_type() == Variant::OBJECT) {
        Object *obj = static_cast<Object*>(var);
        if(obj->get_class() == "VoxelData") {
            VoxelData *data = static_cast<VoxelData*>(obj);
            return data;
        }
    }

    UtilityFunctions::print("Attempted to get a chunk, but it was the wrong type. What are you doing?");
    return nullptr; // Wrong type at position. This is very bad.
}

void VoxelTerrain::remove_chunk(Vector3i position) {
    if(chunks.has(position)) {
        chunks.erase(position);
    }
    if(meshes.has(position)) {
        meshes[position].call("queue_free");
        meshes.erase(position);
    }
}
