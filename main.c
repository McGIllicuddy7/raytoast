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
static u32 red;
static u32 white;
static u32 green;
Vector3 gen_random_vector(float in_radius){
    float radius = sqrt((f32)(rand()%100000)/100000.0);
    radius*= in_radius;
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
        add_force(id, Vector3Scale(Vector3Normalize(location), delta_time*-0.1));
    }
}

void setup(){
    srand(time(0));
    float rad = 1.0;
    u32 cube_id = create_mesh(GenMeshCube(0.2, 0.2, 0.2));
    u32 mesh_id = create_mesh(GenMeshSphere(0.1, 8, 8));
    assert(cube_id != mesh_id);
    MeshComp msh = {};
    msh.mesh_id = mesh_id;
    green= create_shader(LoadShader("shader/sbase.vs", "shaders/green.fs"));
    red = create_shader(LoadShader("shaders/base.vs", "shaders/red.fs"));
    msh.shader_id = white;
    int max = 1000;
    for(int i =0; i<max; i++){
        TestEntity * entity = malloc(sizeof(TestEntity));
        memcpy(entity->bytes, "012345678910", 13);
        entity->entity.vtable = &TestEntityVTable;
        u32 id = create_entity((void*)entity).value;
        assert(id == i);
        entity->entity.self_id = id;
        Transform transform;
        //transform.translation = (Vector3){16.0*cos((f32)i/max *2.0*PI), 16.0 *sin((f32)i/max *2.0*PI), 0};
        transform.translation = gen_random_vector(50.0);
        float scale = 1.0;
        msh.shader_id = i%2 == 0 ? red :white;
        transform.scale = (Vector3){scale, scale, scale};
        transform.rotation = (Quaternion){0.0, 0.0, 0.0, 1.0};
        TransformComp trans ={};
        trans.transform = transform;
        PhysicsComp phys = {};
        phys.movable = 1;
        phys.box.max = (Vector3){0.1, 0.1, 0.1};
        phys.box.min = (Vector3){-0.1, -0.1, -0.1};
        phys.velocity = Vector3Scale(Vector3RotateByQuaternion(Vector3Negate(Vector3Normalize(transform.translation)), (Quaternion){0.0, 0.0, 0.0,1.0}),10.0);
        phys.mass = 1.0;
        set_transform_comp(id, trans);
        set_mesh_comp(id, msh);
        set_physics_comp(id, phys);
    }
}
void on_tick(){
    static u128 frame_count = 0;
    UpdateCamera(get_camera(), CAMERA_THIRD_PERSON);
    if(IsKeyPressed(KEY_ESCAPE)){
        exit(0);
    }
    if(GetFrameTime()>1.0/30.0&& frame_count>120){
        printf("lag spike\n");
        exit(0);
    }
    frame_count += 1;
}
void on_render(){
    DrawFPS(1550, 100);
}
int main(void){
    init_runtime(setup, on_tick, on_render);
} 
