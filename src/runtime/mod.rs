use std::sync::{RwLock, RwLockReadGuard, RwLockWriteGuard};

use ecs::Entity;
use raylib::{color, prelude::RaylibDraw, RaylibHandle, RaylibThread};

pub mod default_components;
pub mod ecs;
pub struct Runtime{
    pub entities: RwLock<Vec<RwLock<Option<Box<dyn Entity>>>>>, 
}
impl Runtime{
    pub const fn new()->Self{
        Self { entities: RwLock::new(Vec::new()) }
    }
    pub fn read_entities<'a>(&'a self)->RwLockReadGuard<'a, Vec<RwLock<Option<Box<dyn Entity>>>>>{
        self.entities.read().expect("works")
    }
    pub fn write_entities<'a>(&'a self)->RwLockWriteGuard<'a, Vec<RwLock<Option<Box<dyn Entity>>>>>{
        self.entities.write().expect("works")
    }
    pub fn run_tick(&self, delta_time:f32){
        let ents = self.read_entities();
        let mut id = 0;
        for i in ents.iter(){
            let mut ent_opt = i.write().expect("works");
            if let Some(ent) = ent_opt.as_mut(){
                ent.on_tick(delta_time, id);
            }
            id+=1;
        }
    }
    pub fn run_render(&self,handle:&mut RaylibHandle, thread:&mut RaylibThread){
        let mut draw = handle.begin_drawing(thread);
        draw.clear_background(color::Color::BLACK);
        
    }
    pub fn run(&self){
        let (mut handle, mut thread) = raylib::prelude::RaylibBuilder::default().size(1024, 1024).title("raytoast").vsync().build();
        while !handle.window_should_close(){
            self.run_tick(handle.get_frame_time());
            self.run_render(&mut handle, &mut thread);
        }
    }
}

#[allow(unused)]
static RT:Runtime = Runtime::new();
pub fn run(){
    RT.run();
}