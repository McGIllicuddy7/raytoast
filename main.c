#include <stdio.h>
#include <raylib.h>
#include "runtime.h"

#include "utils.h"
typedef struct{
    Entity entity;
    u32 hp;
}TestEntity;
Vector3 gen_random_vector(float in_radius){
    float radius = sqrt((rand()%100000)/100000.0)*in_radius;
    float phi = rand()%100000/100000.0*2*PI;
    float theta = rand()%100000/100000.0*2*PI;
    return (Vector3){cos(theta)*cos(phi)*radius, sin(theta)*cos(phi)*radius, sin(phi)*radius};
}
void setup(){
    srand(time(0));
    u32 mesh_id = create_mesh(GenMeshSphere(0.1, 16, 16));
    MeshComp msh = {};
    msh.mesh_id = mesh_id;
    msh.shader_id = create_shader(LoadMaterialDefault().shader);
    for(int i =0; i<100000; i++){
        TestEntity * entity = malloc(sizeof(TestEntity));
        entity->entity.vtable = &entity_default_vtable;
        u32 id = create_entity(&entity->entity).value;
        Transform transform;
        transform.translation = gen_random_vector(200);
        transform.scale = (Vector3){1,1,1};
        transform.rotation = (Quaternion){0.0, 0.0, 0.0, 1.0};
        TransformComp trans ={};
        trans.transform = transform;
        set_transform_comp(id, trans);
        set_mesh_comp(id, msh);
    }
}
void on_tick(){
    UpdateCamera(get_camera(), CAMERA_THIRD_PERSON);
}
void on_render(){
    DrawFPS(1550, 100);
}
int main(void){
    init_runtime(setup, on_tick, on_render);
} 
