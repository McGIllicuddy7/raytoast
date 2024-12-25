#include "../runtime.h"
#include <raymath.h>
#include <pthread.h>
//https://iquilezles.org/articles/distfunctions/
typedef struct {
    int x;
    int y;
    int z;
}Int3;
size_t hash_int3(Int3 i){
    return hash_bytes((void *)&i, sizeof(Int3));
}
bool Int3_equals(Int3 a, Int3 b){
   return a.x == b.x && a.y == b.y && a.z == b.z;
}
enable_vec_type(u32);
enable_hash_type(Int3, u32Vec);
static Int3u32VecHashTable *table = 0;
const float tile_size= 1.0;
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
static bool check_collision(u32 id1, u32 id2){
    BoundingBox b1 = phys.items[id1].value.box;
    BoundingBox b2 = phys.items[id2].value.box;
    b1.max = Vector3Add(b1.max, trans.items[id1].value.transform.translation);
    b2.max = Vector3Add(b2.max, RT.transform_comps.items[id2].value.transform.translation);
    b1.min = Vector3Add(b1.min, trans.items[id1].value.transform.translation);
    b2.min = Vector3Add(b2.min, RT.transform_comps.items[id2].value.transform.translation);
    return CheckCollisionBoxes(b1, b2);
}
static float max_allowed_distance(u32 id){
    float min = tile_size;
    Vector3 t = Vector3Scale(trans.items[id].value.transform.translation, 1/tile_size);
    Int3 xyz = {t.x, t.y, t.z};
    int count = 1;
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
                        b2.max = Vector3Add(b2.max, RT.transform_comps.items[id1].value.transform.translation);
                        b1.min = Vector3Add(b1.min, trans.items[id].value.transform.translation);
                        b2.min = Vector3Add(b2.min, RT.transform_comps.items[id1].value.transform.translation);
                        float dist = bb_distance(b1, b2);
                        Vector3 delta = Vector3Subtract(Vector3Scale(Vector3Add(b1.max,b1.min), 0.5), Vector3Scale(Vector3Add(b2.max,b2.min),0.5));
                        if(dist<min){
                            if(dist>=0.05){
                                min = dist;
                            }
                            else{
                                Vector3 displacement = Vector3Scale(Vector3Normalize(phys.items[id].value.velocity), 0.01);
                                BoundingBox b3;
                                b3.max = Vector3Add(b1.max, displacement);
                                b3.min = Vector3Add(b1.min, displacement);
                                float dist2 = bb_distance(b3,b2);
                                if(dist2<dist){
                                    min = dist;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return min;
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
    while(distance_travelled<max_travelled_dist){
        float max_distance = max_allowed_distance(id);
        if(max_distance <0.01){
            comp->velocity = Vector3Negate(comp->velocity);
            break;
        }
        Vector3 displacement;
        if(max_distance>= max_travelled_dist-distance_travelled){
            displacement = Vector3Scale(norm,max_travelled_dist-distance_travelled);
            distance_travelled = max_travelled_dist;
        } else{
            displacement = Vector3Scale(norm, max_distance);
        }
        distance_travelled += Vector3Length(displacement);
        trans.items[id].value.transform.translation = Vector3Add(trans.items[id].value.transform.translation, displacement);
    }
}
static void substep(float dt){
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
    table = Int3u32VecHashTable_create(10000, hash_int3, Int3_equals);
    for(int i =0; i<phys.length; i++){
        Vector3 v = Vector3Scale(RT.transform_comps.items[i].value.transform.translation, 1/tile_size);
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
    const int steps = 1;
    for(int i =0; i<steps; i++){
        substep(GetFrameTime()/steps);
    }
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
    for(int i =0; i<phys.length; i++){
        assert(Vector3Equals(phys.items[i].value.box.max, RT.physics_comps.items[i].value.box.max));
    }
    OptionPhysicsCompVec oldphys = RT.physics_comps;
    OptionTransformCompVec oldtrans = RT.transform_comps;
    RT.transform_comps = trans;
    RT.physics_comps = phys;
    unmake(oldphys);
    unmake(oldtrans);
}