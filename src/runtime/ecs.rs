use std::sync::{RwLock, RwLockReadGuard};

use raylib::{math::{BoundingBox, Transform, Vector3}, prelude::RaylibDrawHandle};

use super::default_components::{MeshComp, PhysicsComp, TransformComp};


#[allow(unused)]
pub trait Entity:Send+Sync {
    fn on_tick(&mut self, delta_time:f32, id:u32){

    }
    fn on_render(&self, handle:&mut RaylibDrawHandle){

    }
    fn get_collision(&self)->Option<BoundingBox>{
        None
    }
    fn get_transform(&self)->Option<TransformComp>{
        None
    }
    fn get_mesh(&self)->Option<MeshComp>{
        None
    }
    fn get_physics(&self)->Option<PhysicsComp>{
        None
    }
    fn get_health(&self)->Option<usize>{
        None
    }
    fn get_velocty(&self)->Option<Vector3>{
        None
    }

}  

pub struct EntityRef<'a>{
    pub values:Box<(RwLockReadGuard<'a, Vec<RwLock<Option<Box<dyn Entity+Send+Sync>>>>>,&'a dyn Entity)>,
}
impl <'a>EntityRef<'a>{
    #[allow(unused)]
    pub fn get(&self) ->&'a dyn Entity{
        return self.values.1;
    }
}
