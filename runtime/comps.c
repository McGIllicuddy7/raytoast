#include "runtime.h"
#include "default_components.h"
#include <raymath.h>
bool set_transform_comp(Ref id, TransformComp trans){
    if(id.id>=RT.entities.length){
        return false;
    }
    if(!RT.entities.items[id.id]){
        return false;
    }
    if(RT.transform_comps.items[id.id].is_valid){
        unmake(RT.transform_comps.items[id.id].value.children);
    }
    if(RT.generations.items[id.id] != id.gen_idx){
        todo();
        return 0;
    }    
    RT.transform_comps.items[id.id] = (OptionTransformComp)Some(trans);
    return true;
}
TransformComp * get_transform_comp(Ref id){
    if(id.id>RT.entities.length){
        return 0;
    }
    if(!RT.entities.items[id.id]){
        return 0;
    }  
    if(RT.generations.items[id.id] != id.gen_idx){
        return 0;
    }     
    if(RT.transform_comps.items[id.id].is_valid){
       TransformComp * cmp = &(RT.transform_comps.items[id.id].value);
       return cmp;
    }
    return 0;
}
bool remove_transform_comp(Ref id){
    if(id.id>=RT.entities.length){
        return false;
    }
    if(!RT.entities.items[id.id]){
        return false;
    }
    if(RT.generations.items[id.id] != id.gen_idx){
        return 0;
    }    
    if(RT.transform_comps.items[id.id].is_valid) unmake(RT.transform_comps.items[id.id].value.children);
    RT.transform_comps.items[id.id] = (OptionTransformComp)None;
    return true;
}

Ref get_root(Ref base){
    TransformComp * trans = get_transform_comp(base);
    if(trans){
        if(trans->parent.is_valid){
            return get_root(trans->parent.value);
        }
    }
    return base;
}
void attach_to(Ref entity, Ref parent){
   TransformComp * trans = get_transform_comp(entity);
   TransformComp * par = get_transform_comp(parent);
   v_append(par->children, entity);
   trans->parent = (OptionRef)Some(parent);
}
void detach(Ref entity){
    TransformComp * trans = get_transform_comp(entity); 
    if(trans->parent.is_valid){
        TransformComp * par = get_transform_comp(trans->parent.value);
        int idx =-1;
        for(int i =0; i<par->children.length; i++){
            if(par->children.items[i].id == entity.id){
                idx = i;
                break;
            }
        }
        if(idx != -1){
            v_remove(par->children, idx);
        }
    }
    trans->parent = (OptionRef){};
}

