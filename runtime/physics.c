#include "../runtime.h"
#include <raymath.h>
#include <pthread.h>
#include <stdbool.h>
#include <math.h>
#define true 1 
#define false 0
//https://iquilezles.org/articles/distfunctions/
typedef struct {
    int x;
    int y;
    int z;
}Int3;
enable_vec_type(Int3);
typedef struct{
    u32 base;
    u32 end;
    float dt;
    volatile u64* done_flag;
}PhysicsThreadArg;
typedef struct {
    Vector3 v1;
    Vector3 v2;
}VectorTuple;
size_t hash_int3(Int3 i){
    return hash_bytes((void *)&i, sizeof(Int3));
}
bool Int3_equals(Int3 a, Int3 b){
   return a.x == b.x && a.y == b.y && a.z == b.z;
}
#define TABLE_SIZE 64
static u32Vec TABLE[TABLE_SIZE][TABLE_SIZE][TABLE_SIZE] = {};
static Arena * phys_arena = 0;
float tile_size= 1.0;
float table_min = 0.0;
float table_max = 0.0;
static volatile bool phys_done = false;
static OptionPhysicsCompVec phys ={};
static OptionTransformCompVec trans = {};
static pthread_t phys_thread = {0};
#define min(a,b) a<b ? a : b
#define max(a,b) a<b ? b : a

