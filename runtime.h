#pragma once
#include <stdlib.h>
#include "utils.h"
#include <raylib.h>
#include "runtime/default_components.h"
#include "resource.h"
#include <pthread.h>
#define true 1 
#define false 0
enable_option_type(u32);

typedef struct {
    void (*on_tick)(void * self, f32 delta_time);
    void (*on_render)(void * self);
    void (*on_setup)(void* self,u32 self_id);
    void (*destructor)(void *self);
}EntityVTable;
#ifndef RUNTIME_MOD
extern EntityVTable entity_default_vtable;
#endif
typedef struct Entity{
    EntityVTable * vtable;
    u32 self_id;
}Entity;
typedef void (*Event)(void*self, void * args);
typedef struct EventNode{
    Event event;
    void * args;
    u32 entity_id;
    struct EventNode * next;
}EventNode;
void default_on_tick(void *f, f32 dt);
void default_on_setup(void*self ,u32 self_id);
void default_on_render(void * self);
void default_on_destroy(void * self);
typedef Entity * EntityRef;
enable_vec_type(EntityRef);
enable_option_type(Model);
enable_option_type(Shader);
enable_vec_type(OptionShader);
enable_vec_type(Model);
enable_vec_type(OptionModel);
enable_resource_type(Model);
enable_resource_type(Shader);

enable_hash_type(String, u32);
typedef struct {
    EntityRefVec entities;
    ResourceModel models;
    ResourceShader shaders; 
    OptionTransformCompVec transform_comps;
    OptionPhysicsCompVec physics_comps;
    OptionModelCompVec model_comps;
    Stringu32HashTable* loaded_models;
    Stringu32HashTable* loaded_shaders;
    Camera3D camera;
    RenderTexture2D target;
    bool failed_to_create;
    EventNode * event_queue;
}Runtime;

#ifndef RUNTIME_MOD
extern Runtime RT;
#endif
void init_runtime(void (*setup)(), void(*on_tick)(), void (*on_render)());
Optionu32 create_entity(Entity * ent);
bool destroy_entity(u32 id);
Entity * get_entity(u32 id);

bool set_transform_comp(u32 id, TransformComp trans);
TransformComp * get_transform_comp(u32 id);
bool remove_transform_comp(u32 id);

bool set_physics_comp(u32 id, PhysicsComp phys);
PhysicsComp * get_physics_comp(u32 id);
bool remove_physics_comp(u32 id);

bool set_model_comp(u32 id, ModelComp model);
ModelComp * get_model_comp(u32 id);
bool remove_model_comp(u32 id);

u32 create_shader(Shader shader);
OptionShader get_shader(u32 id);
bool remove_shader(u32 id);

u32 create_model(Model model);
OptionModel get_model(u32 id);
bool remove_model(u32 id);

Camera3D * get_camera();

void add_force(u32 id, Vector3 force);

void call_event(u32 id, void (*func)(void* self, void * args), void * args);

u32 load_shader(const char * vertex_path, const char *frag_path);
void unload_shader(u32 id);

u32 load_model(const char * path);
void unload_model(u32 id);