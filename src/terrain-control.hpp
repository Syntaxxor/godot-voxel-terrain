#ifndef TERRAIN_CONTROL_H
#define TERRAIN_CONTROL_H

#include <gdextension_interface.h>
#include <godot_cpp/classes/control.hpp>

#include "voxel-terrain.hpp"


using namespace godot;


const uint32_t SHAPE_SPHERE = 0;
const uint32_t SHAPE_BOX = 1;

enum EditShape {
    SPHERE = SHAPE_SPHERE,
    BOX = SHAPE_BOX
};


class TerrainControl : public Control {
    GDCLASS(TerrainControl, Control)

    private:
    VoxelTerrain *terrain;
    uint32_t edit_shape;
    Vector3 target_location;
    float size;
    float strength;
    float default_dist;

    protected:
    static void _bind_methods();

    public:
    TerrainControl();
    ~TerrainControl();

    /// Set the currently edited terrain.
    void set_terrain(VoxelTerrain *terrain);
    /// Get the currently edited terrain.
    VoxelTerrain *get_terrain();

    void set_edit_shape(uint32_t edit_shape);
    uint32_t get_edit_shape() const;

    /// Set the location of the editing target.
    void set_target_location(Vector3 target_location);
    /// Get the location of the editing target.
    Vector3 get_target_location() const;

    /// Set the size of the current editing brush.
    void set_size(float size);
    /// Get the size of the current editing brush.
    float get_size() const;

    /// Set the size of the current editing brush.
    void set_strength(float strength);
    /// Get the size of the current editing brush.
    float get_strength() const;

    /// Update the location of the editing target.
    void update_target_location(Vector3 r0, Vector3 rd);
    float get_weight_at(Vector3i off);
    void apply_edit();
};

#endif