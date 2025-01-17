#include <raylib.h>
#define RUNTIME_MOD
#include "runtime.h"
#include <raymath.h>
#include <rlgl.h>
#define true 1
#define false 0
void run_physics();
void init_physics_rt();
void update_characters();
void finish_physics();
void draw_logging();
void run_drawing(Material mat, Shader default_post_process, void (*on_render)());
void default_on_tick(void *f, f32 dt){

}
void default_on_setup(void*self ,Ref self_id){
    Entity * ent = self;
    ent->self_id = self_id;
}
void default_on_render(void * self){

}
void default_on_destroy(void * self){

}
EntityVTable entity_default_vtable = 
{.on_tick = default_on_tick, 
.on_setup = default_on_setup, 
.destructor = default_on_destroy,
.on_render = default_on_render};
Runtime RT = {};
static void runtime_reserve(){
    for(int i =0; i<100; i++){
        v_append(RT.generations, 0);
        v_append(RT.entities, 0);
        v_append(RT.model_comps,(OptionModelComp) {});
        v_append(RT.transform_comps,(OptionTransformComp){});
        v_append(RT.physics_comps,(OptionPhysicsComp) {});
        v_append(RT.light_comps, (OptionLightComp){});
        v_append(RT.character_comps, (OptionCharacterComp){});
    }
}
static void process_events(){
    EventNode * current = RT.event_queue;
    while(current){
        Entity * ent = get_entity(current->entity_id);
        if(ent){
            current->event(ent, current->args);
        }
        current = current->next;
    }
}

void run_systems(){
    Iterator it = ITER_HASHTABLE(RT.gen_comps.systems);
    void (**fn)() = 0;
    while((fn = NEXT(it))){
        (*fn)();
    }
}

void setup_default_systems(){
    register_drawing_system("logging",draw_logging);
}
void setup_runtime(void (*setup)(), void(*on_tick)(), void (*on_render)()){
    tmp_reset();
    RT.entities = (EntityRefVec)make(0, EntityRef);
    RT.generations = make(0, u32);
    RT.models = ResourceModel_make(UnloadModel);
    RT.shaders = ResourceShader_make(UnloadShader);
    RT.model_comps =(OptionModelCompVec)make(0, OptionModelComp);
    RT.transform_comps =(OptionTransformCompVec)make(0, OptionTransformComp);
    RT.physics_comps =(OptionPhysicsCompVec)make(0, OptionPhysicsComp);
    RT.light_comps = (OptionLightCompVec)make(0, OptionLightComp);
    RT.character_comps = make(0, OptionCharacterComp);
    RT.loaded_models = Stringu32HashTable_create(1000, hash_string, string_equals, unmake_string, (void*)no_op_void);
    RT.loaded_shaders = Stringu32HashTable_create(1000, hash_string, string_equals, unmake_string, (void*)no_op_void); 
    RT.textures = ResourceTexture_make(UnloadTexture);
    RT.loaded_textures = Stringu32HashTable_create(1000, hash_string, string_equals, unmake_string,(void*)no_op_void);
    RT.ambient_color = (Color){64, 64, 64,128};
    RT.directional_light_color = (Color){128, 64, 58, 255};
    RT.directional_light_direction = (Vector3){0,0,-1};
    RT.gen_comps.systems = cstrVoidFNHashTable_create(100, (void*)hash_cstring, (void*)cstr_equals, (void*)no_op_void, (void*)no_op_void);
    RT.gen_comps.graphics_systems = cstrVoidFNHashTable_create(100, (void*)hash_cstring, (void*)cstr_equals, (void*)no_op_void, (void*)no_op_void);
    RT.gen_comps.drawing_systems = cstrVoidFNHashTable_create(100, (void*)hash_cstring, (void*)cstr_equals, (void*)no_op_void, (void*)no_op_void); 
    RT.gen_comps.table = cstrGenericComponentHashTable_create(100, (void*)hash_cstring, (void*)cstr_equals, (void*)no_op_void,unload_gen_comp);
    for(int i =0; i<100; i++){
        runtime_reserve();
    }
    SetTraceLogLevel(LOG_ALL);
    InitWindow(1600, 1024,"raytoast");
    SetTargetFPS(61);
    DisableCursor();
    init_physics_rt();
    RT.camera.fovy = 90.0;
    RT.camera.position = (Vector3){0.0, 0.0, 0.0};
    RT.camera.up = (Vector3){0.0,0.0, 1.0};
    RT.camera.target = (Vector3){1.0, 0.0, 0.0};
    RT.camera.projection = CAMERA_PERSPECTIVE;
    RT.target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    setup();
    RT.draw_calls =tmp_make(fn_void);
}