//https://stackoverflow.com/questions/41208881/how-to-calculate-closest-distance-between-two-aabbs-vector-form
static BoundingBox bb_intersection(BoundingBox a, BoundingBox b){
    BoundingBox out;
    out.min.x = max(a.min.x, b.min.x);
    out.min.y = max(a.min.y, b.min.y);
    out.min.z = max(a.min.z, b.min.z);
    out.max.x = min(a.max.x, b.max.x);
    out.max.y = min(a.max.y, b.max.y);
    out.max.z = min(a.max.z, b.max.z);
    return out;
}
static float bb_distance(BoundingBox a, BoundingBox b){
    BoundingBox ibox = bb_intersection(a,b);
    float distance = 0.0;
    if(ibox.min.x>ibox.max.x){
        distance += ( ibox.min.x - ibox.max.x );
    }
    if(ibox.min.y>ibox.max.y){
        distance += ( ibox.min.y - ibox.max.y );
    }
    if(ibox.min.z>ibox.max.z){
        distance += ( ibox.min.z - ibox.max.z );
    }
    return distance;
}
static int u32Vec_find(u32Vec vec, u32 needle){
    for(int i=0; i<vec.length; i++){
        if(vec.items[i] == needle){
            return i;
        }
    }
    return -1;
}
static int u32Vec_find_cap(u32Vec vec, u32 needle){
    for(int i=0; i<vec.capacity; i++){
        if(vec.items[i] == needle){
            return i;
        }
    }
    return -1;
}
static void u32Vec_print(u32Vec vec){
    for(int i =0; i<vec.length; i++){
        if(i != vec.length-1){
            printf("%u, ", vec.items[i]);
        } else{
            printf("%u\n", vec.items[i]);
        }
    }
}
static Vector3 box_collision_normal_vector(BoundingBox a, BoundingBox b){
    Vector3 norms[] = {{1,0,0}, {-1, 0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1}};
    Vector3 av = Vector3Scale(Vector3Add(a.max, a.min), 0.5);
    Vector3 bv = Vector3Scale(Vector3Add(b.max, b.min), 0.5);
    Vector3 del = Vector3Subtract(bv, av);
    float length = Vector3Length(del);
    if(length<0.0001){
        assert(false);
        return (Vector3){0,0,1};
    }
    Vector3 norm = Vector3Scale(del, 1/length);
    int idx = 0;
    float min = 1000000.0;
    for(int i =0; i<6; i++){
        float dot = Vector3DotProduct(norm, norms[i]);
        if(dot<min){
            idx =i;
            min = dot;
        }
    }
    return norms[idx];
}
static Int3 location_to_int3(Vector3 loc){
    float dx = loc.x-table_min;
    float dy = loc.y-table_min;
    float dz = loc.z-table_min;
    int x = dx/tile_size;
    int y = dy/tile_size;
    int z = dz/tile_size;
    if(x<0){
        x =0;
    }
    if(y<0){
        y = 0;
    }
    if(z<0){
        z =0;
    }
    if(x>=TABLE_SIZE){
        x=TABLE_SIZE-1;
    }
    if(y>=TABLE_SIZE){
        y =TABLE_SIZE-1;
    }
    if(z>=TABLE_SIZE){
        z =TABLE_SIZE-1;
    }
    return (Int3){x,y,z};
}
static float min_dimension(BoundingBox bx){
    return min((bx.max.x-bx.min.x),min((bx.max.y-bx.min.y), (bx.max.z-bx.min.z)));
}
static float max_allowed_distance_array(u32 id, u32Vec cmps){
    float min = tile_size;
    for(int i = 0; i<cmps.length; i++){
        u32 id1 = cmps.items[i];
        if(!trans.items[id1].is_valid){
            continue;
        }
        if(id1 == id){
            continue;
        }
        Vector3 t1 = trans.items[id].value.transform.translation;
        Vector3 t2 = trans.items[id1].value.transform.translation;
        BoundingBox b1 = phys.items[id].value.box;
        BoundingBox b2 = phys.items[id1].value.box;
        if(Vector3Distance(t1, t2)>min*2.0){
            continue;
        }
        b1.max = Vector3Add(b1.max, trans.items[id].value.transform.translation);
        b2.max = Vector3Add(b2.max, trans.items[id1].value.transform.translation);
        b1.min = Vector3Add(b1.min, trans.items[id].value.transform.translation);
        b2.min = Vector3Add(b2.min, trans.items[id1].value.transform.translation);
        float dist = bb_distance(b1, b2);
        if(dist<min){
            min = dist;
        }
    }
    return min;
}
static float max_allowed_distance(u32 id){
    float min = tile_size;
    int count =2;
    Int3 lc = location_to_int3(trans.items[id].value.transform.translation);
    for(int dz = -count; dz<= count; dz++){
        for(int dy = -count; dy<=count; dy++){
            for(int dx = -count; dx<=count; dx++){
                if(lc.x+dx <0 || lc.x+dx>=TABLE_SIZE || lc.y+dy<0 || lc.y+dy>=TABLE_SIZE || lc.z+dz<0 || lc.z+dz>=TABLE_SIZE){
                    continue;
                }
                
                u32Vec cmps = TABLE[lc.z+dz][lc.y+dy][lc.x+dx];
                float fmin = max_allowed_distance_array(id, cmps);
                if(fmin<min){
                    min = fmin;
                }
            }
        }
    }
    return min;
}
static bool check_hit_array(u32 id, u32Vec cmps,Vector3 * normal,u32 * other_hit){
    int count =2;
    for(int i = 0; i<cmps.length; i++){
        u32 id1 = cmps.items[i];
        if(id1 == id){
            continue;
        }
        if(!trans.items[id1].is_valid){
            continue;
        }
        Vector3 t1 = trans.items[id].value.transform.translation;
        Vector3 t2 = trans.items[id1].value.transform.translation;
        BoundingBox b1 = phys.items[id].value.box;
        BoundingBox b2 = phys.items[id1].value.box;
        if(Vector3Distance(t1, t2)>(Vector3Distance(b1.max, b1.min)+Vector3Distance(b2.max, b2.min))*2.0){
            continue;
        }


        b1.max = Vector3Add(b1.max, trans.items[id].value.transform.translation);
        b2.max = Vector3Add(b2.max, trans.items[id1].value.transform.translation);
        b1.min = Vector3Add(b1.min, trans.items[id].value.transform.translation);
        b2.min = Vector3Add(b2.min, trans.items[id1].value.transform.translation);
        bool hit = CheckCollisionBoxes(b1, b2);
        if(hit){
            const Vector3 norm = box_collision_normal_vector(b1, b2);
            *normal=  norm;
            *other_hit = id1;
            return true;
        }
    }
    return false;
}
static bool check_hit(u32 id, Vector3 * normal,u32 * other_hit){
    int count =1;
    Int3 lc = location_to_int3(trans.items[id].value.transform.translation);
    bool hit0 = false;
    for(int dz = -count; dz<= count; dz++){
        for(int dy = -count; dy<=count; dy++){
            for(int dx = -count; dx<=count; dx++){
                if(lc.x+dx <0 || lc.x+dx>=TABLE_SIZE || lc.y+dy<0 || lc.y+dy>=TABLE_SIZE || lc.z+dz<0 || lc.z+dz>=TABLE_SIZE){
                    continue;
                }
                if(Int3_equals(lc, (Int3){lc.x+dx, lc.y+dy, lc.z+dz})){
                    hit0 = true;
                }
                u32Vec cmps = TABLE[lc.z+dz][lc.y+dy][lc.x+dx];
                bool hit = check_hit_array(id, cmps,normal, other_hit);
                if(hit){
                    return true;
                }
            }
        }
    }
    assert(hit0);
    return false;
}

