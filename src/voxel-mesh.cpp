#include <map>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/concave_polygon_shape3d.hpp>

#include "voxel-terrain.hpp"
#include "voxel-mesh.hpp"

void VoxelMesh::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_chunk_position", "chunk_position"), &VoxelMesh::set_chunk_position);
    ClassDB::bind_method(D_METHOD("get_chunk_position"), &VoxelMesh::get_chunk_position);

    ClassDB::bind_method(D_METHOD("update_mesh", "terrain"), &VoxelMesh::update_mesh);
    ClassDB::bind_method(D_METHOD("set_mesh_material", "material"), &VoxelMesh::set_mesh_material);
    ClassDB::bind_method(D_METHOD("update_collision"), &VoxelMesh::update_collision);
    ClassDB::bind_method(D_METHOD("is_update_ready"), &VoxelMesh::is_update_ready);
}


VoxelMesh::VoxelMesh() {
    collision_generated = true;
    col_shape = nullptr;
    array_mesh = nullptr;
}

VoxelMesh::~VoxelMesh() {
}


void VoxelMesh::_ready() {
    
}


void VoxelMesh::set_chunk_position(Vector3i chunK_position) {
    this->chunk_position = chunK_position;
    set_position(static_cast<Vector3>(chunK_position * 16));
}

Vector3i VoxelMesh::get_chunk_position() const {
    return chunk_position;
}


Vector3 interp_vert(Vector3 p0, Vector3 p1, float valp0, float valp1) {
    float mu = (-valp0) / (valp1 - valp0);
    return p0 + mu * (p1 - p0);
}

void add_quad(PackedInt32Array &indices, int32_t a, int32_t b, int32_t c, int32_t d) {
    indices.append_array({b, a, c, b, c, d});
}


