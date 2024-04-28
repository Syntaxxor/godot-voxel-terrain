#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include "voxel-data.hpp"

void VoxelData::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_voxel", "position", "value"), &VoxelData::set_voxel);
    ClassDB::bind_method(D_METHOD("get_voxel", "position"), &VoxelData::get_voxel);
    
    ClassDB::bind_method(D_METHOD("set_voxels", "voxels"), &VoxelData::set_voxels);
    ClassDB::bind_method(D_METHOD("get_voxels"), &VoxelData::get_voxels);

    ClassDB::bind_method(D_METHOD("is_in_range", "position"), &VoxelData::is_in_range);
    ClassDB::bind_method(D_METHOD("is_empty"), &VoxelData::is_empty);

    ADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "voxels"), "set_voxels", "get_voxels");
}


VoxelData::VoxelData() {
    voxels = PackedByteArray();
    voxels.resize(4096);
    for(int i = 0; i < 4096; i++) {
        voxels.set(i, 0);
    }
    dirty = true;
}

VoxelData::~VoxelData() {
}


void VoxelData::set_voxels(PackedByteArray voxels) {
    this->voxels = voxels;
}


PackedByteArray VoxelData::get_voxels() const {
    return voxels;
}


void VoxelData::set_dirty(bool dirty) {
    this->dirty = dirty;
}

bool VoxelData::get_dirty() const {
    return dirty;
}


void VoxelData::set_voxel(Vector3i position, uint8_t value) {
    if (is_in_range(position)) {
        voxels.set(position.z * 256 + position.y * 16 + position.x, value);
        dirty = true;
    } else {
        UtilityFunctions::print("Voxel index out of range!");
    }
}

uint8_t VoxelData::get_voxel(Vector3i position) const {
    if (is_in_range(position)) {
        return voxels[position.z * 256 + position.y * 16 + position.x];
    } else {
        UtilityFunctions::print("Voxel index out of range!");
        return 0;
    }
}


bool VoxelData::is_in_range(Vector3i position) const {
    return position.x >= 0 && position.x < 16 && position.y >= 0 && position.y < 16 && position.z >= 0 && position.z < 16;
}

bool VoxelData::is_empty() const {
    for(int i = 0; i < 4096; i++) {
        if(voxels[i] != 0) {
            return false;
        }
    }
    return true;
}
