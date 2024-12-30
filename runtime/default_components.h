#pragma once
#include <raylib.h>
#include "../utils.h"
enable_option_type(u32);
typedef struct{
    Transform transform;
    Optionu32 parent;
    u32Vec children;
}TransformComp;
enable_option_type(TransformComp);
enable_vec_type(OptionTransformComp);

typedef struct{
    u32 shader_id;
    u32 model_id;
}ModelComp;
enable_option_type(ModelComp);
enable_vec_type(OptionModelComp);

typedef struct{
    BoundingBox box;
    Vector3 velocity;
    float mass;
    bool collided_this_frame;
    bool movable;
    bool can_bounce;
}PhysicsComp;
enable_option_type(PhysicsComp);
enable_vec_type(OptionPhysicsComp);