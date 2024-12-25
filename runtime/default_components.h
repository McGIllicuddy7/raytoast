#pragma once
#include <raylib.h>

typedef struct{
    Transform transform;
}TransformComp;
enable_option_type(TransformComp);
enable_vec_type(OptionTransformComp);

typedef struct{
    u32 shader_id;
    u32 mesh_id;
}MeshComp;
enable_option_type(MeshComp);
enable_vec_type(OptionMeshComp);

typedef struct{
    BoundingBox box;
    Vector3 velocity;
    float mass;
    bool collided_this_frame;
    bool movable;
}PhysicsComp;
enable_option_type(PhysicsComp);
enable_vec_type(OptionPhysicsComp);