bool set_physics_comp(Ref id, PhysicsComp phys){
    if(id.id>=RT.entities.length){
        return false;
    }
    if(!RT.entities.items[id.id]){
        return false;
    }
    if(RT.generations.items[id.id] != id.gen_idx){
        return false;
    }
    RT.physics_comps.items[id.id].value = phys;
    RT.physics_comps.items[id.id].is_valid = true;
    return true;
}
PhysicsComp * get_physics_comp(Ref id){
    if(id.id>=RT.entities.length){
        return 0;
    }
    if(!RT.entities.items[id.id]){
        return 0;
    }
    if(RT.generations.items[id.id] != id.gen_idx){
        return 0;
    }    
    if(RT.physics_comps.items[id.id].is_valid){
       return &RT.physics_comps.items[id.id].value;
    }
    return 0;
}
bool remove_physics_comp(Ref id){
    if(id.id>=RT.entities.length){
        return false;
    }
    if(!RT.entities.items[id.id]){
        return false;
    }
    if(RT.generations.items[id.id] != id.gen_idx){
        return 0;
    }    
    RT.physics_comps.items[id.id] = (OptionPhysicsComp)None;
    return true;
}
bool transform_set_contains_child(Ref root, Ref needle){
    if(root.id == needle.id){
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

float transform_set_mass(Ref root){
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
Vector3 get_location(Ref id){
    if(get_transform_comp(id)->parent.is_valid){
        Vector3 base = Vector3RotateByQuaternion(get_transform_comp(id)->transform.translation,  get_rotation(get_transform_comp(id)->parent.value));
        return Vector3Add(base,  get_location(get_transform_comp(id)->parent.value));
    }
    return get_transform_comp(id)->transform.translation; 
}
Quaternion get_rotation(Ref id){
    if(get_transform_comp(id)->parent.is_valid){
        return QuaternionAdd(get_transform_comp(id)->transform.rotation, get_rotation(get_transform_comp(id)->parent.value));
    }
    return get_transform_comp(id)->transform.rotation;
}
Vector3 get_scale(Ref id){
    if(get_transform_comp(id)->parent.is_valid){
        return Vector3Multiply(get_transform_comp(id)->transform.scale ,get_scale(get_transform_comp(id)->parent.value));
    }
    return get_transform_comp(id)->transform.scale;
}

Vector3 get_forward_vector(Ref id){
    return Vector3RotateByQuaternion((Vector3){1,0,0}, get_rotation(id));
}

Vector3 get_up_vector(Ref id){
    return Vector3RotateByQuaternion((Vector3){0,0,1}, get_rotation(id));
}
Vector3 get_right_vector(Ref id){
    return Vector3RotateByQuaternion((Vector3){0,1,0}, get_rotation(id));
}

bool set_model_comp(Ref id, ModelComp model){
    if(id.id>=RT.entities.length){
        return false;
    }
    if(!RT.entities.items[id.id]){
        return false;
    }
    RT.model_comps.items[id.id] = (OptionModelComp)Some(model); 
    return true;
}

ModelComp * get_model_comp(Ref id){
    if(id.id>=RT.entities.length){
        return 0;
    }
    if(!RT.entities.items[id.id]){
        return 0;
    }
    if(RT.model_comps.items[id.id].is_valid){
       return &RT.model_comps.items[id.id].value;
    }
    return 0;
}

bool remove_model_comp(Ref id){
    if(id.id>=RT.entities.length){
        return false;
    }
    if(!RT.entities.items[id.id]){
        return false;
    }
    RT.model_comps.items[id.id] = (OptionModelComp)None;
    return true;
}

void add_force(Ref id, Vector3 force){
    PhysicsComp * phys = get_physics_comp(id);
    if(phys){
        phys->velocity = Vector3Add(phys->velocity, Vector3Scale(force, 1/phys->mass));
    }
}

void attach_camera_to(Ref id, Transform relative_trans){
    RT.camera_parent = (OptionRef)Some(id);
    RT.camera_relative_transform = relative_trans;
}
void detach_camera(){
    RT.camera_parent = (OptionRef)None;
}
Transform transform_default(){
    Transform out = {};
    out.scale = (Vector3){1,1,1};
    out.rotation = (Quaternion){0,0,0,1};
    return out;
}

Quaternion quat_from_vector(Vector3 loc){
    float phi = asin(loc.z);
    float theta = atan2(loc.y, loc.x);
    return QuaternionFromEuler(0, -phi,theta);
}

bool set_character_comp(Ref id, CharacterComp character_comp){
    if(id.id>RT.entities.length){
        return false;
    }
    if(!RT.entities.items[id.id]){
        return false;
    }
    RT.character_comps.items[id.id] = (OptionCharacterComp)Some(character_comp); 
    return true; 
}

CharacterComp * get_character_comp(Ref id){
    if(id.id>RT.entities.length){
        return 0;
    }
    if(!RT.entities.items[id.id]){
        return 0;
    }
    if(RT.character_comps.items[id.id].is_valid){
       return &RT.character_comps.items[id.id].value;
    }
    return 0;
}

bool remove_character_comp(Ref id){
    if(id.id>RT.entities.length){
        return false;
    }
    if(!RT.entities.items[id.id]){
        return false;
    }
    RT.character_comps.items[id.id] = (OptionCharacterComp)None;
    return true;
}

bool set_light_comp(Ref id, LightComp light){
    if(id.id>RT.entities.length){
        return false;
    }
    if(!RT.entities.items[id.id]){
        return false;
    }
    RT.light_comps.items[id.id] = (OptionLightComp)Some(light); 
    return true; 
}

LightComp* get_light_comp(Ref id){
    if(id.id>RT.entities.length){
        return 0;
    }
    if(!RT.entities.items[id.id]){
        return 0;
    }
    if(RT.light_comps.items[id.id].is_valid){
       return &RT.light_comps.items[id.id].value;
    }
    return 0;
}

bool remove_light_comp(Ref id){
    if(id.id>RT.entities.length){
        return false;
    }
    if(!RT.entities.items[id.id]){
        return false;
    }
    RT.light_comps.items[id.id] = (OptionLightComp)None;
    return true;  
}

Ref create_light(Vector3 location, Color color, float brightness, float radius){
    Entity * ent = malloc(sizeof(Entity));
    ent->vtable = &entity_default_vtable;
    Ref id = create_entity(ent).value;
    LightComp cmp;
    cmp.brightness = brightness;
    cmp.color = color;
    cmp.influence_radius = radius;
    TransformComp trans = {};
    trans.transform = transform_default();
    trans.transform.translation = location;
    trans.transform.rotation = QuaternionFromEuler(-PI/2.0, 0.0, 0.0);
    set_transform_comp(id, trans);
    set_light_comp(id,cmp);
    ModelComp msh = {};
    msh.shader_id = load_shader("shaders/base.vs", "shaders/bsdf.fs");
    msh.model_id = load_model("lightbulb.glb");
    msh.emmisive_texture_id = 0;
    msh.emmision = WHITE;
    msh.diffuse_texture_id = load_texture("lightbolb.png");
    set_model_comp(id, msh);
    return id;
}