static bool check_hit_non_opt(u32 id, Vector3 * normal, u32 * other_hit){
    u32Vec cmps = make_with_cap(0, u32, phys.length/2);
    for(int i =0; i<phys.length; i++){
        if(phys.items[i].is_valid){
            v_append(cmps, i);
        }
    }
    bool hit = check_hit_array(id, cmps, normal, other_hit);
    unmake(cmps);
    if(hit){
        Int3 aloc = location_to_int3(trans.items[id].value.transform.translation);
        Int3  bloc =  location_to_int3(trans.items[*other_hit].value.transform.translation);
        Vector3 a = (Vector3){aloc.x, aloc.y, aloc.z};
        Vector3 b = (Vector3){bloc.x, bloc.y, bloc.z};
       // printf("aloc:{%d, %d,%d}, bloc:{%d,%d,%d}, distance:%f\n", aloc.x, aloc.y, aloc.z, bloc.x, bloc.y, bloc.z, Vector3Distance(a,b));

    }
    return hit;
}

static float max_allowed_distance_non_opt(u32 id){
    u32Vec cmps = make_with_cap(0, u32, phys.length/2);
    for(int i =0; i<phys.length; i++){
        if(phys.items[i].is_valid){
            v_append(cmps, i);
        }
    }
    float dist = max_allowed_distance_array(id, cmps);
    unmake(cmps);
    return dist;
}