void init_runtime(void (*setup)(), void(*on_tick)(), void (*on_render)()){
    setup_runtime(setup, on_tick, on_render);
    setup_default_systems();
    Material mat = LoadMaterialDefault();
    Material svd_mat = mat;
    Shader default_post_process = LoadShader("shaders/base.vs", "shaders/default_post_process.fs");
    while(!WindowShouldClose()){
        RT.draw_calls =tmp_make(fn_void);
        if (RT.failed_to_create){
            runtime_reserve();
        }
        float ft = GetFrameTime();
        for (int i =0; i<RT.entities.length; i++){
            if (RT.entities.items[i]){
                RT.entities.items[i]->vtable->on_tick(RT.entities.items[i], ft);
            }
        }
        on_tick();
        run_systems();
        update_characters();
        run_physics();
        run_drawing(mat, default_post_process, on_render);
        finish_physics();
        process_events();
        tmp_reset();
    }
    UnloadMaterial(svd_mat);
    unload_level();
    CloseWindow();
}
static void free_string(String *s){
    arena_free(s->arena, s->items);
}
static void free_transform(OptionTransformComp t){
    unmake(t.value.children);
}
void unload_level(){
    for(int i =0; i<RT.entities.length; i++){
        if(RT.entities.items[i]){
            RT.entities.items[i]->vtable->destructor(RT.entities.items[i]);
            free(RT.entities.items[i]);
        }
    }
    unmake(RT.generations);
    unmake(RT.entities);
    ResourceModel_unmake(&RT.models);
    ResourceShader_unmake(&RT.shaders);
    ResourceTexture_unmake(&RT.textures);
    unmake_fn(RT.transform_comps, free_transform);
    unmake(RT.physics_comps);
    unmake(RT.model_comps);
    unmake(RT.character_comps);
    unmake(RT.light_comps);
    unload_gen_comps(RT.gen_comps);
    Stringu32HashTable_unmake(RT.loaded_models);
    Stringu32HashTable_unmake(RT.loaded_shaders);
    Stringu32HashTable_unmake(RT.loaded_textures);
    UnloadRenderTexture(RT.target);
    EventNode* queue = RT.event_queue;
    tmp_reset();
}
OptionRef create_entity(Entity * ent){
    for (int i=0; i<RT.entities.length; i++){
        if(!RT.entities.items[i]){
            RT.entities.items[i] = ent;
            RT.generations.items[i]++;
            Ref rt = {i, RT.generations.items[i]};
            ent->vtable->on_setup(ent,rt);
            return (OptionRef)Some(rt);
        }
    }
    RT.failed_to_create = true;
    return (OptionRef)None;
}
static void destroy_entity_actual(void * ptr){
    Ref * p = ptr;
    Ref id = *p;
    if(id.id> RT.entities.length){
        return;
    } 
    if(RT.entities.items[id.id]){
        RT.entities.items[id.id]->vtable->destructor(RT.entities.items[id.id]); 
        free(RT.entities.items[id.id]);
        RT.entities.items[id.id] = 0; 
        RT.character_comps.items[id.id] = (OptionCharacterComp)None;
        RT.model_comps.items[id.id] = (OptionModelComp)None;
        RT.transform_comps.items[id.id] = (OptionTransformComp)None;
        RT.light_comps.items[id.id] = (OptionLightComp)None;
        RT.physics_comps.items[id.id] = (OptionPhysicsComp)None;
        if(RT.camera_parent.is_valid&& RT.camera_parent.value.id == id.id){
            RT.camera_parent = (OptionRef)None;
        }
        return;
    } 
}
bool destroy_entity(Ref id){
    if(id.id> RT.entities.length){
        return false;
    } 
    if(RT.entities.items[id.id]){
        Ref * p = tmp_alloc(sizeof(Ref));
        *p = id;
        defer(&temporary_allocator, destroy_entity_actual, p);
    }
    return false;
}
Entity * get_entity(Ref id){
    if(id.id>= RT.entities.length){
        return 0;
    } 
    if(id.gen_idx != RT.generations.items[id.id]){
        return 0;
    }
    return RT.entities.items[id.id];
}

Camera3D * get_camera(){
    return &RT.camera;
}

Vector3 * get_light_direction(){
    return &RT.directional_light_direction;
}

Color* get_light_color(){
    return &RT.directional_light_color;
}

Color * get_ambient_color(){
    return &RT.ambient_color;
}
void call_event(Ref id, void (*func)(void * self, void * args), void * args){
    EventNode * node = tmp_alloc(sizeof(EventNode));
    node->args = args;
    node->event = func;
    node->entity_id = id;
    node->next = 0;
    EventNode * current = RT.event_queue;
    if(!current){
        RT.event_queue = current;
    }
    else{
        while(current->next){
            current = current->next;
        }
        current->next = node;
    }
}

void register_system(char * name, void (*fn)()){
    cstrVoidFNHashTable_insert(RT.gen_comps.systems, name, fn);
}
void deregister_system(char * name){
    cstrVoidFNHashTable_remove(RT.gen_comps.systems, name);
}

void draw_call(fn_void func){
    v_append(RT.draw_calls, func);
}

void register_graphics_system(char * name, void(*fn)()){
    cstrVoidFNHashTable_insert(RT.gen_comps.graphics_systems, name, fn);
}
void deregister_graphics_system(char * name){
    cstrVoidFNHashTable_remove(RT.gen_comps.graphics_systems, name);
}

void register_drawing_system(char * name, void (*fn)()){
    cstrVoidFNHashTable_insert(RT.gen_comps.drawing_systems, name, fn);
}
void deregister_drawing_system(char * name){
    cstrVoidFNHashTable_remove(RT.gen_comps.drawing_systems, name);
}