#include <stdio.h>
#include <raylib.h>
#include "runtime.h"
#include <raymath.h>
#include <unistd.h>
#include "utils.h"
#include "drawing.h"
#include </opt/homebrew/include/gperftools/profiler.h>

typedef struct{
    Entity entity;
    char bytes[13];
}TestEntity;
void TestEntity_on_tick(TestEntity * self, float delta_time);
EntityVTable TestEntityVTable = {.destructor  = default_on_destroy, .on_render = default_on_render, .on_tick = (void*)TestEntity_on_tick, .on_setup = default_on_setup};
extern char * hello_from_rust(int f);
extern Entity * create_test_rust_entity();
static Ref player_id = {};
float random_float(){
   return GetRandomValue(0,10'000'000)/(10'000'000.0);
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
    Ref id = self->entity.self_id;
    PhysicsComp*cmp = get_physics_comp(self->entity.self_id);
    if(!cmp){
        return;
    }
    cmp->collided_this_frame = false;
    Vector3 location = get_transform_comp(id)->transform.translation;
    //printf("%f\n", Vector3Length(cmp->velocity));
    if(Vector3Distance(location, (Vector3){0,0,0})<1e-16){
        //add_force(id, gen_random_vector(10.0));
    } else{
        add_force(id,(Vector3) {0,0,-delta_time});
    }
}
Ref create_wall(Vector3 location, Vector3 scale, u32 mesh_id,Color tin){
    TestEntity * floor = malloc(sizeof(TestEntity));
    memcpy(floor->bytes, "012345678910", 13);
    floor->entity.vtable = &TestEntityVTable;
    Ref id = create_entity((void*)floor).value;
    floor->entity.self_id = id;
    Transform transform;
    transform.rotation = (Quaternion){0,0,0,1};
    transform.scale = Vector3Scale(scale, 1.0);
    transform.translation = location;
    TransformComp trans ={};
    trans.transform = transform;
    trans.parent.is_valid = false;
    PhysicsComp phys = {};
    phys.movable =false;
    phys.box.max = Vector3Scale(scale, 0.5);
    phys.box.min = Vector3Scale(scale, -0.5);
    phys.velocity = (Vector3){};
    phys.mass = scale.x*scale.y*scale.z;
    phys.can_bounce = false;
    set_transform_comp(id, trans);
    ModelComp msh = {};
    msh.model_id =mesh_id;
    msh.shader_id=0;
    msh.tint = tin;
    msh.relative_transform = transform_default();
    set_model_comp(id, msh);
    set_physics_comp(id, phys);
    return id;
}
Ref create_character(Vector3 location, Vector3 forward){
    Entity*et = malloc(sizeof(TestEntity));
    et->vtable = &TestEntityVTable;
    Ref ref = create_entity(et).value;
    PhysicsComp phys = {};
    float sc = 0.5;
    phys.box.max = (Vector3){sc, sc, sc};
    phys.box.min = (Vector3){-sc, -sc, -sc};
    phys.can_bounce =false;
    phys.movable = true;
    phys.mass = 1.0;
    CharacterComp chr;
    chr.control_rotation = quat_from_vector(forward);
    TransformComp trans  ={};
    trans.transform = transform_default();
    trans.transform.translation = location;
    set_physics_comp(ref, phys);
    set_transform_comp(ref, trans);
    set_character_comp(ref, chr);
    Transform rel = transform_default();
    attach_camera_to(ref, rel);
    return ref;
}
void setup(){
    srand(time(0));
    SetRandomSeed(time(0));
    float rad = 1.0;
    u32 cube_id = create_model(LoadModelFromMesh(GenMeshCube(1.0, 1.0, 1.0)));
    u32 model_id = load_model("sphere.obj");
    ModelComp msh = {};
    msh.relative_transform = transform_default();
    msh.model_id = cube_id;
    msh.shader_id = load_shader("shaders/base.vs", "shaders/bsdf.fs");
    msh.tint = GREEN;
    int max =500;
    int movable_amnt = 1;
    int xc = 0;
    int yc = 0;
    for(int i =0; i<max; i++){
        if(i%10 ==0) yc++;
        xc++;
        xc = xc%10;
        TestEntity * entity = malloc(sizeof(TestEntity));
        memcpy(entity->bytes, "012345678910", 13);
        entity->entity.vtable = &TestEntityVTable;
        Ref id = create_entity((void*)entity).value;
        entity->entity.self_id = id;
        Transform transform;
        transform.translation = (Vector3){5*xc-25, 5*yc-25, 20};
        float theta = random_float()*2*PI;
        float phi = random_float()*2*PI;
        float radius = sqrt(random_float())*50;
        float scale = 1.0;
        transform.translation = vec_from_sphere(radius, phi, theta);
        transform.translation.z = fabs(transform.translation.z)+5.0+random_float()*40;
        transform.scale = (Vector3){scale, scale, scale};
        transform.rotation = (Quaternion){0.0, 0.0, 0.0, 1.0};
        TransformComp trans ={};
        trans.transform = transform;
        trans.parent.is_valid = false;
        PhysicsComp phys = {};
        phys.movable = true;
        phys.box.max = (Vector3){0.5, 0.5, 0.5};
        phys.box.min = (Vector3){-0.5, -0.5, -0.5};
        phys.box.max = Vector3Scale(phys.box.max, scale);
        phys.box.min = Vector3Scale(phys.box.min, scale);
       // phys.only_overlap =true;
        //phys.velocity = Vector3Negate(Vector3Normalize(transform.translation));
        phys.mass = scale;
        phys.can_bounce = true;
        set_transform_comp(id, trans);
        set_model_comp(id, msh);
        set_physics_comp(id, phys);
    }
    get_camera()->position = (Vector3){-4,0,5};
    create_wall((Vector3){}, (Vector3){100,100,2.0}, cube_id,RED);
    //create_light((Vector3){0, 0, 8.0}, WHITE, 1.0, 100.0);
    player_id = create_character((Vector3){0,0,5}, (Vector3){1,0,0});
   //get_transform_comp(0)->transform.rotation =  quat_from_vector(Vector3Normalize(Vector3Negate( get_transform_comp(0)->transform.translation)));
}
void on_tick(){
    static u128 frame_count = 0;
    //UpdateCamera(get_camera(), CAMERA_FREE);
    if(IsKeyPressed(KEY_ESCAPE)){
        exit(0);
    }
    Vector3 target = Vector3Add(Vector3Scale(get_camera_forward_vector(),100.0), get_camera()->position);
    //CollisionResult hit = line_trace(Vector3Add(get_camera()->position, Vector3Scale(get_camera_forward_vector(), 0.6)), target, &player_id.id, 1);
    //DrawSphere(hit.hit ? hit.location : target, 0.01, BLUE);
    //String msg = tmp_string_format("%s, {%f, %f,%f}, hit_id: %u, player_id: %u", hit.hit ? "hit": "no hit", hit.hit ? hit.location.x:target.x, hit.hit ? hit.location.y : target.y, hit.hit ? hit.location.z : target.y, hit.hit_id, player_id.id);
    //log_msg(msg.items, GetFrameTime()/2.0);
    frame_count += 1;
}
void on_render(){
    DrawFPS(1500, 100);
}

int main(int argc, const char ** argv){
    ProfilerStart("dump.txt");
    tmp_init();
    init_runtime(setup, on_tick, on_render);
} 

