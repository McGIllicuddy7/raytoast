#include <stdio.h>
#include <raylib.h>
#include "runtime.h"
#include <raymath.h>
#include "utils.h"
typedef struct{
    Entity entity;
    char bytes[13];
}TestEntity;
void TestEntity_on_tick(TestEntity * self, float delta_time);
EntityVTable TestEntityVTable = {.destructor  = default_on_destroy, .on_render = default_on_render, .on_tick = (void*)TestEntity_on_tick, .on_setup = default_on_setup};
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
        //add_force(id, gen_random_vector(10.0));
    } else{
        //add_force(id, Vector3Scale(Vector3Normalize(location), delta_time*-0.1));
    }
    if(get_physics_comp(id)->collided_this_frame){
        get_mesh_comp(id)->shader_id = 1;
    } else{
        get_mesh_comp(id)->shader_id = 0; 
    }
}

void setup(){
    srand(time(0));
    float rad = 0.1;
    u32 mesh_id = create_mesh(GenMeshSphere(0.1, 8, 8));
    MeshComp msh = {};
    msh.mesh_id = mesh_id;
    msh.shader_id = create_shader(LoadShader(0, "shaders/white.fs"));
    create_shader(LoadShader(0, "shaders/red.fs"));
    for(int i =0; i<100; i++){
        TestEntity * entity = malloc(sizeof(TestEntity));
        memcpy(entity->bytes, "012345678910", 13);
        entity->entity.vtable = &TestEntityVTable;
        u32 id = create_entity((void*)entity).value;
        assert(id == i);
        entity->entity.self_id = id;
        Transform transform;
        transform.translation = gen_random_vector(1.0);
        float scale = 1.0;
        transform.scale = (Vector3){scale, scale, scale};
        transform.rotation = (Quaternion){0.0, 0.0, 0.0, 1.0};
        TransformComp trans ={};
        trans.transform = transform;
        PhysicsComp phys = {};
        phys.box = (BoundingBox){{-rad, -rad, -rad}, {rad, rad, rad}};
        phys.velocity = Vector3Negate(transform.translation);
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