static VectorTuple calc_hit_impulses(u32 id1, u32 id2, Vector3 normal_vector,float drag){
    //va2 = (ma-mb)/(ma+mb)va1+(2mb)/(ma+mb)vb1
    //vb2 = (2ma)/(ma+mb)va1+(mb-ma)/(ma+mb)vb1
    const Vector3 base_a1 = get_physics_comp(id1)->velocity;
    const Vector3 base_b1= get_physics_comp(id2)->velocity;
    const float va1 = Vector3DotProduct(get_physics_comp(id1)->velocity, normal_vector);
    const float vb1 = Vector3DotProduct(get_physics_comp(id2)->velocity, normal_vector);
    const float ma = get_physics_comp(id1)->mass;
    const float mb = get_physics_comp(id2)->mass;
    const float va2 = ((ma-mb)/(ma+mb)*va1+(2*mb)/(ma+mb)*vb1)*drag; 
    const float vb2 = ((2*ma)/(ma+mb)*va1+(mb-ma)/(ma+mb)*vb1)*drag;
    const float dela = va2-va1;
    const float delb = vb2-vb1;
    const Vector3 va =Vector3Scale(normal_vector, dela);
    const Vector3 vb = Vector3Scale(normal_vector, delb);
    return (VectorTuple){Vector3Add(base_a1, va),Vector3Add(base_b1, vb)};
}
static void store_locations(u32 id){
    BoundingBox box = phys.items[id].value.box;
    box.min = Vector3Add(box.min , trans.items[id].value.transform.translation);
    box.max = Vector3Add(box.max , trans.items[id].value.transform.translation);
    int dx = ceil((box.max.x-box.min.x)/tile_size);
    int dy = ceil((box.max.y-box.min.y)/tile_size); 
    int dz = ceil((box.max.z-box.min.z)/tile_size); 
    Int3 loc = location_to_int3(box.min);
    for(int z =0; z<dz; z++){
        for(int y =0; y<dy; y++){
            for(int x= 0; x<dx; x++){
                if(x+loc.x >=TABLE_SIZE || loc.x+x <0|| loc.y+y>=TABLE_SIZE || loc.y+y<0 || loc.z+z >= TABLE_SIZE || loc.z+z<0){
                    continue;
                }
                v_append(TABLE[z+loc.z][y+loc.y][x+loc.x], id);
            }
        }
    }
}
static void collision_iter(PhysicsComp * comp, Transform * transform, u32 id, float dt){
    dt = 0.01666;
    if(!comp->movable){
        return;
    }
    if(Vector3Equals(comp->velocity, (Vector3){0.0, 0.0, 0.0})){
        return;
    }
    Vector3 col_norm;
    u32 collision_id;
    //assert(!check_hit_non_opt(id, &col_norm, &collision_id));
    float max_travelled_dist = Vector3Length(phys.items[id].value.velocity)*dt;
    float distance_travelled = 0.0;
    Vector3 norm = Vector3Normalize(phys.items[id].value.velocity);
    float min = min_dimension(comp->box);
    Int3 key = location_to_int3(trans.items[id].value.transform.translation);
    Vector3 old_location = trans.items[id].value.transform.translation  ;
    while(distance_travelled<max_travelled_dist){
        float max_distance = max_allowed_distance(id);
        if(max_distance>tile_size){max_distance = tile_size;}
        bool check = false;
        if(max_distance <= tile_size){
            assert(max_distance>-0.1);
            max_distance = min;
            check = true;
        }
        Vector3 displacement;
        if(max_travelled_dist-distance_travelled>0){
            displacement = Vector3Scale(norm,max_travelled_dist-distance_travelled);
            distance_travelled = max_travelled_dist;
        } else{
            displacement = Vector3Scale(norm, max_distance);
        }
        trans.items[id].value.transform.translation = Vector3Add(trans.items[id].value.transform.translation, displacement);
        distance_travelled += Vector3Length(displacement);
        if(check){
            u32 other = 0;
            Vector3 norm = {};
            bool hit = check_hit(id, &norm, &other);
            if(hit){
                trans.items[id].value.transform.translation = old_location;
                u32 other_old = other;
                Vector3 old_norm = norm;
                if(check_hit(id, &norm, &other)){
                    assert(false);
                    if(Vector3Length(norm)<0.00001){
                        norm =(Vector3){1,0,0};
                    }
                    trans.items[id].value.transform.translation = Vector3Add(trans.items[id].value.transform.translation, Vector3Scale(norm,0.01));
                    other_old = other;
                    old_norm = norm;
                }
                if(!get_physics_comp(other_old)){
                    phys.items[id].value.velocity = Vector3Negate(  phys.items[id].value.velocity);
                    assert(false);
                    break;
                }
                if(get_physics_comp(other_old)->movable){
                    VectorTuple tup = calc_hit_impulses(id, other_old, old_norm, 1.0);
                    phys.items[id].value.velocity = tup.v1;
                    phys.items[other_old].value.velocity= tup.v2;
                } else{
                    printf("hit");
                    printf("normal: {%f, %f, %f} ", old_norm.x, old_norm.y, old_norm.z);
                    Vector3 v0 = phys.items[id].value.velocity;
                    printf("first vel:{%f, %f, %f} ", v0.x, v0.y, v0.z);
                    phys.items[id].value.velocity = Vector3Reflect(  phys.items[id].value.velocity, old_norm);
                    v0 = phys.items[id].value.velocity;
                    printf("seconf vel:{%f, %f, %f}\n", v0.x, v0.y, v0.z);
                }
                break;
            }
        }
    }
    store_locations(id);
}
static void run_single(float dt){
    float min = 0.0;
    float max = 0.0;
    bool hit = false;
    for(int i =0; i<phys.length; i++){
        if(trans.items[i].is_valid&& phys.items[i].is_valid){
            Vector3 value = trans.items[i].value.transform.translation;
            if(!hit){
                min = min(value.x, min(value.y, value.z));
                max = max(value.x, max(value.y, value.z));
            } else{
                if(value.x<min){
                    min= value.x;
                }
                if(value.y<min){
                    min = value.y;
                } 
                if(value.z<min){
                    min = value.z;
                }
                if(value.x>max){
                    max = value.x;
                }
                if(value.y>max){
                    max = value.y;
                }
                if(value.z>max){
                    max = value.z;
                }
            }
            hit = true;
        }
    }
    if(!hit){
        return;
    }
    tile_size = (max-min)/TABLE_SIZE;
    if(tile_size<0.5){
        tile_size = 0.5;
    }
    table_max = max;
    table_min = min;
    for(int i =0; i<phys.length; i++){
        store_locations(i);
    }
    for(int i =0; i<phys.length; i++){
        if(trans.items[i].is_valid&& phys.items[i].is_valid){
            if(!phys.items[i].value.movable){
                continue;
            }
            collision_iter(&phys.items[i].value, &trans.items[i].value.transform, i, dt);
        }
    }
}
static void *tick(void*){
    arena_reset(phys_arena);
    for(int z = 0; z<TABLE_SIZE; z++){
        for(int y =0; y<TABLE_SIZE; y++){
            for(int x =0; x<TABLE_SIZE; x++){
                //unmake(TABLE[z][y][x]);
                TABLE[z][y][x] = (u32Vec)make(phys_arena, u32);
            }
        }
    }
    if(phys.capacity < RT.physics_comps.capacity){
        OptionPhysicsComp * old = phys.items;
        phys.items = arena_alloc(0, sizeof(OptionPhysicsComp)*RT.physics_comps.capacity);
        phys.capacity = RT.physics_comps.capacity;
        arena_free(0, old);
    } 
    if(trans.capacity < RT.transform_comps.capacity){
        OptionTransformComp * old = trans.items;
        trans.items = arena_alloc(0, sizeof(OptionTransformComp)*RT.transform_comps.capacity);
        trans.capacity = RT.transform_comps.capacity;
        arena_free(0, old);
    } 
    phys.length = RT.physics_comps.length;
    memcpy(phys.items, RT.physics_comps.items, sizeof(OptionPhysicsComp)*phys.length);
    trans.length = RT.transform_comps.length;
    memcpy(trans.items, RT.transform_comps.items, sizeof(OptionTransformComp)*trans.length);
    run_single(GetFrameTime());
    phys_done = true;
    return 0;
}
Arena * arena_create_sized(size_t reqsize);
void init_physics_rt(){
    phys_arena = arena_create_sized(sizeof(PhysicsComp)*10000);
    for(int z = 0; z<TABLE_SIZE; z++){
        for(int y =0; y<TABLE_SIZE; y++){
            for(int x =0; x<TABLE_SIZE; x++){
                TABLE[z][y][x] = (u32Vec)make(phys_arena, u32);
            }
        }
    }
}
void run_physics(){
    phys_done = false;
    //tick(0);
    pthread_create(&phys_thread, 0, tick, 0);
}
void finish_physics(){
    while(!phys_done){}
    phys_done = false;
    pthread_join(phys_thread,0);
    RT.transform_comps = clone(trans,0);
    RT.physics_comps = clone(phys,0);
}