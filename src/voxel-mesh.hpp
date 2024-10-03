#ifndef VOXEL_MESH_H
#define VOXEL_MESH_H

#include <gdextension_interface.h>

#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/array_mesh.hpp>

using namespace godot;

class VoxelTerrain;

class VoxelMesh : public MeshInstance3D {
    GDCLASS(VoxelMesh, MeshInstance3D);

    private:
    ArrayMesh *array_mesh;
    Vector3i chunk_position;
    bool collision_generated;
    CollisionShape3D *col_shape;
    ConcavePolygonShape3D *trimesh_shape;

    protected:
    static void _bind_methods();

    public:
    VoxelMesh();
    ~VoxelMesh();

    void _ready() override;

    void set_chunk_position(Vector3i chunk_position);
    Vector3i get_chunk_position() const;

    void update_mesh(VoxelTerrain *terrain);
    void set_mesh_material(Ref<Material> material);
    void update_collision(PackedVector3Array collision_faces);
    bool is_update_ready();
};

#endif