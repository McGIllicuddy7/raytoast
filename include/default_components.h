#pragma once
#include <raylib.h>
#include "utils.h"
enable_option_type(u32);
typedef struct{
    Transform transform;
    Optionu32 parent;
    u32Vec children;
}TransformComp;
enable_option_type(TransformComp);
enable_vec_type(OptionTransformComp);

bool set_transform_comp(u32 id, TransformComp trans);
TransformComp * get_transform_comp(u32 id);
bool remove_transform_comp(u32 id);

typedef struct{
    u32 shader_id;
    u32 model_id;
    u32 anim_id;
    u32 anim_frame;
    u32 diffuse_texture_id;
    u32 normal_texture_id;
    u32 roughness_texture_id;
    u32 emmisive_texture_id;
    Color tint;
    float roughness;
    Color emmision;
}ModelComp;
enable_option_type(ModelComp);
enable_vec_type(OptionModelComp);

bool set_model_comp(u32 id, ModelComp model);
ModelComp * get_model_comp(u32 id);
bool remove_model_comp(u32 id);

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

bool set_physics_comp(u32 id, PhysicsComp phys);
PhysicsComp * get_physics_comp(u32 id);
bool remove_physics_comp(u32 id);

bool transform_set_contains_child(u32 root, u32 needle);
u32 get_root(u32 base);
void attach_to(u32 entity, u32 parent);
void detach(u32 entity);

typedef struct{
    Quaternion control_rotation;
    Vector3 desired_velocity;
}CharacterComp;
enable_option_type(CharacterComp)

bool set_character_comp(u32 id, CharacterComp character_comp);
CharacterComp * get_character_comp(u32 id);
bool remove_character_comp(u32 id);

enable_vec_type(OptionCharacterComp);
typedef struct{
    Color color;
    float brightness;
    float influence_radius;
}LightComp;
enable_option_type(LightComp);
enable_vec_type(OptionLightComp);

bool set_light_comp(u32 id, LightComp light);
LightComp* get_light_comp(u32 id);
bool remove_light_comp(u32 id);

u32 create_light(Vector3 location, Color color, float brightness, float radius);