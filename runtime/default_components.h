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
    bool movable;
    bool can_bounce;
    bool collided_this_frame;
}PhysicsComp;
enable_option_type(PhysicsComp);
enable_vec_type(OptionPhysicsComp);

bool transform_set_contains_child(u32 root, u32 needle);
u32 get_root(u32 base);
void attach_to(u32 entity, u32 parent);
void detach(u32 entity);