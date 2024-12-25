#include "../runtime.h"
#include <raymath.h>
void run_physics(){
    OptionPhysicsCompVec phys = clone(RT.physics_comps,0);
    OptionTransformCompVec trans = clone(RT.transform_comps, 0);
    for(int i =0; i<phys.length; i++){
        if(trans.items[i].is_valid&& phys.items[i].is_valid){
            trans.items[i].value.transform.translation = Vector3Add(trans.items[i].value.transform.translation, Vector3Scale(phys.items[i].value.velocity,GetFrameTime()));
        }
    }
   OptionPhysicsCompVec oldphys = RT.physics_comps;
   OptionTransformCompVec oldtrans = RT.transform_comps;
   RT.transform_comps = trans;
   RT.physics_comps = phys;
   unmake(oldphys);
   unmake(oldtrans);
}