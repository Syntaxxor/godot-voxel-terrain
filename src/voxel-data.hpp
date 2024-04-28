#ifndef VOXEL_DATA_H
#define VOXEL_DATA_H

#include <gdextension_interface.h>

#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

using namespace godot;

/// A 16x16x16 voxel chunk
class VoxelData : public Resource {
    GDCLASS(VoxelData, Resource);

    private:
    PackedByteArray voxels;
    bool dirty;

    protected:
    static void _bind_methods();

    public:
    VoxelData();
    ~VoxelData();

    /// Set the current array of voxels in this chunk.
    void set_voxels(PackedByteArray voxels);
    /// Get the current array of voxels in this chunk.
    PackedByteArray get_voxels() const;

    /// Set the voxel at the specified position.
    void set_voxel(Vector3i position, uint8_t value);
    /// Get the voxel at the specified position.
    uint8_t get_voxel(Vector3i position) const;

    void set_dirty(bool dirty);
    bool get_dirty() const;

    /// Check if a voxel position is in the chunk.
    bool is_in_range(Vector3i position) const;
    /// Check if all the voxels in the chunk are 0.
    bool is_empty() const;
};

#endif