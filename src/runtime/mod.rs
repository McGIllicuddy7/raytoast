use std::sync::{Mutex, RwLock, RwLockReadGuard, RwLockWriteGuard};

use default_components::{MeshComp, PhysicsComp, TransformComp};
use ecs::{Entity, EntityRef};
use raylib::{
    color, models::Mesh, prelude::{RaylibDraw, RaylibDrawHandle}, shaders::Shader, texture::Texture2D, RaylibHandle, RaylibThread
};

use crate::utils::{Resource, ThreadLock};

pub mod default_components;
pub mod ecs;
pub struct GraphicsResources {
    pub shaders: Resource<Shader>,
    pub textures:Resource<Texture2D>,
    pub meshes: Resource<Mesh>,
}
impl Default for GraphicsResources {
    fn default() -> Self {
        Self::new()
    }
}

impl GraphicsResources {
    pub const fn new() -> Self {
        Self {
            shaders: Resource::new(),
            textures: Resource::new(),
            meshes: Resource::new(),
        }
    }
    pub fn create_shader(&mut self, shader: Shader) -> Option<u32> {
        self.shaders.create_new(shader).map(|i| i as u32)
    }
    pub fn delete_shader(&mut self, id: u32) -> bool {
        self.shaders.destroy(id as usize)
    }
    pub fn create_texture(&mut self, i:Texture2D) -> Option<u32> {
        self.textures.create_new(i).map(|i| i as u32)
    }
    pub fn delete_material(&mut self, id: u32) -> bool {
        self.textures.destroy(id as usize) 
    }
    pub fn create_mesh(&mut self, i: Mesh) -> Option<u32> {
        self.meshes.create_new(i).map(|i| i as u32)
    }
    pub fn delete_mesh(&mut self, id: u32) -> bool {
        self.meshes.destroy(id as usize)
    }

    pub fn get_shader(&self, id:usize)->Option<&Shader>{
        self.shaders.get(id)
    }
    pub fn get_mesh(&self, id:usize)->Option<&Mesh>{
        self.meshes.get(id)
    }
    pub fn get_material(&self, id:usize)->Option<&Texture2D>{
        self.textures.get(id) 
    }
}
pub struct Runtime {
    pub entities: RwLock<Vec<RwLock<Option<Box<dyn Entity + Send + Sync>>>>>,
    pub failed_to_create: Mutex<bool>,
    pub destroy_queue: Mutex<Vec<u32>>,
    pub transform_comps: RwLock<Resource<TransformComp>>,
    pub physics_comps: RwLock<Resource<PhysicsComp>>,
    pub mesh_comps: RwLock<Resource<MeshComp>>,
    pub graphics_resources: ThreadLock<GraphicsResources>,
}
impl Default for Runtime {
    fn default() -> Self {
        Self::new()
    }
}

