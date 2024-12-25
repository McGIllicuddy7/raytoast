#include <stdio.h>
#include <raylib.h>
#include "runtime.h"
#include <raymath.h>
#include "utils.h"
typedef struct{
    Entity entity;
    u32 hp;
}TestEntity;
Vector3 gen_random_vector(float in_radius){
    float radius = ((rand()%100000)/100000.0)*((rand()%100000)/100000.0)*in_radius;
    float phi = rand()%100000/100000.0*2*PI;
    float theta = rand()%100000/100000.0*2*PI;
    return (Vector3){cos(theta)*cos(phi)*radius, sin(theta)*cos(phi)*radius, sin(phi)*radius};
}
void TestEntity_on_tick(TestEntity * self, float delta_time){
    u32 id = self->entity.self_id;
    Vector3 location = get_transform_comp(id)->transform.translation;
    if(Vector3Distance(location, (Vector3){0,0,0})<1e-16){
        add_force(id, gen_random_vector(10.0));
    } else{
        add_force(id, Vector3Scale(Vector3Normalize(location), delta_time*-1.0));
    }
}
void setup(){
    srand(time(0));
    float rad = 0.1;
    u32 mesh_id = create_mesh(GenMeshSphere(0.1, 8, 8));
    MeshComp msh = {};
    msh.mesh_id = mesh_id;
    msh.shader_id = create_shader(LoadMaterialDefault().shader);
    for(int i =0; i<10000; i++){
        TestEntity * entity = malloc(sizeof(TestEntity));
        entity->entity.vtable = &entity_default_vtable;
        entity->entity.vtable->on_tick = (void*)TestEntity_on_tick;
        u32 id = create_entity(&entity->entity).value;
        Transform transform;
        transform.translation = gen_random_vector(10.0);
        float scale = rand()%1000/1000.0;
        if(scale<0.5) scale = 0.5;
        transform.scale = (Vector3){scale, scale, scale};
        transform.rotation = (Quaternion){0.0, 0.0, 0.0, 1.0};
        TransformComp trans ={};
        trans.transform = transform;
        PhysicsComp phys = {};
        phys.box = (BoundingBox){{-rad, -rad, -rad}, {rad, rad, rad}};
        phys.velocity = Vector3Normalize(Vector3RotateByAxisAngle(transform.translation, (Vector3){1,0,0}, 90.0));
        phys.mass = 1.0;
        set_transform_comp(id, trans);
        set_mesh_comp(id, msh);
        set_physics_comp(id, phys);
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
