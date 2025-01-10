#include <raylib.h>
#define RUNTIME_MOD
#include "runtime.h"
#include <raymath.h>
#include <rlgl.h>
#define true 1
#define false 0
void run_physics();
void init_physics_rt();
void finish_physics();
void default_on_tick(void *f, f32 dt){

}
void default_on_setup(void*self ,u32 self_id){
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
        v_append(RT.entities, 0);
        v_append(RT.model_comps,(OptionModelComp) {});
        v_append(RT.transform_comps,(OptionTransformComp){});
        v_append(RT.physics_comps,(OptionPhysicsComp) {});
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

void init_runtime(void (*setup)(), void(*on_tick)(), void (*on_render)()){
    tmp_reset();
    RT.entities = (EntityRefVec)make(0, EntityRef);
    RT.models = ResourceModel_make(UnloadModel);
    RT.shaders = ResourceShader_make(UnloadShader);
    RT.model_comps =(OptionModelCompVec)make(0, OptionModelComp);
    RT.transform_comps =(OptionTransformCompVec)make(0, OptionTransformComp);
    RT.physics_comps =(OptionPhysicsCompVec)make(0, OptionPhysicsComp);
    RT.loaded_models = Stringu32HashTable_create(1000, hash_string, string_equals);
    RT.loaded_shaders = Stringu32HashTable_create(1000, hash_string, string_equals); 
    for(int i =0; i<10; i++){
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
    Material mat = LoadMaterialDefault();
    Material svd_mat = mat;
    while(!WindowShouldClose()){
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
        run_physics();
        BeginTextureMode(RT.target);
        ClearBackground(BLACK);
        rlEnableBackfaceCulling();
        for (int i =0; i<RT.entities.length; i++){
            if (RT.entities.items[i]){
                RT.entities.items[i]->vtable->on_render( RT.entities.items[i]);
            }
        }
        if(RT.camera_parent.is_valid){
            RT.camera.position = get_transform_comp(RT.camera_parent.value)->transform.translation;
            RT.camera.target = Vector3Add(get_forward_vector(RT.camera_parent.value),RT.camera.position);
            RT.camera.up = get_up_vector(RT.camera_parent.value);
        }
        BeginMode3D(RT.camera);
        for (int i =0; i<RT.model_comps.length; i++){
            if(!RT.entities.items[i]){
                continue;
            }
            if (RT.model_comps.items[i].is_valid){
                OptionShader shade = get_shader(RT.model_comps.items[i].value.shader_id);
           
                assert(shade.is_valid);
                if(!shade.is_valid){
                    continue;
                }
                mat.shader = shade.value;
                OptionModel msh = get_model(RT.model_comps.items[i].value.model_id);
                 if(!msh.is_valid){
                    continue;
                } 
                TransformComp * trans = get_transform_comp(i);
                if(!trans){
                    continue;
                }
                Matrix rot = QuaternionToMatrix(get_rotation(i));
                Vector3 sc = get_scale(i);
                Matrix scale = MatrixScale(sc.x, sc.y, sc.z);
                Matrix transform = MatrixMultiply(rot,scale);
                msh.value.transform = transform;
                Material old = msh.value.materials[0];
                msh.value.materials[0] = mat;
                DrawModel(msh.value,get_location(i), 1.0,WHITE);
                msh.value.materials[0] = old;
            }
        }
        EndMode3D();
        on_render();
        EndTextureMode();
        BeginDrawing();
        DrawTextureRec(RT.target.texture, (Rectangle){ 0, 0, (float)RT.target.texture.width, (float)-RT.target.texture.height }, (Vector2){ 0, 0 }, WHITE);
        EndShaderMode();
        EndDrawing();
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
void unload_level(){
    for(int i =0; i<RT.entities.length; i++){
        if(RT.entities.items[i]){
            RT.entities.items[i]->vtable->destructor(RT.entities.items[i]);
            free(RT.entities.items[i]);
        }
    }
    unmake(RT.entities);
    ResourceModel_unmake(&RT.models);
    ResourceShader_unmake(&RT.shaders);
    unmake(RT.transform_comps);
    unmake(RT.physics_comps);
    unmake(RT.model_comps);
    Stringu32HashTable_unmake_funcs(RT.loaded_models, free_string, 0);
    Stringu32HashTable_unmake_funcs(RT.loaded_shaders, free_string, 0);
    UnloadRenderTexture(RT.target);
    EventNode* queue = RT.event_queue;
    tmp_reset();
}
Optionu32 create_entity(Entity * ent){
    for (int i=0; i<RT.entities.length; i++){
        if(!RT.entities.items[i]){
            RT.entities.items[i] = ent;
            ent->vtable->on_setup(ent, i);
            return (Optionu32)Some(i);
        }
    }
    RT.failed_to_create = true;
    return (Optionu32)None;
}
bool destroy_entity(u32 id){
    if(id> RT.entities.length){
        return false;
    } 
    if(RT.entities.items[id]){
        RT.entities.items[id]->vtable->destructor(RT.entities.items[id]); 
        free(RT.entities.items[id]);
        RT.entities.items[id] = 0; 
        return true;
    }
    return false;
}
Entity * get_entity(u32 id){
    if(id> RT.entities.length){
        return 0;
    } 
    return RT.entities.items[id];
}

Camera3D * get_camera(){
    return &RT.camera;
}


void call_event(u32 id, void (*func)(void * self, void * args), void * args){
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