impl Runtime {
    pub const fn new() -> Self {
        Self {
            entities: RwLock::new(Vec::new()),
            failed_to_create: Mutex::new(true),
            destroy_queue: Mutex::new(Vec::new()),
            physics_comps: RwLock::new(Resource::new()),
            mesh_comps: RwLock::new(Resource::new()),
            transform_comps: RwLock::new(Resource::new()),
            graphics_resources: ThreadLock::new(GraphicsResources::new()),
        }
    }
    fn reserve_slots(&self) {
        let mut ents = self.entities.write().expect("msg");
        let reserve_count = 1000;
        for _ in 0..reserve_count {
            ents.push(RwLock::new(None));
        }
        let mut tras = self.transform_comps.write().expect("msg");
        tras.reserve(reserve_count);
        let mut phys = self.physics_comps.write().expect("msg");
        phys.reserve(reserve_count);
        let mut meshs = self.mesh_comps.write().expect("msg");
        meshs.reserve(reserve_count);
    }
    pub fn read_entities(
        &self,
    ) -> RwLockReadGuard<'_, Vec<RwLock<Option<Box<dyn Entity + Send + Sync>>>>> {
        self.entities.read().expect("works")
    }
    pub fn write_entities(
        &self,
    ) -> RwLockWriteGuard<'_, Vec<RwLock<Option<Box<dyn Entity + Send + Sync>>>>> {
        self.entities.write().expect("works")
    }
    pub fn run_tick(&self, delta_time: f32, on_tick: &dyn Fn()) {
        {
            let mut lck = self.failed_to_create.lock().expect("msg");
            if *lck {
                self.reserve_slots();
                *lck = false;
            }
        }
        let ents = self.read_entities();
        let mut id = 0;
        on_tick();
        for i in ents.iter() {
            let mut ent_opt = i.write().expect("works");
            if let Some(ent) = ent_opt.as_mut() {
                ent.on_tick(delta_time);
            }
            id += 1;
        }
    }
    pub fn run_render(
        &self,
        handle: &mut RaylibHandle,
        thread: &mut RaylibThread,
        on_draw: &dyn Fn(&mut RaylibDrawHandle),
    ) {
        let mut draw = handle.begin_drawing(thread);
        draw.clear_background(color::Color::BLACK);
        let rd = self.entities.read().expect("msg");
        for i in rd.iter() {
            if let Some(j) = i.read().expect("msg").as_ref() {
                j.on_render(&mut draw);
            }
        }
        on_draw(&mut draw);
    }
    pub fn run(
        &self,
        setup: &dyn Fn(),
        on_tick: &dyn Fn(),
        on_draw: &dyn Fn(&mut RaylibDrawHandle),
    ) {
        let (mut handle, mut thread) = raylib::prelude::RaylibBuilder::default()
            .size(1024, 1024)
            .title("raytoast")
            .vsync()
            .build();
        self.reserve_slots();
        setup();
        while !handle.window_should_close() {
            self.run_tick(handle.get_frame_time(), on_tick);
            self.run_render(&mut handle, &mut thread, on_draw);
        }
    }
}

#[allow(unused)]
static RT: Runtime = Runtime::new();
pub fn run(setup: &dyn Fn(), on_tick: &dyn Fn(), on_draw: &dyn Fn(&mut RaylibDrawHandle)) {
    RT.run(setup, on_tick, on_draw);
}
pub fn get_entity(id: &u32) -> Option<EntityRef<'_>> {
    if let Ok(ents) = RT.entities.read() {
        if *id as usize >= ents.len() {
            return None;
        }
        if let Ok(r1) = ents[*id as usize].read().as_ref() {
            if let Some(r2) = r1.as_ref() {
                let hack = r2.as_ref() as *const dyn Entity;
                return Some(EntityRef {
                    values: Box::new((RT.entities.read().expect("LMAO"), unsafe {
                        hack.as_ref().expect("msg")
                    })),
                });
            }
        }
    }
    None
}

pub fn destroy_entity(id: u32) {
    let mut m = RT.destroy_queue.lock().expect("msg");
    m.push(id);
}

pub fn create_entity(mut entity: Box<dyn Entity + Send + Sync>) -> Option<u32> {
    if let Ok(ents) = RT.entities.read() {
        for i in 0..ents.len() {
            if let Ok(mut ent) = ents[i].write() {
                if ent.is_none() {
                    entity.as_mut().on_init(i as u32);
                    *ent = Some(entity);
                    return Some(i as u32);
                }
            }
        }
    }
    let mut m = RT.failed_to_create.lock().expect("not poisoned");
    *m = true;
    None
}

pub fn get_physics_comp(id:u32)->Option<PhysicsComp>{
    if let Ok(m) = RT.physics_comps.read(){
        m.get(id as usize).map(|i| *i)
    } else{
        None
    }
}

pub fn get_mesh_comp(id:u32)->Option<MeshComp>{
    if let Ok(m) = RT.mesh_comps.read(){
        m.get(id as usize).map(|i| *i)
    } else{
        None
    } 
}

pub fn get_transform_comp(id:u32)->Option<TransformComp>{
    if let Ok(m) = RT.transform_comps.read(){
        m.get(id as usize).map(|i| *i)
    } else{
        None
    }
}

