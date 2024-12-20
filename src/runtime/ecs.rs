use std::sync::{RwLock, RwLockReadGuard};

pub trait Entity:Send+Sync {
    fn on_tick(&mut self, delta_time:f32, id:usize);
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
