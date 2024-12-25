#include "../runtime.h"
#include <raymath.h>
#include <pthread.h>
static OptionPhysicsCompVec phys ={};
static OptionTransformCompVec trans = {};
static pthread_t phys_thread = {0};
static bool check_collision(u32 id1, u32 id2){
    BoundingBox b1 = phys.items[id1].value.box;
    BoundingBox b2 = phys.items[id2].value.box;
    b1.max = Vector3Add(b1.max, trans.items[id1].value.transform.translation);
    b2.max = Vector3Add(b2.max, trans.items[id2].value.transform.translation);
    b1.min = Vector3Add(b1.min, trans.items[id1].value.transform.translation);
    b2.min = Vector3Add(b2.min, trans.items[id2].value.transform.translation);
    return CheckCollisionBoxes(b1, b2);
}
static void substep(float dt){
    for(int i =0; i<phys.length; i++){
        if(trans.items[i].is_valid&& phys.items[i].is_valid){
            Vector3 old_loc = trans.items[i].value.transform.translation; 
            trans.items[i].value.transform.translation = Vector3Add(trans.items[i].value.transform.translation, Vector3Scale(phys.items[i].value.velocity,dt));
            bool collided = false;
            for(int j =0; j<phys.length; j++){
                if(i != j){
                    if(trans.items[j].is_valid&& phys.items[j].is_valid){ 
                        if(check_collision(i,j)){
                            collided = true;
                            break;
                        }
                    }
                }
            }
            if (collided){
               trans.items[i].value.transform.translation = old_loc; 
               phys.items[i].value.velocity = Vector3Negate(phys.items[i].value.velocity); 
               phys.items[i].value.collided_this_frame = true;
            }
        }
    }
}
static void *tick(void*){
    for(int i =0; i<10; i++){
        substep(GetFrameTime()/10.0);
    }
    return 0;
}
void run_physics(){
    phys = clone(RT.physics_comps,0);
    trans = clone(RT.transform_comps, 0);
    for(int i =0; i<phys.length; i++){
        phys.items[i].value.collided_this_frame = false;
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