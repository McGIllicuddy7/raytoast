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
enable_vec_type(u32);
enable_hash_type(Int3, u32Vec);
static Int3u32VecHashTable *table = 0;
const float tile_size= 0.5;
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
static Vector3 box_collision_normal_vector(BoundingBox a, BoundingBox b){
    Vector3 norms[] = {{1,0,0}, {-1, 0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1}};
    Vector3 av = Vector3Scale(Vector3Add(a.max, a.min), 0.5);
    Vector3 bv = Vector3Scale(Vector3Add(b.max, b.min), 0.5);
    Vector3 del = Vector3Subtract(bv, av);
    float length = Vector3Length(del);
    if(length<0.0001){
        return (Vector3){1,0,0};
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
static float min_dimension(BoundingBox bx){
    return min((bx.max.x-bx.min.x),min((bx.max.y-bx.min.y), (bx.max.z-bx.min.z)));
}

static float max_allowed_distance(u32 id){
    float min = tile_size;
    Vector3 t = Vector3Scale(trans.items[id].value.transform.translation, 1/tile_size);
    Int3 xyz = {t.x, t.y, t.z};
    int count = 2;
    for(int dx = -count; dx<=count; dx++){
        for(int dy = -count; dy<=count; dy++){
            for(int dz = -count; dz<=count;dz++ ){
                Int3 v = {xyz.x+dx, xyz.y+dy, xyz.z+dz};
                u32Vec * vec = Int3u32VecHashTable_find(table, v);
                if(vec){
                    for(int i =0; i<vec->length; i++){
                        u32 id1 = vec->items[i];
                        if(id1 == id){
                            continue;
                        }
                        BoundingBox b1 = phys.items[id].value.box;
                        BoundingBox b2 = phys.items[id1].value.box;
                        b1.max = Vector3Add(b1.max, trans.items[id].value.transform.translation);
                        b2.max = Vector3Add(b2.max, trans.items[id1].value.transform.translation);
                        b1.min = Vector3Add(b1.min, trans.items[id].value.transform.translation);
                        b2.min = Vector3Add(b2.min, trans.items[id1].value.transform.translation);
                        float dist = bb_distance(b1, b2);
                        if(dist<min){
                            min = dist;
                        }
                    }
                }
            }
        }
    }
    return min;
}
static bool check_hit(u32 id, Vector3 * normal,u32 * other_hit){
    Vector3 t = Vector3Scale(trans.items[id].value.transform.translation, 1/tile_size);
    Int3 xyz = {t.x, t.y, t.z};
    int count = 2;
    for(int dx = -count; dx<=count; dx++){
        for(int dy = -count; dy<=count; dy++){
            for(int dz = -count; dz<=count;dz++ ){
                Int3 v = {xyz.x+dx, xyz.y+dy, xyz.z+dz};
                u32Vec * vec = Int3u32VecHashTable_find(table, v);
                if(vec){
                    for(int i =0; i<vec->length; i++){
                        u32 id1 = vec->items[i];
                        if(id1 == id){
                            continue;
                        }
                        BoundingBox b1 = phys.items[id].value.box;
                        BoundingBox b2 = phys.items[id1].value.box;
                        b1.max = Vector3Add(b1.max, trans.items[id].value.transform.translation);
                        b2.max = Vector3Add(b2.max, trans.items[id1].value.transform.translation);
                        b1.min = Vector3Add(b1.min, trans.items[id].value.transform.translation);
                        b2.min = Vector3Add(b2.min, trans.items[id1].value.transform.translation);
                        bool hit = CheckCollisionBoxes(b1, b2);
                        if(hit){
                            const Vector3 norm = box_collision_normal_vector(b1, b2);
                            *normal=  norm;
                            *other_hit = i;
                            return true;
                        }
                
                    }
                }
            }
        }
    }
    return false;
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
static void collision_iter(PhysicsComp * comp, Transform * transform, u32 id, float dt){
    if(!comp->movable){
        return;
    }
    if(Vector3Equals(comp->velocity, (Vector3){0.0, 0.0, 0.0})){
        return;
    }
    float max_travelled_dist = Vector3Length(Vector3Scale(comp->velocity, dt));
    float distance_travelled = 0.0;
    Vector3 norm = Vector3Normalize(comp->velocity);
    float min = min_dimension(comp->box);
    Vector3 v = Vector3Scale(trans.items[id].value.transform.translation, 1/tile_size);
    Int3 key = {v.x, v.y, v.z}; 
    while(distance_travelled<max_travelled_dist){
        Vector3 col_norm;
        u32 collision_id;
        float max_distance = max_allowed_distance(id);
        bool check = false;
        Vector3 old_location = trans.items[id].value.transform.translation ; 
        if(max_distance <= 0.01){
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
                int other_old = other;
                Vector3 old_norm = norm;
                if(check_hit(id, &norm, &other)){
                    trans.items[id].value.transform.translation = Vector3Add(trans.items[id].value.transform.translation, Vector3Scale(norm, 0.01));
                    other_old = other;
                    old_norm = norm;
                }
                if(get_physics_comp(other_old)->movable){
                    VectorTuple tup = calc_hit_impulses(id, other_old, old_norm, 1.0);
                    comp->velocity = tup.v1;
                    get_physics_comp(other_old)->velocity = tup.v2;
                } else{
                    comp->velocity = Vector3Reflect(comp->velocity, old_norm);
                }
                break;
            }
        }

    }
    Vector3 v0 = Vector3Scale(trans.items[id].value.transform.translation, 1/tile_size);
    Int3 key0 = {v.x, v.y, v.z}; 
    if(!Int3_equals(key0, key)){
        u32Vec *vc = Int3u32VecHashTable_find(table,key);
        if(vc){
            int idx = -1;
            for(int i =0; i<vc->length; i++){
                if(vc->items[i] == id){
                    idx = i;
                }
            }
            v_remove((*vc), idx);
        }
        vc = Int3u32VecHashTable_find(table,key0); 
        if(vc){
            v_append((*vc), id);
        } else{
            u32Vec vec = make(0, u32);
            v_append(vec, id);
            Int3u32VecHashTable_insert(table, key0, vec);
        }
    }
}
static void run_single(float dt){
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
    table = Int3u32VecHashTable_create(1000, hash_int3, Int3_equals);
    for(int i =0; i<phys.length; i++){
        Vector3 v = Vector3Scale(trans.items[i].value.transform.translation, 1/tile_size);
        Int3 key = {v.x, v.y, v.z};
        if(!Int3u32VecHashTable_contains(table, key)){
            u32Vec v = make(0, u32);
            v_append(v, i);
            Int3u32VecHashTable_insert(table, key, v);
        } else{
           u32Vec * v = Int3u32VecHashTable_find(table, key);
           v_append((*v), i);
        }
    }
    //run_single(GetFrameTime());
    run_single(GetFrameTime());
    Int3u32VecHashTable_unmake(table);
    table = 0;
    return 0;
}
void run_physics(){
    phys = clone(RT.physics_comps,0);
    trans = clone(RT.transform_comps, 0);
    for(int i =0; i<phys.length; i++){
        phys.items[i].value.collided_this_frame = false;
        assert(Vector3Equals(trans.items[i].value.transform.translation, RT.transform_comps.items[i].value.transform.translation));
    }
    pthread_create(&phys_thread, 0, tick, 0);

}
void finish_physics(){
    pthread_join(phys_thread,0);
    OptionPhysicsCompVec oldphys = RT.physics_comps;
    OptionTransformCompVec oldtrans = RT.transform_comps;
    RT.transform_comps = trans;
    RT.physics_comps = phys;
    unmake(oldphys);
    unmake(oldtrans);
}