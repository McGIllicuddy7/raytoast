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
float random_float(){
   return rand()%1'000/1'000.0;
}
Vector3 gen_random_vector(float in_radius){
    
    float radius = sqrt(random_float());
    radius*= in_radius;
    float phi = random_float()*2*PI;
    if(fabs(phi-PI/2.0)<PI/10.0|| fabs(phi-3*PI/2.0)<PI/10.0){
        phi += (2*random_float()-1)*PI;
    }
    float theta = random_float()*2*PI;
    return (Vector3){cos(theta)*cos(phi)*radius, sin(theta)*cos(phi)*radius, sin(phi)*radius};
}
void TestEntity_on_tick(TestEntity * self, float delta_time){
    u32 id = self->entity.self_id;
    Vector3 location = get_transform_comp(id)->transform.translation;
    if(Vector3Distance(location, (Vector3){0,0,0})<1e-16){
        //add_force(id, gen_random_vector(10.0));
    } else{
        add_force(id, Vector3Scale(location, -delta_time*0.1));
    }
}

void setup(){
    srand(time(0));
    float rad = 1.0;
    u32 cube_id = create_mesh(GenMeshCube(0.2, 0.2, 0.2));
    u32 mesh_id = create_mesh(GenMeshSphere(0.2, 16, 16));
    assert(cube_id != mesh_id);
    MeshComp msh = {};
    msh.mesh_id = mesh_id;
    green= create_shader(LoadShader("shader/sbase.vs", "shaders/white.fs"));
    red = create_shader(LoadShader("shaders/base.vs", "shaders/red.fs"));
    msh.shader_id = white;
    int max = 12000;
    for(int i =0; i<max; i++){
        TestEntity * entity = malloc(sizeof(TestEntity));
        memcpy(entity->bytes, "012345678910", 13);
        entity->entity.vtable = &TestEntityVTable;
        u32 id = create_entity((void*)entity).value;
        assert(id == i);
        entity->entity.self_id = id;
        Transform transform;
        //transform.translation = (Vector3){16.0*cos((f32)i/max *2.0*PI), 16.0 *sin((f32)i/max *2.0*PI), 0};
        transform.translation = gen_random_vector(40.0);
        float scale = 1.0;
        msh.shader_id = white;
        transform.scale = (Vector3){scale, scale, scale};
        transform.rotation = (Quaternion){0.0, 0.0, 0.0, 1.0};
        TransformComp trans ={};
        trans.transform = transform;
        PhysicsComp phys = {};
        phys.movable = 1;
        phys.box.max = (Vector3){0.1, 0.1, 0.1};
        phys.box.min = (Vector3){-0.1, -0.1, -0.1};
        phys.velocity = Vector3Scale(Vector3Negate(Vector3Normalize(transform.translation)),1.0);
        phys.mass = 1.0;
        set_transform_comp(id, trans);
        set_mesh_comp(id, msh);
        set_physics_comp(id, phys);
        get_camera()->position = (Vector3){-20.0, 0,0};
    }
}
void on_tick(){
    static u128 frame_count = 0;
    UpdateCamera(get_camera(), CAMERA_FREE);
    if(IsKeyPressed(KEY_ESCAPE)){
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
