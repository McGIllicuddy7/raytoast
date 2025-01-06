#include "../runtime.h"
#include "default_components.h"
#include <raymath.h>
bool set_transform_comp(u32 id, TransformComp trans){
    if(id>RT.entities.length){
        return false;
    }
    if(!RT.entities.items[id]){
        return false;
    }
    RT.transform_comps.items[id] = (OptionTransformComp)Some(trans);
    return true;
}
TransformComp * get_transform_comp(u32 id){
    if(id>=RT.entities.length){
        return 0;
    }
    if(!RT.entities.items[id]){
        return 0;
    }
    if(RT.transform_comps.items[id].is_valid){
       return &RT.transform_comps.items[id].value;
    }
    return 0;
}
bool remove_transform_comp(u32 id){
    if(id>RT.entities.length){
        return false;
    }
    if(!RT.entities.items[id]){
        return false;
    }
    RT.transform_comps.items[id] = (OptionTransformComp)None;
    return true;
}

u32 get_root(u32 base){
    TransformComp * trans = get_transform_comp(base);
    if(trans){
        if(trans->parent.is_valid){
            return get_root(trans->parent.value);
        }
    }
    return base;
}
void attach_to(u32 entity, u32 parent){
    
}
void detach(u32 entity){

}

bool set_physics_comp(u32 id, PhysicsComp phys){
    if(id>RT.entities.length){
        return false;
    }
    if(!RT.entities.items[id]){
        return false;
    }
    RT.physics_comps.items[id].value = phys;
    RT.physics_comps.items[id].is_valid = true;
    return true;
}
PhysicsComp * get_physics_comp(u32 id){
    if(id>RT.entities.length){
        return 0;
    }
    if(!RT.entities.items[id]){
        return 0;
    }
    if(RT.physics_comps.items[id].is_valid){
       return &RT.physics_comps.items[id].value;
    }
    return 0;
}
bool remove_physics_comp(u32 id){
    if(id>RT.entities.length){
        return false;
    }
    if(!RT.entities.items[id]){
        return false;
    }
    RT.physics_comps.items[id] = (OptionPhysicsComp)None;
    return true;
}
bool transform_set_contains_child(u32 root, u32 needle){
    if(root == needle){
        return true;
    }
    for(int i= 0; i<get_transform_comp(root)->children.length; i++){
        bool hit = transform_set_contains_child(get_transform_comp(root)->children.items[i],needle);
        if(hit){
            return true;
        }
    }
    return false;
}

float transform_set_mass(u32 root){
    float out = 0.0;
    if(get_physics_comp(root)){
        out += get_physics_comp(root)->mass;
    }
    TransformComp * trans = get_transform_comp(root);
    if(trans){
        for(int i =0; i<trans->children.length; i++){
            out += transform_set_mass(trans->children.items[i]);
        }
    }
    return out;
}
Vector3 get_location(u32 id){
    if(get_transform_comp(id)->parent.is_valid){
        Vector3 base = Vector3RotateByQuaternion(get_transform_comp(id)->transform.translation,  get_rotation(get_transform_comp(id)->parent.value));
        return Vector3Add(base,  get_location(get_transform_comp(id)->parent.value));
    }
    return get_transform_comp(id)->transform.translation; 
}
Quaternion get_rotation(u32 id){
    if(get_transform_comp(id)->parent.is_valid){
        return QuaternionAdd(get_transform_comp(id)->transform.rotation, get_rotation(get_transform_comp(id)->parent.value));
    }
    return get_transform_comp(id)->transform.rotation;
}
Vector3 get_scale(u32 id){
    if(get_transform_comp(id)->parent.is_valid){
        return Vector3Multiply(get_transform_comp(id)->transform.scale ,get_scale(get_transform_comp(id)->parent.value));
    }
    return get_transform_comp(id)->transform.scale;
}
Vector3 get_forward_vector(u32 id){
    return Vector3RotateByQuaternion((Vector3){1,0,0}, get_rotation(id));
}
Vector3 get_up_vector(u32 id){
    return Vector3RotateByQuaternion((Vector3){0,0,1}, get_rotation(id));
}
Vector3 get_right_vector(u32 id){
    return Vector3RotateByQuaternion((Vector3){0,1,0}, get_rotation(id));
}

bool set_model_comp(u32 id, ModelComp model){
    if(id>RT.entities.length){
        return false;
    }
    if(!RT.entities.items[id]){
        return false;
    }
    RT.model_comps.items[id] = (OptionModelComp)Some(model); 
    return true;
}
ModelComp * get_model_comp(u32 id){
    if(id>RT.entities.length){
        return 0;
    }
    if(!RT.entities.items[id]){
        return 0;
    }
    if(RT.model_comps.items[id].is_valid){
       return &RT.model_comps.items[id].value;
    }
    return 0;
}
bool remove_model_comp(u32 id){
    if(id>RT.entities.length){
        return false;
    }
    if(!RT.entities.items[id]){
        return false;
    }
    RT.model_comps.items[id] = (OptionModelComp)None;
    return true;
}
void add_force(u32 id, Vector3 force){
    PhysicsComp * phys = get_physics_comp(id);
    if(phys){
        phys->velocity = Vector3Add(phys->velocity, Vector3Scale(force, 1/phys->mass));
    }
}
