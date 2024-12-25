#define RUNTIME_MOD
#include "../runtime.h"
#include <raymath.h>
static void default_on_tick(void *f, f32 dt){

}
static void default_on_setup(void*self ,u32 self_id){

}
static void default_on_render(void * self){

}
static void default_on_destroy(void * self){

}
EntityVTable entity_default_vtable = 
{.on_tick = default_on_tick, 
.on_setup = default_on_setup, 
.destructor = default_on_destroy,
.on_render = default_on_render};
static Runtime RT = {};
static void runtime_reserve(){
    for(int i =0; i<10000; i++){
        v_append(RT.entities, 0);
        v_append(RT.mesh_comps,(OptionMeshComp) None);
        v_append(RT.transform_comps,(OptionTransformComp) None);
        v_append(RT.physics_comps,(OptionPhysicsComp) None);
    }
}

void init_runtime(void (*setup)(), void(*on_tick)(), void (*on_render)()){
    RT.entities = (EntityRefVec)make(0, EntityRef);
    RT.meshes = ResourceMesh_new(UnloadMesh);
    RT.shaders = ResourceShader_new(UnloadShader);
    RT.mesh_comps =(OptionMeshCompVec)make(0, OptionMeshComp);
    RT.transform_comps =(OptionTransformCompVec)make(0, OptionTransformComp);
    RT.physics_comps =(OptionPhysicsCompVec)make(0, OptionPhysicsComp);
    runtime_reserve();
    InitWindow(1024, 1024,"rayc");
    SetTargetFPS(61);
    DisableCursor();
    RT.camera.fovy = 90.0;
    RT.camera.position = (Vector3){0.0, 0.0, 0.0};
    RT.camera.up = (Vector3){0.0,0.0, 1.0};
    RT.camera.target = (Vector3){1.0, 0.0, 0.0};
    RT.camera.projection = CAMERA_PERSPECTIVE;
    setup();
    while(!WindowShouldClose()){
        if (RT.failed_to_create){
            runtime_reserve();
        }
        for (int i =0; i<RT.entities.length; i++){
            if (RT.entities.items[i]){
                RT.entities.items[i]->vtable->on_render( &RT.entities.items[i]);
            }
        }
        on_tick();
        BeginDrawing();
        ClearBackground(BLACK);
        for (int i =0; i<RT.entities.length; i++){
            if (RT.entities.items[i]){
                RT.entities.items[i]->vtable->on_render( &RT.entities.items[i]);
            }
        }
        Material mat = LoadMaterialDefault();
        BeginMode3D(RT.camera);
        for (int i =0; i<RT.mesh_comps.length; i++){
            if (RT.mesh_comps.items[i].is_valid){
                OptionShader shade = get_shader(RT.mesh_comps.items[i].value.shader_id);
                if(!shade.is_valid){
                    continue;
                }
                mat.shader = shade.value;
                OptionMesh msh = get_mesh(RT.mesh_comps.items[i].value.shader_id);
                 if(!msh.is_valid){
                    continue;
                } 
                TransformComp * trans = get_transform_comp(i);
                if(!trans){
                    continue;
                }
                Matrix loc = MatrixTranslate(trans->transform.translation.x, 
                trans->transform.translation.y, trans->transform.translation.z);
                Matrix rot = QuaternionToMatrix(trans->transform.rotation);
                Matrix scale = MatrixScale(trans->transform.scale.x, trans->transform.scale.y, trans->transform.scale.z);
                Matrix transform = MatrixMultiply(loc, MatrixMultiply(rot, scale));
                DrawMesh(msh.value, mat,transform);
            }
        }
        EndMode3D();
        on_render();
        EndDrawing();
    }
}

Optionu32 create_entity(Entity * ent){
    for (int i=0; i<RT.entities.length; i++){
        if(!RT.entities.items[i]){
            RT.entities.items[i] = ent;
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
    if(id>RT.entities.length){
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
    RT.physics_comps.items[id] = (OptionPhysicsComp)Some(phys);
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

bool set_mesh_comp(u32 id, MeshComp mesh){
    if(id>RT.entities.length){
        return false;
    }
    if(!RT.entities.items[id]){
        return false;
    }
    RT.mesh_comps.items[id] = (OptionMeshComp)Some(mesh); 
    return true;
}
MeshComp * get_mesh_comp(u32 id){
    if(id>RT.entities.length){
        return 0;
    }
    if(!RT.entities.items[id]){
        return 0;
    }
    if(RT.mesh_comps.items[id].is_valid){
       return &RT.mesh_comps.items[id].value;
    }
    return 0;
}
bool remove_mesh_comp(u32 id){
    if(id>RT.entities.length){
        return false;
    }
    if(!RT.entities.items[id]){
        return false;
    }
    RT.mesh_comps.items[id] = (OptionMeshComp)None;
    return true;
}

OptionShader get_shader(u32 id){
    return RT.shaders.values.items[id];
}
OptionMesh get_mesh(u32 id){
    return RT.meshes.values.items[id];
}

u32 create_shader(Shader shader){
    return ResourceShader_create(&RT.shaders, shader);
}
bool remove_shader(u32 id){
    ResourceShader_destroy(&RT.shaders, id);
    return true;
}

u32 create_mesh(Mesh mesh){
    return ResourceMesh_create(&RT.meshes, mesh);
}
bool remove_mesh(u32 id){
  ResourceMesh_destroy(&RT.meshes, id);
  return true;
}
Camera3D * get_camera(){
    return &RT.camera;
}