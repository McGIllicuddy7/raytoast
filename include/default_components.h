#pragma once
#include <raylib.h>
#include "utils.h"
typedef struct{
    u32 id;
    u32 gen_idx;
}Ref;
enable_option_type(Ref);
enable_vec_type(Ref);
typedef struct{
    Transform transform;
    OptionRef parent;
    RefVec children;
}TransformComp;
enable_option_type(TransformComp);
enable_vec_type(OptionTransformComp);

bool set_transform_comp(Ref id, TransformComp trans);
TransformComp * get_transform_comp(Ref id);
bool remove_transform_comp(Ref id);

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
    Transform relative_transform;
}ModelComp;
enable_option_type(ModelComp);
enable_vec_type(OptionModelComp);

bool set_model_comp(Ref id, ModelComp model);
ModelComp * get_model_comp(Ref id);
bool remove_model_comp(Ref id);

typedef struct{
    BoundingBox box;
    Vector3 velocity;
    float mass;
    bool movable;
    bool can_bounce;
    bool collided_this_frame;
    bool only_overlap;
}PhysicsComp;
enable_option_type(PhysicsComp);
enable_vec_type(OptionPhysicsComp);

bool set_physics_comp(Ref id, PhysicsComp phys);
PhysicsComp * get_physics_comp(Ref id);
bool remove_physics_comp(Ref id);
void add_force(Ref id, Vector3 force);

bool transform_set_contains_child(Ref root, Ref needle);
Ref get_root(Ref base);
void attach_to(Ref entity, Ref parent);
void detach(Ref entity);

typedef struct{
    Quaternion control_rotation;
    Vector3 desired_velocity;
}CharacterComp;
enable_option_type(CharacterComp)

bool set_character_comp(Ref id, CharacterComp character_comp);
CharacterComp * get_character_comp(Ref id);
bool remove_character_comp(Ref id);

enable_vec_type(OptionCharacterComp);
typedef struct{
    Color color;
    float brightness;
    float influence_radius;
}LightComp;
enable_option_type(LightComp);
enable_vec_type(OptionLightComp);

bool set_light_comp(Ref id, LightComp light);
LightComp* get_light_comp(Ref id);
bool remove_light_comp(Ref id);

Ref create_light(Vector3 location, Color color, float brightness, float radius);

typedef struct{
    voidVec vec;
    void (*destructor)(void *);
    void (*reserve_fn)(voidVec, size_t);
    size_t size;
}GenericComponent;
enable_hash_type(cstr,GenericComponent);
typedef void (*VoidFN)(void);
enable_vec_type(VoidFN);
enable_hash_type(cstr, VoidFN);
typedef struct{
    cstrGenericComponentHashTable * table;
    cstrVoidFNHashTable * systems;
    cstrVoidFNHashTable * graphics_systems;
    cstrVoidFNHashTable * drawing_systems;
}GenericComponents;
void unload_gen_comp(GenericComponent * cmp);
void unload_gen_comps(GenericComponents cmps);
#define enable_component(T, comp_name, destructor)\
bool set_##comp_name##_comp(Ref id, T comp_name);\
T* get_##comp_name##_comp(Ref id);\
bool remove_##comp_name##_comp(Ref id);\
Option##T##Vec * get_##comp_name##_comps();\
void register_##comp_name##_components();\

#define enable_component_impl(T, comp_name, destructor)\
bool set_##comp_name##_comp(Ref id, T comp){\
    if(id.id>=RT.entities.length){\
        return false;\
    }\
    if(!RT.entities.items[id.id]){\
        return false\
    }\
    if(RT.generations.items[id.id] != id.gen_idx){\
        return 0;\
    }\
    Option##TVec * v = get_##comp_name##_comps();\
    if(!v) return false;\
    if(v->items[id.id].is_valid){\
        destructor(&v->items[id.id].value);\
    }\
    v->items[id.id] = (Option##T)Some(comp);\
    return true;\
}\
T* get_##comp_name##_comp(Ref id){\
    if(id.id>=RT.entities.length){\
        return 0\
    }\
    if(!RT.entities.items[id.id]){\
        return 0;\
    }\
    if(RT.generations.items[id.id] != id.gen_idx){\
        return 0;\
    }\
    Option##TVec * v = get_##comp_name##_comps();\
    if(v->items[id.id].is_valid){\
        return &v->items[id.id].value;\
    }\
}\
bool remove_##comp_name##_comp(Ref id){\
    if(id>=RT.entities.length) return false;\
    if(!RT.entities.items[id.id]) return false;\
    Option##TVec * v = get_##comp_name##_comps();\
    if(RT.generations.items[id.id] != id.gen_idx){\
        return 0;\
    }\
    if(v->items[id.id].is_valid){\
        destructor(&v->items[id.id].value);\
    }\
    v->items[id.id] = (Option##T)None;\
    return true;\
}\
Option##T##Vec * get_##comp_name##_comps(){\
    return (Option##T##Vec *)(void*)(&(cstrGenericComponentHashTable_find(RT.gen_comps.table. STRINGIFY(T))->vec));\
}\
void T##reserve_fn(Option##T##Vec * v, size_t count){\
    for(int i=0; i<count; i++){\
        v_append((*v), (Option##T)None);\
    }\
}\
void T##destructor_fn(Option##TVec * v){\
    for(int i =0; i<v->length; i++){\
        destructor(&v->items[i]);\
    }\
}\
void register_##comp_name##_components(){\
    if(cstrGenericComponentHashTable_find(RT.gen_comps, STRINGIFY(T))){return;}\
    else{\
        Option##T##Vec v = make(0, Option##T);\
        cstrGenericComponentHashTable_insert(STRINGIFY(T), v);\
    }\
}\

void register_system(char * name, void (*fn)());
void deregister_system(char * name);

void register_graphics_system(char * name, void(*fn)());
void deregister_graphics_system(char * name);

void register_drawing_system(char * name, void (*fn)());
void deregister_drawing_system(char * name);