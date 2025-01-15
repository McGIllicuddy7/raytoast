#pragma once
#include <stdlib.h>
#include "utils.h"
#include <raylib.h>
#include "default_components.h"
#include "resource.h"
#include <pthread.h>
#define true 1 
#define false 0


typedef struct {
    void (*on_tick)(void * self, f32 delta_time);
    void (*on_render)(void * self);
    void (*on_setup)(void* self,Ref self_id);
    void (*destructor)(void *self);
    ByteVec (*serialize)(void *self);
}EntityVTable;
#ifndef RUNTIME_MOD
extern EntityVTable entity_default_vtable;
#endif
typedef struct Entity{
    EntityVTable * vtable;
    Ref self_id;
}Entity;
typedef void (*Event)(void*self, void * args);
typedef struct EventNode{
    Event event;
    void * args;
    Ref entity_id;
    struct EventNode * next;
}EventNode;
void default_on_tick(void *f, f32 dt);
void default_on_setup(void*self ,Ref self_id);
void default_on_render(void * self);
void default_on_destroy(void * self);
void default_serialize(void * self);
typedef Entity * EntityRef;
enable_vec_type(EntityRef);
enable_option_type(Model);
enable_option_type(Shader);
enable_option_type(Texture);
enable_vec_type(OptionShader);
enable_vec_type(Model);
enable_vec_type(OptionModel);
enable_vec_type(OptionTexture);
enable_resource_type(Model);
enable_resource_type(Shader);
enable_resource_type(Texture);
enable_hash_type(String, u32);

typedef struct {
    EntityRefVec entities;
    u32Vec generations;
    ResourceModel models;
    ResourceShader shaders; 
    ResourceTexture textures;
    OptionTransformCompVec transform_comps;
    OptionPhysicsCompVec physics_comps;
    OptionModelCompVec model_comps;
    OptionCharacterCompVec character_comps;
    OptionLightCompVec light_comps;
    GenericComponents gen_comps;
    Vector3 directional_light_direction;
    Color directional_light_color;
    Color ambient_color;
    Stringu32HashTable* loaded_models;
    Stringu32HashTable* loaded_shaders;
    Stringu32HashTable* loaded_textures;
    Camera3D camera;
    OptionRef camera_parent;
    Transform camera_relative_transform;
    RenderTexture2D target;
    bool failed_to_create;
    EventNode * event_queue;
    bool paused;
}Runtime;

#ifndef RUNTIME_MOD
extern Runtime RT;
#endif
void init_runtime(void (*setup)(), void(*on_tick)(), void (*on_render)());
OptionRef create_entity(Entity * ent);
bool destroy_entity(Ref id);
Entity * get_entity(Ref id);



u32 create_shader(Shader shader);
OptionShader get_shader(u32 id);
bool remove_shader(u32 id);

u32 create_texture(Texture texture);
OptionTexture get_texture(u32 id);
bool remove_texture(u32 id);

u32 create_model(Model model);
OptionModel get_model(u32 id);
bool remove_model(u32 id);


void call_event(Ref id, void (*func)(void* self, void * args), void * args);

u32 load_shader(const char * vertex_path, const char *frag_path);
String* get_shader_name(u32 id);
void unload_shader(u32 id);

u32 load_model(const char * path);
String* get_model_name(u32 id);
void unload_model(u32 id);

u32 load_texture(const char * path);
String* get_texture_name(u32 id);
void unload_texture(u32 id);

void unload_level();

Vector3 get_location(Ref id);
Quaternion get_rotation(Ref id);
Vector3 get_scale(Ref id);

Vector3 get_forward_vector(Ref id);
Vector3 get_up_vector(Ref id);
Vector3 get_right_vector(Ref id);

Camera3D * get_camera();
void attach_camera_to(Ref id, Transform relative_trans);
void detach_camera();

Vector3 * get_light_direction();
Color* get_light_color();
Color * get_ambient_color();

Transform transform_default();
Quaternion quat_from_vector(Vector3 location);
void load_level(const char * path);
void save_level(const char * path);