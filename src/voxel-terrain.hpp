#ifndef VOXEL_TERRAIN_H
#define VOXEL_TERRAIN_H

#include <gdextension_interface.h>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/material.hpp>



using namespace godot;

class VoxelData;

class VoxelTerrain : public Node3D {
    GDCLASS(VoxelTerrain, Node3D);

    private:
    Dictionary chunks;
    Dictionary meshes;
    Ref<Material> terrain_material;
    bool dirty;
    
    protected:
    static void _bind_methods();

    public:
    VoxelTerrain();
    ~VoxelTerrain();

    void _ready() override;
    void _process(double delta) override;

    void update_terrain();

    /// Set the current chunks dictionary.
    void set_chunks(Dictionary chunks);
    /// Get the current chunks dictionary.
    Dictionary get_chunks() const;


    void set_terrain_material(const Ref<Material> terrain_material);
    Ref<Material> get_terrain_material() const;
    

    /// Set the voxel at the specified position.
    void set_voxel(Vector3i position, uint8_t value);
    /// Get the voxel at the specified position.
    uint8_t get_voxel(Vector3i position) const;
    /// Get the voxel at the specified position and convert to float.
    float get_voxel_float(Vector3i position) const;

    /// Get a chunk at the given location.
    /// Returns nullptr if there isn't a chunk,
    /// or if the item there in the dictionary
    /// isn't a VoxelData object.
    VoxelData *get_chunk(Vector3i position) const;
    /// Removes the chunk at the given location.
    /// Also removes any meshes at that location.
    /// Will not mark surroundings as dirty.
    void remove_chunk(Vector3i position);
};

#endif
