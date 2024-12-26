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
   return GetRandomValue(0,10'000)/(10'000.0);
}
Vector3 gen_random_vector(float in_radius){
    
    float radius = sqrt(random_float());
    radius*= in_radius;
    float phi = random_float()*2*PI;
    if(fabs(phi-PI/2.0)<PI/12.0|| fabs(phi-3*PI/2.0)<PI/12.0){
        phi += (2*random_float()-1)*PI*2.0;
    }
    float theta = random_float()*2*PI;
    return (Vector3){cos(theta)*cos(phi)*radius, sin(theta)*cos(phi)*radius, sin(phi)*radius};
}
Vector3 vec_from_sphere(float radius, float phi, float theta){
    if(fabs(phi-PI/2.0)<PI/12.0|| fabs(phi-3*PI/2.0)<PI/12.0){
        phi += (2*random_float()-1)*PI*2.0;
    }
    return (Vector3){cos(theta)*cos(phi)*radius, sin(theta)*cos(phi)*radius, sin(phi)*radius}; 
}
void TestEntity_on_tick(TestEntity * self, float delta_time){
    u32 id = self->entity.self_id;
    Vector3 location = get_transform_comp(id)->transform.translation;
    if(Vector3Distance(location, (Vector3){0,0,0})<1e-16){
        //add_force(id, gen_random_vector(10.0));
    } else{
        add_force(id, Vector3Scale(location, -delta_time*0.05));
    }
}

void setup(){
    srand(time(0));
    SetRandomSeed(time(0));
    float rad = 1.0;
    u32 cube_id = create_mesh(GenMeshCube(0.2, 0.2, 0.2));
    u32 mesh_id = create_mesh(GenMeshSphere(0.1, 16, 16));
    assert(cube_id != mesh_id);
    MeshComp msh = {};
    msh.mesh_id = mesh_id;
    green= create_shader(LoadShader("shader/sbase.vs", "shaders/white.fs"));
    red = create_shader(LoadShader("shaders/base.vs", "shaders/red.fs"));
    msh.shader_id = white;
    int max = 1000;
    int movable_amnt = 1;
    for(int i =0; i<max; i++){
        TestEntity * entity = malloc(sizeof(TestEntity));
        memcpy(entity->bytes, "012345678910", 13);
        entity->entity.vtable = &TestEntityVTable;
        u32 id = create_entity((void*)entity).value;
        entity->entity.self_id = id;
        Transform transform;
        //transform.translation = (Vector3){4*cos((f32)i/max *2.0*PI), 4 *sin((f32)i/max *2.0*PI), 0};
        float theta = random_float()*2*PI;
        float phi = random_float()*2*PI;
        float radius = sqrt(random_float())*30.0;
        float scale = 1.0;
        transform.translation = vec_from_sphere(radius, phi, theta);
        msh.shader_id = i %movable_amnt == 0?  white: red;
        msh.mesh_id = i%movable_amnt == 0 ? mesh_id : cube_id;
        transform.scale = (Vector3){scale, scale, scale};
        transform.rotation = (Quaternion){0.0, 0.0, 0.0, 1.0};
        TransformComp trans ={};
        trans.transform = transform;
        PhysicsComp phys = {};
        phys.movable = 1;
        phys.box.max = (Vector3){0.1, 0.1, 0.1};
        phys.box.min = (Vector3){-0.1, -0.1, -0.1};
        //phys.velocity = (Vector3){sin(theta), -cos(theta), 0};
        phys.velocity = gen_random_vector(10.0);
        phys.velocity = Vector3Scale(phys.velocity, sqrt(radius));
        phys.mass = 1.0;
        phys.movable = i%movable_amnt==0 ;
        set_transform_comp(id, trans);
        set_mesh_comp(id, msh);
        set_physics_comp(id, phys);
        get_camera()->position = (Vector3){-8.0, 0,0};
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