/// NOTE: Much of this algorithm is directly pulled from my past attempts using GDScript
/// It's more of a port than anything, so if it's bad, well... RIP I guess.
void VoxelMesh::update_mesh(VoxelTerrain *terrain) {
    collision_generated = false;
    // Edge pairs that can be checked.
    const int32_t E0[12] = {0, 2, 4, 6, 0, 1, 4, 5, 0, 1, 2, 3};
    const int32_t E1[12] = {1, 3, 5, 7, 2, 3, 6, 7, 4, 5, 6, 7};

    // The directions of each corner.
    const Vector3 CORNER_DIRS[8] = {
        Vector3(-1.0, -1.0, -1.0),
        Vector3(1.0, -1.0, -1.0),
        Vector3(-1.0, 1.0, -1.0),
        Vector3(1.0, 1.0, -1.0),
        Vector3(-1.0, -1.0, 1.0),
        Vector3(1.0, -1.0, 1.0),
        Vector3(-1.0, 1.0, 1.0),
        Vector3(1.0, 1.0, 1.0),
    };

    // Mesh and collision data.
    PackedVector3Array vertices = PackedVector3Array();
    PackedVector3Array normals = PackedVector3Array();
    PackedInt32Array indices = PackedInt32Array();

    // The index of each cell is stored in here.
    std::map<Vector3i, uint16_t> cell_indices = std::map<Vector3i, uint16_t>();
    Vector3i chunk_offset = chunk_position * 16;

    // Currently, we go from 0 to 16. Once I implement seams, this will be 0 to 15.
    for (int z = 0; z <= 16; z++) {
        for (int y = 0; y <= 16; y++) {
            for (int x = 0; x <= 16; x++) {
                Vector3i ind = Vector3i(x, y, z);
                // Cache the corners of the current voxel.
                Vector3i corner_pos[8] = {
                    ind,
                    ind + Vector3i(1, 0, 0),
                    ind + Vector3i(0, 1, 0),
                    ind + Vector3i(1, 1, 0),
                    ind + Vector3i(0, 0, 1),
                    ind + Vector3i(1, 0, 1),
                    ind + Vector3i(0, 1, 1),
                    ind + Vector3i(1, 1, 1),
                };
                Vector3 cell_norm = Vector3(0.0, 0.0, 0.0);
                bool has_negative = false;
                bool has_positive = false;

                // Store the corners of the current voxel.
                float corners[8];
                for(int i = 0; i < 8; i++) {
                    // Once seams are implemented, this should be doable with just the chunk and corner_pos[i].
                    // Should be a significant performance bump.
                    corners[i] = terrain->get_voxel_float(chunk_offset + corner_pos[i]);
                    // Offset the calculated normal by the voxel value at this corner.
                    cell_norm += CORNER_DIRS[i] * -corners[i];
                    if(corners[i] < 0.0) {
                        has_negative = true;
                    } else if(corners[i] > 0.0) {
                        has_positive = true;
                    }
                }
                cell_norm.normalize();

                // Only run on the 0-crossing (easily calculated if there are both positive and negative values bordering this cell)
                if(has_positive && has_negative) {
                    Vector3 vert_pos = Vector3(0.0, 0.0, 0.0);
                    float pos_weight = 0.0;
                    for(int i = 0; i < 12; i++) {
                        float e0 = corners[E0[i]];
                        float e1 = corners[E1[i]];
                        // If the sign is different, this edge lies along a 0-crossing and should affect the vertex.
                        if(Math::sign(e0) != Math::sign(e1)) {
                            vert_pos += interp_vert(corner_pos[E0[i]], corner_pos[E1[i]], e0, e1);
                            pos_weight += 1;
                        }
                    }
                    vert_pos /= pos_weight;
                    //vert_pos = vert_pos.clamp(corner_pos[0], corner_pos[7]); // I don't think this is necessary, but it was here so I'm leaving it just in case I need it.
                    
                    // A chunk will only have up to 4913 (because we don't have seams) cells in the worst case.
                    // Because of this, we can assume that we only need 13 bits to store the voxel index.
                    // Therefore, we can use a 16-bit unsigned integer with the first 3 bits indicating 0-crossings per-axis.

                    uint16_t index = vertices.size(); // Bits 0-12 contain the index.
                    if(Math::sign(corners[0]) != Math::sign(corners[1])) {
                        index |= 1 << 13; // Bit 13 indicates that we need an x face.
                    }
                    if(Math::sign(corners[0]) != Math::sign(corners[2])) {
                        index |= 1 << 14; // Bit 13 indicates that we need a y face.
                    }
                    if(Math::sign(corners[0]) != Math::sign(corners[4])) {
                        index |= 1 << 15; // Bit 15 indicates that we need a z face.
                    }
                    cell_indices.insert_or_assign(ind, index);

                    vertices.push_back(vert_pos);
                    normals.push_back(cell_norm);
                }
            }
        }
    } // End of 3D for loop

    for(std::pair<Vector3i, uint16_t> k_v : cell_indices) {
        const uint16_t index_mask = 0b0001111111111111;
        Vector3i p = k_v.first;
        
        // Skip anything along the bottom, left, and back faces of the chunk.
        if(p.x == 0 || p.y == 0 || p.z == 0) {
            continue;
        }

        uint16_t flags = k_v.second >> 13;

        // With seams, we could get down to a max value of 4095. Then, we could store the sign of this in another flag.
        // For now, this will have to do.
        uint8_t p_v = terrain->get_voxel(chunk_offset + p);
        
        int a = cell_indices[p] & index_mask;
        int b, c, d;

        // Place a face on any axis relative to the current cell that has a 0-crossing.
        if(flags & 1) {
            // Place an x face
            b = cell_indices[p - Vector3i(0, 1, 0)] & index_mask;
            c = cell_indices[p - Vector3i(0, 0, 1)] & index_mask;
            d = cell_indices[p - Vector3i(0, 1, 1)] & index_mask;
            if(p_v > 127) {
                add_quad(indices, a, b, c, d);
            } else {
                add_quad(indices, a, c, b, d);
            }
        }
        if(flags & 2) {
            b = cell_indices[p - Vector3i(0, 0, 1)] & index_mask;
            c = cell_indices[p - Vector3i(1, 0, 0)] & index_mask;
            d = cell_indices[p - Vector3i(1, 0, 1)] & index_mask;
            // Place a y face
            if(p_v > 127) {
                add_quad(indices, a, b, c, d);
            } else {
                add_quad(indices, a, c, b, d);
            }
        }
        if(flags & 4) {
            b = cell_indices[p - Vector3i(1, 0, 0)] & index_mask;
            c = cell_indices[p - Vector3i(0, 1, 0)] & index_mask;
            d = cell_indices[p - Vector3i(1, 1, 0)] & index_mask;
            // Place a z face
            if(p_v > 127) {
                add_quad(indices, a, b, c, d);
            } else {
                add_quad(indices, a, c, b, d);
            }
        }
    } // End of indexing loop

    if(indices.size() >= 3) {
        // Create our mesh.
        Array surface_arrays;
        surface_arrays.resize(ArrayMesh::ARRAY_MAX);

        surface_arrays[ArrayMesh::ARRAY_VERTEX] = vertices;
        surface_arrays[ArrayMesh::ARRAY_NORMAL] = normals;
        surface_arrays[ArrayMesh::ARRAY_INDEX] = indices;

        array_mesh = memnew(ArrayMesh);
        array_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, surface_arrays);
        call_deferred("set_mesh", array_mesh);

        PackedVector3Array collision_faces = PackedVector3Array();
        collision_faces.resize(indices.size());

        // Generate our collision faces.
        for(int i = 0; i < indices.size(); i++) {
            collision_faces[i] = vertices[indices[i]];
        }
        update_collision(collision_faces);
        //call_deferred("update_collision", collision_faces);
    }

    set_mesh_material(terrain->get_terrain_material());
}


void VoxelMesh::set_mesh_material(Ref<Material> material) {
    set_material_override(material);
}


void VoxelMesh::update_collision(PackedVector3Array collision_faces) {
    if(collision_generated) {
        return;
    }
    if(!col_shape) {
        StaticBody3D *body = memnew(StaticBody3D);
        col_shape = memnew(CollisionShape3D);
        trimesh_shape = memnew(ConcavePolygonShape3D);
        call_deferred("add_child", body);
        //add_child(body);
        body->call_deferred("add_child", col_shape);
        //body->add_child(col_shape);
        col_shape->call_deferred("set_shape", trimesh_shape);
    }

    trimesh_shape->call_deferred("set_faces", collision_faces);
    //trimesh_shape->set_faces(collision_faces);
    //col_shape->set_shape(trimesh_shape);
    collision_generated = true;
}


bool VoxelMesh::is_update_ready() {
    return this->collision_generated;
}
