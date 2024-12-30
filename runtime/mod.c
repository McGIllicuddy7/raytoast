#define RUNTIME_MOD
#include "../runtime.h"
#include <raymath.h>
#include <rlgl.h>
#define true 1
#define false 0
void run_physics();
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
    for(int i =0; i<500; i++){
        runtime_reserve();
    }
    SetTraceLogLevel(LOG_ALL);
    InitWindow(1600, 1024,"raytoast");
    SetTargetFPS(61);
    DisableCursor();
    RT.camera.fovy = 90.0;
    RT.camera.position = (Vector3){0.0, 0.0, 0.0};
    RT.camera.up = (Vector3){0.0,0.0, 1.0};
    RT.camera.target = (Vector3){1.0, 0.0, 0.0};
    RT.camera.projection = CAMERA_PERSPECTIVE;
    RT.target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    setup();

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
        Material mat = LoadMaterialDefault();
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
                //Matrix loc = MatrixTranslate(trans->transform.translation.x, 
               // trans->transform.translation.y, trans->transform.translation.z);
                Matrix rot = QuaternionToMatrix(trans->transform.rotation);
                Matrix scale = MatrixScale(trans->transform.scale.x, trans->transform.scale.y, trans->transform.scale.z);
                Matrix transform = MatrixMultiply(rot,scale);
                msh.value.transform = transform;
                Material old = msh.value.materials[0];
                msh.value.materials[0] = mat;
                DrawModel(msh.value,trans->transform.translation, 1.0,WHITE);
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

bool set_transform_comp(u32 id, TransformComp trans){
    if(id>RT.entities.length){
        return false;
    }
    if(!RT.entities.items[id]){
        return false;
    }
    RT.transform_comps.items[id] = (OptionTransformComp)Some(trans);
    return true;
}
TransformComp * get_transform_comp(u32 id){
    if(id>=RT.entities.length){
        return 0;
    }
    if(!RT.entities.items[id]){
        return 0;
    }
    if(RT.transform_comps.items[id].is_valid){
       return &RT.transform_comps.items[id].value;
    }
    return 0;
}
bool remove_transform_comp(u32 id){
    if(id>RT.entities.length){
        return false;
    }
    if(!RT.entities.items[id]){
        return false;
    }
    RT.transform_comps.items[id] = (OptionTransformComp)None;
    return true;
}

bool set_physics_comp(u32 id, PhysicsComp phys){
    if(id>RT.entities.length){
        return false;
    }
    if(!RT.entities.items[id]){
        return false;
    }
    RT.physics_comps.items[id].value = phys;
    RT.physics_comps.items[id].is_valid = true;
    return true;
}
PhysicsComp * get_physics_comp(u32 id){
    if(id>RT.entities.length){
        return 0;
    }
    if(!RT.entities.items[id]){
        return 0;
    }
    if(RT.physics_comps.items[id].is_valid){
       return &RT.physics_comps.items[id].value;
    }
    return 0;
}
bool remove_physics_comp(u32 id){
    if(id>RT.entities.length){
        return false;
    }
    if(!RT.entities.items[id]){
        return false;
    }
    RT.physics_comps.items[id] = (OptionPhysicsComp)None;
    return true;
}

bool set_model_comp(u32 id, ModelComp model){
    if(id>RT.entities.length){
        return false;
    }
    if(!RT.entities.items[id]){
        return false;
    }
    RT.model_comps.items[id] = (OptionModelComp)Some(model); 
    return true;
}
ModelComp * get_model_comp(u32 id){
    if(id>RT.entities.length){
        return 0;
    }
    if(!RT.entities.items[id]){
        return 0;
    }
    if(RT.model_comps.items[id].is_valid){
       return &RT.model_comps.items[id].value;
    }
    return 0;
}
bool remove_model_comp(u32 id){
    if(id>RT.entities.length){
        return false;
    }
    if(!RT.entities.items[id]){
        return false;
    }
    RT.model_comps.items[id] = (OptionModelComp)None;
    return true;
}

OptionShader get_shader(u32 id){
    return RT.shaders.values.items[id];
}
OptionModel get_model(u32 id){
    return RT.models.values.items[id];
}

u32 create_shader(Shader shader){
    return ResourceShader_create(&RT.shaders, shader);
}
bool remove_shader(u32 id){
    ResourceShader_destroy(&RT.shaders, id);
    return true;
}

u32 create_model(Model model){
    return ResourceModel_create(&RT.models, model);
}
bool remove_model(u32 id){
  ResourceModel_destroy(&RT.models, id);
  return true;
}
Camera3D * get_camera(){
    return &RT.camera;
}

void add_force(u32 id, Vector3 force){
    PhysicsComp * phys = get_physics_comp(id);
    if(phys){
        phys->velocity = Vector3Add(phys->velocity, Vector3Scale(force, 1/phys->mass));
    }
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

u32 load_shader(const char * vertex_path, const char *frag_path){
    String name = new_string(0,vertex_path);
    str_concat(name, "\0");
    str_concat(name, frag_path);
    if(Stringu32HashTable_contains(RT.loaded_shaders, name)){
        u32 * out = Stringu32HashTable_find(RT.loaded_shaders, name);
        unmake(name);
        return *out;
    }  else{
        Shader shader = LoadShader(vertex_path, frag_path);
        u32 out = create_shader(shader);
        Stringu32HashTable_insert(RT.loaded_shaders, name, out);
        return out;
    }

}
void unload_shader(u32 id){
    if(!RT.shaders.values.items[id].is_valid){
        return;
    }
    UnloadShader(RT.shaders.values.items[id].value);
    RT.shaders.values.items[id] = (OptionShader){};
    for(int i =0; i<RT.loaded_shaders->TableSize; i++){
        Stringu32KeyValuePairVec * v = &RT.loaded_shaders->Table[i];
        for(int j =0; j<v->length; j++){
            Stringu32KeyValuePair p = v->items[i];
            if(p.value == id){
                unmake(p.key);
                v_remove((*v), j);
            }
        }
    }
}

u32 load_model(const char * path){
    String name = new_string(0,path);
    if(Stringu32HashTable_contains(RT.loaded_shaders, name)){
        u32 * out = Stringu32HashTable_find(RT.loaded_models, name);
        unmake(name);
        return *out;
    }  else{
        Model model = LoadModel(path);
        u32 out = create_model(model);
        Stringu32HashTable_insert(RT.loaded_models,name, out );
        return out;
    }
}
void unload_model(u32 id){
     if(!RT.models.values.items[id].is_valid){
        return;
    }
    UnloadModel(RT.models.values.items[id].value);
    RT.models.values.items[id] = (OptionModel){};
    for(int i =0; i<RT.loaded_models->TableSize; i++){
        Stringu32KeyValuePairVec * v = &RT.loaded_models->Table[i];
        for(int j =0; j<v->length; j++){
            Stringu32KeyValuePair p = v->items[i];
            if(p.value == id){
                unmake(p.key);
                v_remove((*v), j);
            }
        }
    }
}