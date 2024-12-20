use std::sync::{RwLock, RwLockReadGuard, RwLockWriteGuard};

use ecs::{Entity, EntityRef};
use raylib::{color, prelude::{RaylibDraw, RaylibDrawHandle}, RaylibHandle, RaylibThread};

pub mod default_components;
pub mod ecs;
pub struct Runtime{
    pub entities: RwLock<Vec<RwLock<Option<Box<dyn Entity+Send+Sync>>>>>, 
}
impl Runtime{
    pub const fn new()->Self{
        Self { entities: RwLock::new(Vec::new()) }
    }
    pub fn read_entities<'a>(&'a self)->RwLockReadGuard<'a, Vec<RwLock<Option<Box<dyn Entity+Send+Sync>>>>>{
        self.entities.read().expect("works")
    }
    pub fn write_entities<'a>(&'a self)->RwLockWriteGuard<'a, Vec<RwLock<Option<Box<dyn Entity+Send+Sync>>>>>{
        self.entities.write().expect("works")
    }
    pub fn run_tick(&self, delta_time:f32, on_tick:&dyn Fn()){
        let ents = self.read_entities();
        let mut id = 0;
        on_tick();
        for i in ents.iter(){
            let mut ent_opt = i.write().expect("works");
            if let Some(ent) = ent_opt.as_mut(){
                ent.on_tick(delta_time, id);
            }
            id+=1;
        }
    }
    pub fn run_render(&self,handle:&mut RaylibHandle, thread:&mut RaylibThread, on_draw:&dyn Fn(&mut RaylibDrawHandle)){
        let mut draw = handle.begin_drawing(thread);
        draw.clear_background(color::Color::BLACK);
        on_draw(&mut draw);
        
    }
    pub fn run(&self, setup:&dyn Fn(), on_tick:&dyn Fn(), on_draw:&dyn Fn(&mut RaylibDrawHandle)){
        let (mut handle, mut thread) = raylib::prelude::RaylibBuilder::default().size(1024, 1024).title("raytoast").vsync().build();
        setup();
        while !handle.window_should_close(){
            self.run_tick(handle.get_frame_time(), on_tick);
            self.run_render(&mut handle, &mut thread, on_draw);
        }
    }
}

#[allow(unused)]
static RT:Runtime = Runtime::new();
pub fn run(setup:&dyn Fn(), on_tick:&dyn Fn(), on_draw:&dyn Fn(&mut RaylibDrawHandle)){
    RT.run(setup, on_tick, on_draw);
}
pub fn get_entity<'a>(id:&'a u32)->Option<EntityRef<'a>>{
    if let Ok(ents) = RT.entities.read(){
        if let Ok(r1) = ents[*id as usize].read().as_ref(){
            if let Some(r2) = r1.as_ref(){
                let hack = r2.as_ref() as *const dyn Entity;
                return Some(EntityRef{values:Box::new((RT.entities.read().expect("LMAO"), unsafe {
                    hack.as_ref().expect("msg")
                }))})
            }
        }
    }
    None
}

pub fn destroy_entity(id:u32){}