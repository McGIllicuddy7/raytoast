
use raylib::{color::Color, math::{Transform, Vector3, Vector4, BoundingBox}};

use super::{get_transform_comp, RT};

#[derive(Clone, Copy)]
pub struct TransformComp{
    pub location:Transform, 
    pub relative_location:Transform, 
    pub parent_id:Option<u32>,
}
impl TransformComp{
    pub fn get_relative_location(&self)->Vector3{
        if let Some(_) = self.parent_id{
            return self.relative_location.translation;
        } else{
            return  Vector3::new(0.0, 0.0, 0.0);
        }
    }
    pub fn get_world_location(&self)->Vector3{
        if let Some(id) = self.parent_id{
            return self.relative_location.translation+get_transform_comp(id).expect("msg").get_world_location();
        } else{
            return self.location.translation;
        }
    }
    pub fn get_relative_rotation(&self)->Vector4{
        if let Some(_) = self.parent_id{
            return self.relative_location.rotation;
        } else{
            return  Vector4::new(0.0, 0.0, 0.0,1.0);
        }
    }
    pub fn get_world_rotation(&self)->Vector4{
        if let Some(id) = self.parent_id{
            return self.relative_location.rotation*get_transform_comp(id).expect("msg").get_world_rotation();
        } else{
            return self.location.rotation;
        }
    }
    pub fn get_relative_scale(&self)->Vector3{
        if let Some(_) = self.parent_id{
            return self.relative_location.scale
        } else{
            return Vector3::new(1.0, 1.0, 1.0,);
        }
    }
    pub fn get_world_scale(&self)->Vector3{
        if let Some(id) = self.parent_id{
            return self.relative_location.scale*get_transform_comp(id).expect("msg").get_world_scale()
        } else{
            return self.location.scale;
        }
    }
    pub fn get_id(&self)->u32{
        let s = self as *const Self as *const Option<Self>;
        let base = RT.transform_comps.read().expect("msg").values.as_ptr();
        return unsafe{ s.offset_from(base)} as u32
    }
}

#[derive(Clone, Copy)]
//requires transform
pub struct PhysicsComp{
    pub velocity:Vector3, 
    pub bounds:BoundingBox,
    pub mass:f32,
    pub movable:bool,
}
impl PhysicsComp{
    pub fn get_id(&self)->u32{
        let s = self as *const Self as *const Option<Self>;
        let base = RT.physics_comps.read().expect("msg").values.as_ptr();
        return unsafe{ s.offset_from(base)} as u32
    }
    pub fn get_transform(&self)->TransformComp{
        get_transform_comp(self.get_id()).expect("required")
    }
    pub fn get_location(&self)->Vector3{
        self.get_transform().get_world_location()
    }
    pub fn get_bounds(&self)->BoundingBox{
        let bmin = self.bounds.min;
        let bmax = self.bounds.max;
        let scale = self.get_transform().get_world_scale();
        let loc = self.get_location();
        let min = Vector3::new(bmin.x*scale.x, bmin.y*scale.y, bmin.z*scale.z)+loc;
        let max = Vector3::new(bmax.x*scale.x, bmax.y*scale.y, bmax.z*scale.z)+loc;
        let out = BoundingBox{min:min.into(), max:max.into()};
        super::physics::rotate_bounding_box(out,self.get_transform().get_world_rotation())
    }
}

#[derive(Clone, Copy)]
//requires transform
pub struct MeshComp{
    pub mesh:u32,
    pub mat:u32,
}
impl MeshComp{
    pub fn get_id(&self)->u32{
        let s = self as *const Self as *const Option<Self>;
        let base = RT.mesh_comps.read().expect("msg").values.as_ptr();
        return unsafe{ s.offset_from(base)} as u32
    }
}

#[derive(Clone, Copy)]
//requires transform
pub struct LightComp{
    pub color:Color, 
}
