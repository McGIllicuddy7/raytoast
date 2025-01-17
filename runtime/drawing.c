#include "drawing.h"
#include "runtime.h"
#include "rlgl.h"
make_lambda_capture(fn_void, void, draw_line_lambda, {DrawLine3D(captures->start_pos, captures->end_pos, captures->color);},{Vector3 start_pos; Vector3 end_pos; Color color;});

make_lambda_capture(fn_void, void, draw_sphere_lambda, {DrawSphere(captures->center, captures->radius, captures->color); printf("called draw\n");},{Vector3 center; float radius; Color color;});
void draw_line(Vector3 start, Vector3 end, Color color){
    fn_void func = tmp_lambda(draw_line_lambda, {start, end, color});
    draw_call(func);
}
void draw_sphere(Vector3 center_pos,float radius, Color color ){
    fn_void func = tmp_lambda(draw_sphere_lambda, {center_pos, radius, color});
    draw_call(func); 
}


static void lighting_call(Shader s,int light_count, Vector3 locations[], Vector4 colors[]){
    SetShaderValueV(s, GetShaderLocation(s,"light_positions"), locations, RL_SHADER_UNIFORM_VEC3, light_count);     
    SetShaderValueV(s, GetShaderLocation(s,"light_colors"), colors, RL_SHADER_UNIFORM_VEC4, light_count);  
    SetShaderValue(s, GetShaderLocation(s,"light_count"), &light_count, RL_SHADER_UNIFORM_INT);
}

static void run_lighting(Shader s,Vector3 location){
    #define size 100
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
    #undef size
}
void run_graphics_systems(){
    Iterator it = ITER_HASHTABLE(RT.gen_comps.graphics_systems);
    void (**fn)() = 0;
    while((fn = NEXT(it))){
        (*fn)();
    } 
}
void run_drawing_systems(){
    Iterator it = ITER_HASHTABLE(RT.gen_comps.drawing_systems);
    void (**fn)() = 0;
    while((fn = NEXT(it))){
        (*fn)();
    }  
}
void run_drawing(Material mat, Shader default_post_process, void (*on_render)()){
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
                ModelComp mdcmp = RT.model_comps.items[i].value;
                Matrix rot = QuaternionToMatrix(get_rotation((Ref){i, RT.generations.items[i]}));
                Vector3 sc = get_scale((Ref){i, RT.generations.items[i]});
                Matrix scale = MatrixScale(sc.x, sc.y, sc.z);
                Matrix transform = MatrixMultiply(rot,scale);
                Matrix mt = MatrixMultiply(MatrixTranslate(get_location((Ref){i, RT.generations.items[i]}).x ,get_location((Ref){i, RT.generations.items[i]}).y, get_location((Ref){i,RT.generations.items[i]}).z), transform);
                Matrix rel = MatrixMultiply(MatrixTranslate(mdcmp.relative_transform.translation.x, mdcmp.relative_transform.translation.y, mdcmp.relative_transform.translation.z), MatrixMultiply(QuaternionToMatrix(mdcmp.relative_transform.rotation), MatrixScale(mdcmp.relative_transform.scale.x, mdcmp.relative_transform.scale.y,mdcmp.relative_transform.scale.z)));
                mt = MatrixMultiply(rel, mt);
                SetShaderValueMatrix(s, GetShaderLocation(s, "matModel"), mt);
                transform = MatrixMultiply(transform,MatrixMultiply(QuaternionToMatrix(mdcmp.relative_transform.rotation), MatrixScale(mdcmp.relative_transform.scale.x, mdcmp.relative_transform.scale.y,mdcmp.relative_transform.scale.z)));
                msh.value.transform = transform;
                Material old = msh.value.materials[0];
                msh.value.materials[0] = mat;
                DrawModel(msh.value,Vector3Add(get_location((Ref){i, RT.generations.items[i]}),mdcmp.relative_transform.translation), 1.0,WHITE);
                msh.value.materials[0] = old;
            }
        }
        for(int i =0; i<RT.draw_calls.length; i++){
            fn_void func = RT.draw_calls.items[i];
            call(func);
        }
        run_graphics_systems();
        EndMode3D();
        run_drawing_systems();
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
}