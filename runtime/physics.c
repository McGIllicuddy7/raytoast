#include "../runtime.h"
#include <raymath.h>
#include <pthread.h>
OptionPhysicsCompVec phys ={};
OptionTransformCompVec trans = {};
pthread_t phys_thread = {0};
static void *tick(void*){;
    for(int i =0; i<phys.length; i++){
        if(trans.items[i].is_valid&& phys.items[i].is_valid){
            trans.items[i].value.transform.translation = Vector3Add(trans.items[i].value.transform.translation, Vector3Scale(phys.items[i].value.velocity,GetFrameTime()));
        }
    }
    return 0;
}
void run_physics(){
    phys = clone(RT.physics_comps,0);
    trans = clone(RT.transform_comps, 0);
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