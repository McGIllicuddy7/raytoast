use std::sync::{Arc, RwLock, RwLockReadGuard};

use raylib::prelude::RaylibDrawHandle;

use super::default_components::{MeshComp, PhysicsComp, TransformComp};


#[allow(unused)]
pub trait Entity:Send+Sync {
    fn get_tags(&self)->Arc<[String]>{
        Arc::new([])
    }
    fn on_init(&mut self, id:u32){
        
    }
    fn on_tick(&mut self, delta_time:f32){

    }
    fn on_render(&self, handle:&mut RaylibDrawHandle){

    }
    fn get_transform(&self)->Option<TransformComp>{
        super::get_transform_comp(self.get_id()?)
    }
    fn get_mesh(&self)->Option<MeshComp>{
        super::get_mesh_comp(self.get_id()?)
    }
    fn get_physics(&self)->Option<PhysicsComp>{
        super::get_physics_comp(self.get_id()?)
    }
    fn get_id(&self)->Option<u32>{
        None
    }
}  

pub struct EntityRef<'a>{
    pub values:Box<(RwLockReadGuard<'a, Vec<RwLock<Option<Box<dyn Entity+Send+Sync>>>>>,&'a dyn Entity)>,
}
impl <'a>EntityRef<'a>{
    #[allow(unused)]
    pub fn get(&self) ->&'a dyn Entity{
        self.values.1
    }
}
