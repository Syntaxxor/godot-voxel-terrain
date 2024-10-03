#ifndef TERRAIN_CONTROL_H
#define TERRAIN_CONTROL_H

#include <gdextension_interface.h>
#include <godot_cpp/classes/control.hpp>

#include "voxel-terrain.hpp"


using namespace godot;


const uint32_t SHAPE_SPHERE = 0;
const uint32_t SHAPE_BOX = 1;
const uint32_t SHAPE_CYLINDER = 2;

const uint32_t TOOL_DRAW = 0;
const uint32_t TOOL_SCULPT = 1;
const uint32_t TOOL_SMOOTH = 2;


class TerrainControl : public Control {
    GDCLASS(TerrainControl, Control)

    private:
    VoxelTerrain *terrain;
    uint32_t edit_shape;
    uint32_t edit_tool;
    Vector3 target_location;
    float size;
    float strength;
    float default_dist;



    /// Edit tools
    void edit_draw(Vector3i location, float weight);
    void edit_sculpt(Vector3i location, float weight);
    void edit_smooth(Vector3i location, float weight);

    protected:
    static void _bind_methods();

    public:
    TerrainControl();
    ~TerrainControl();

    /// Set the currently edited terrain.
    void set_terrain(VoxelTerrain *terrain);
    /// Get the currently edited terrain.
    VoxelTerrain *get_terrain();

    // Set the shape of the editing brush.
    void set_edit_shape(uint32_t edit_shape);
    // Get the shape of the editing brush.
    uint32_t get_edit_shape() const;

    // Set the editing tool.
    void set_edit_tool(uint32_t edit_tool);
    // Get the shape of the editing brush.
    uint32_t get_edit_tool() const;

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

    /// Get the weight at a specific location according to the current edit shape.
    float get_weight_at(Vector3i off);
    /// Apply editing at the cursor.
    void apply_edit();
};

#endif