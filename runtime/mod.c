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
static void lighting_call(Shader s,int light_count, Vector3 locations[], Vector4 colors[]){
    SetShaderValueV(s, GetShaderLocation(s,"light_positions"), locations, RL_SHADER_UNIFORM_VEC3, light_count);     
    SetShaderValueV(s, GetShaderLocation(s,"light_colors"), colors, RL_SHADER_UNIFORM_VEC4, light_count);  
    SetShaderValue(s, GetShaderLocation(s,"light_count"), &light_count, RL_SHADER_UNIFORM_INT);
}

static void run_lighting(Shader s,Vector3 location){
    const int size = 100;
    Vector3 positions[size] = {};
    Vector4 colors[size] = {};
    int count =0;
    SetShaderValue(s, GetShaderLocation(s, "directional_light"), &RT.directional_light_direction, RL_SHADER_UNIFORM_VEC3);
    Vector4 amb = {RT.ambient_color.r, RT.ambient_color.g, RT.ambient_color.b, RT.ambient_color.a};
    Vector4 dir = {RT.directional_light_color.r, RT.directional_light_color.g, RT.directional_light_color.b, RT.directional_light_color.a};
    amb = (Vector4){amb.x/255, amb.y/255, amb.z/255, amb.w/255};
    dir = (Vector4){dir.x/255, dir.y/255, dir.z/255, dir.w/255};
    SetShaderValue(s, GetShaderLocation(s, "ambient_color"), &amb,SHADER_UNIFORM_VEC4);
    SetShaderValue(s, GetShaderLocation(s, "directional_light_color"), &dir,SHADER_UNIFORM_VEC4); 
    for(int i= 0; i<RT.light_comps.length; i++){
        if(RT.light_comps.items[i].is_valid){
            if(Vector3Distance(location, get_location((Ref){i, RT.generations.items[i]}))<RT.light_comps.items[i].value.influence_radius){
                if(count<size){
                    positions[count] = get_location((Ref){i, RT.generations.items[i]});
                    Color t = RT.light_comps.items[i].value.color;
                    float brightness = RT.light_comps.items[i].value.brightness; 
                    colors[count] =(Vector4){t.r, t.g, t.b, t.a};
                    colors[count] = (Vector4){colors[count].x/255.0*brightness, colors[count].y/255.0*brightness, colors[count].z/255.0*brightness, colors[count].w/255.0*brightness};
                    count ++;
                }
            }
        }
    }
    lighting_call(s, count, positions, colors);
}
void init_runtime(void (*setup)(), void(*on_tick)(), void (*on_render)()){
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
    RT.loaded_models = Stringu32HashTable_create(1000, hash_string, string_equals);
    RT.loaded_shaders = Stringu32HashTable_create(1000, hash_string, string_equals); 
    RT.textures = ResourceTexture_make(UnloadTexture);
    RT.loaded_textures = Stringu32HashTable_create(1000, hash_string, string_equals);
    RT.ambient_color = (Color){64, 64, 64,128};
    RT.directional_light_color = (Color){128, 64, 58, 255};
    RT.directional_light_direction = (Vector3){0,0,-1};
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
    Shader default_post_process = LoadShader("shaders/base.vs", "shaders/default_post_process.fs");
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
                ModelComp cmp =RT.model_comps.items[i].value; 
                OptionShader shade = get_shader(RT.model_comps.items[i].value.shader_id);
                assert(shade.is_valid);
                if(!shade.is_valid){
                    continue;
                }
                Shader s = shade.value;
                Vector3 lc = get_location((Ref){i, RT.generations.items[i]});
                run_lighting(s, lc);
                int use_diffuse = cmp.diffuse_texture_id>0;
                int use_normals = cmp.normal_texture_id>0;
                int use_roughness = cmp.roughness_texture_id>0;
                int use_emmision = cmp.emmisive_texture_id>0;
                SetShaderValue(s, GetShaderLocation(s, "use_diffuse"), &use_diffuse, RL_SHADER_UNIFORM_INT);
                SetShaderValue(s, GetShaderLocation(s, "use_normals"), &use_normals, RL_SHADER_UNIFORM_INT);
                SetShaderValue(s, GetShaderLocation(s, "use_roughness"), &use_normals, RL_SHADER_UNIFORM_INT);
                SetShaderValue(s, GetShaderLocation(s, "use_emmision"), &use_emmision, RL_SHADER_UNIFORM_INT);
                if(!use_diffuse){
                    float color[4]= {cmp.tint.r, cmp.tint.g, cmp.tint.b, cmp.tint.a};
                    color[0] /= 255.0;
                    color[1] /= 255.0;
                    color[2] /= 255.0;
                    color[3]/= 255.0;
                    SetShaderValue(s, GetShaderLocation(s,"diffuse_tint"), &color, RL_SHADER_UNIFORM_VEC4);
                } else{
                    SetShaderValueTexture(s, GetShaderLocation(s, "diffuse"), get_texture(cmp.diffuse_texture_id).value);
                }
                if(!use_emmision){
                    float color[4]= {cmp.emmision.r, cmp.emmision.g, cmp.emmision.b, cmp.emmision.a};
                    color[0] /= 255.0;
                    color[1] /= 255.0;
                    color[2] /= 255.0;
                    color[3]/= 255.0;
                    SetShaderValue(s, GetShaderLocation(s,"emmision_tint"), &color, RL_SHADER_UNIFORM_VEC4);
                } else{
                    SetShaderValueTexture(s, GetShaderLocation(s, "emmision"), get_texture(cmp.emmisive_texture_id).value);
                }
                if(use_normals){
                  SetShaderValueTexture(s, GetShaderLocation(s, "normal"), get_texture(cmp.normal_texture_id).value);
                }
                if(!use_roughness){
                    float roughness = cmp.roughness;
                     SetShaderValue(s, GetShaderLocation(s,"colDiffuse"), &roughness, RL_SHADER_UNIFORM_FLOAT);
                } else{
                    SetShaderValueTexture(s, GetShaderLocation(s, "roughness"), get_texture(cmp.roughness_texture_id).value);
                }
                mat.shader = s;
                OptionModel msh = get_model(RT.model_comps.items[i].value.model_id);
                 if(!msh.is_valid){
                    continue;
                } 
                TransformComp * trans = get_transform_comp((Ref){i, RT.generations.items[i]});
                if(!trans){
                    continue;
                }
                Matrix rot = QuaternionToMatrix(get_rotation((Ref){i, RT.generations.items[i]}));
                Vector3 sc = get_scale((Ref){i, RT.generations.items[i]});
                Matrix scale = MatrixScale(sc.x, sc.y, sc.z);
                Matrix transform = MatrixMultiply(rot,scale);
                Matrix mt = MatrixMultiply(MatrixTranslate(get_location((Ref){i, RT.generations.items[i]}).x ,get_location((Ref){i, RT.generations.items[i]}).y, get_location((Ref){i,RT.generations.items[i]}).z), transform);
                SetShaderValueMatrix(s, GetShaderLocation(s, "matModel"), mt);
                msh.value.transform = transform;
                Material old = msh.value.materials[0];
                msh.value.materials[0] = mat;
                DrawModel(msh.value,get_location((Ref){i, RT.generations.items[i]}), 1.0,WHITE);
                msh.value.materials[0] = old;
            }
        }
        EndMode3D();
        EndTextureMode();
        BeginDrawing();
        float height = GetScreenHeight();
        float width = GetScreenWidth();
        //SetShaderValue(default_post_process, GetShaderLocation(default_post_process, "height"), &height, RL_SHADER_UNIFORM_FLOAT);
        //SetShaderValue(default_post_process, GetShaderLocation(default_post_process, "width"), &height, RL_SHADER_UNIFORM_FLOAT);
        //BeginShaderMode(default_post_process);
        DrawTextureRec(RT.target.texture, (Rectangle){ 0, 0, (float)RT.target.texture.width, (float)-RT.target.texture.height }, (Vector2){ 0, 0 }, WHITE);
        //EndShaderMode();
        on_render();
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
    unmake(RT.entities);
    ResourceModel_unmake(&RT.models);
    ResourceShader_unmake(&RT.shaders);
    ResourceTexture_unmake(&RT.textures);
    unmake_fn(RT.transform_comps, free_transform);
    unmake(RT.physics_comps);
    unmake(RT.model_comps);
    unmake(RT.character_comps);
    unmake(RT.light_comps);
    Stringu32HashTable_unmake_funcs(RT.loaded_models, free_string, 0);
    Stringu32HashTable_unmake_funcs(RT.loaded_shaders, free_string, 0);
   Stringu32HashTable_unmake_funcs(RT.loaded_textures, free_string, 0); 
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
bool destroy_entity(Ref id){
    if(id.id> RT.entities.length){
        return false;
    } 
    if(RT.entities.items[id.id]){
        RT.entities.items[id.id]->vtable->destructor(RT.entities.items[id.id]); 
        free(RT.entities.items[id.id]);
        RT.entities.items[id.id] = 0; 
        return true;
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
