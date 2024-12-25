#pragma once
#include <stdlib.h>
#include "utils.h"
#include <raylib.h>
#include "runtime/default_components.h"
#include "resource.h"
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
    u32 __dont_touch;
}Entity;
void default_on_tick(void *f, f32 dt);
void default_on_setup(void*self ,u32 self_id);
void default_on_render(void * self);
void default_on_destroy(void * self);
typedef Entity * EntityRef;
enable_vec_type(EntityRef);
enable_option_type(Mesh);
enable_option_type(Shader);
enable_vec_type(OptionShader);
enable_vec_type(Mesh);
enable_vec_type(OptionMesh);
enable_resource_type(Mesh);
enable_resource_type(Shader);

typedef struct {
    EntityRefVec entities;
    ResourceMesh meshes;
    ResourceShader shaders; 
    OptionTransformCompVec transform_comps;
    OptionPhysicsCompVec physics_comps;
    OptionMeshCompVec mesh_comps;
    Camera3D camera;
    bool failed_to_create;
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

bool set_mesh_comp(u32 id, MeshComp mesh);
MeshComp * get_mesh_comp(u32 id);
bool remove_mesh_comp(u32 id);

u32 create_shader(Shader shader);
OptionShader get_shader(u32 id);
bool remove_shader(u32 id);

u32 create_mesh(Mesh mesh);
OptionMesh get_mesh(u32 id);
bool remove_mesh(u32 id);

Camera3D * get_camera();

void add_force(u32 id, Vector3 force);