use std::{ffi::c_void, mem::ManuallyDrop};

use libc::memcpy;
#[allow(unused)]
/* 
struct EntityVTable{
    void (*on_tick)(void * self, f32 delta_time),
    void (*on_render)(void * self),
    void (*on_setup)(void* self,u32 self_id),
    void (*destructor)(void *self),
}

#[repr C]
struct Entity{
    EntityVTable * vtable;
    u32 self_id;
}*/
#[repr(C)]
#[derive(Copy, Clone)]
pub struct EntityVTable {
    pub on_tick:*const extern fn(*const c_void,f32), 
    pub on_render:*const extern fn(*const c_void), 
    pub on_setup:*const extern fn(*const c_void, u32),
    pub destructor:*const extern fn(*const c_void),
}
#[allow(unused)]
macro_rules! setup_vtable {
    ($T:ident, $name:ident, $destructor:ident, $setup_fn_name:ident) => {
        static $name:EntityVTable = EntityVTable{on_tick:$T::on_tick as OnTick,on_render:$T::on_render as OnRender, on_setup:$T::on_setup as OnSetup, destructor:$destructor as Destructor};
        pub fn $setup_fn_name()->CEntity{
            CEntity{vtable:&$name  as *const EntityVTable, self_id:0}
        }
    };
}
pub(crate) use setup_vtable;
unsafe impl Send for EntityVTable{}
unsafe impl Sync for EntityVTable{}
pub type OnTick = *const extern fn(*const c_void,f32);
pub type OnRender = *const extern fn(*const c_void);
pub type OnSetup = *const extern fn(*const c_void, u32);
pub type Destructor = *const extern fn (*const c_void);
#[allow(unused)]
pub trait Entity: {
    fn on_tick(&mut self, dt:f32){

    }
    fn on_render(&self){

    }
    fn on_setup(&mut self, id:u32){

    }
    fn destructor(&mut self){

    }
} 
#[repr(C)]
#[derive(Clone, Copy)]
pub struct CEntity{
    pub vtable:*const EntityVTable, 
    pub self_id:u32,
}
impl Entity for CEntity{
    fn on_tick(&mut self, dt:f32){
        unsafe{
            (*((*self.vtable).on_tick))(self as *mut Self as *mut c_void, dt)
        }
    }
    fn on_render(&self){
        unsafe{
            (*((*self.vtable).on_render))(self as *const Self as *mut c_void)
        }
    }
    fn on_setup(&mut self, id:u32){
        unsafe{
            (*((*self.vtable).on_setup))(self as *const Self as *mut c_void,id)
        }
    }
    fn destructor(&mut self){
        unsafe{
            (*((*self.vtable).destructor))(self as *const Self as *mut c_void)
        }
    } 
}
#[repr(C)]
#[derive(Clone, Copy)]
pub struct Color{
    pub r:u8, 
    pub g:u8, 
    pub b:u8,
    pub a:u8
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct Shader{
    pub id:u32, 
    pub locs:*mut i32,
}
unsafe impl Send for Shader{}
unsafe impl Sync for Shader{} 

#[repr(C)]
#[derive(Clone, Copy)]
pub struct Mesh{
    pub vertex_count:i32, 
    pub triangle_count:i32, 
    pub vertices:*mut f32, 
    pub texcoords:*mut f32,
    pub texcoords2:*mut f32, 
    pub normals:*mut f32, 
    pub tangents:*mut f32,
    pub colors:*mut u8, 
    pub indices:*mut u16, 
    pub anim_vertices:*mut f32, 
    pub anim_normals:*mut f32, 
    pub bone_ids:*mut u8, 
    pub bone_weights:*mut f32, 
    pub vao_id:u32, 
    pub vbo_id:*mut u32, 
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct Vector3{
    pub x:f32, 
    pub y:f32, 
    pub z:f32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct Vector4{
    pub x:f32, 
    pub y:f32, 
    pub z:f32,
    pub w:f32,
}
#[repr(C)]
#[derive(Clone, Copy)]
pub struct Optu32{
    pub valid:bool, 
    pub value:u32,
}
#[repr(C)]
#[derive(Clone, Copy)]
pub struct CVec<T>{
    pub items :*mut T, 
    pub length:usize, 
    pub capacity:usize, 
    pub arena:*mut c_funcs::Arena, 
}
unsafe impl <T> Send for CVec<T>{}
unsafe impl <T> Sync for CVec<T>{}
#[repr(C)]
#[derive(Clone, Copy)]
pub struct Transform{
    pub translation:Vector3, 
    pub rotation:Vector4, 
    pub scale:Vector4,
}
#[repr(C)]
#[derive(Clone, Copy)]
pub struct TransformComp{
    pub transform:Transform, 
    pub parent:Optu32, 
    pub children:CVec<u32>,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct MeshComp{
    pub shader_id:u32,
    pub mesh_id:u32,
}
#[repr(C)]
#[derive(Clone, Copy)]
pub struct BoundingBox{
    pub min:Vector3, 
    pub max:Vector3, 
}
#[repr(C)]
#[derive(Clone, Copy)]
pub struct PhysicsComp{
    pub bx:BoundingBox,
    pub velocity:Vector3,
    pub mass:f32,
    pub collided_this_frame:bool,
    pub movable:bool, 
    pub can_bounce:bool
}
pub mod c_funcs{        
    use super::*;
    #[repr(C)]
    #[derive(Clone, Copy)]
    pub struct Arena{
        pub lock:libc::pthread_mutex_t, 
        pub buffer:*const i8, 
        pub next_ptr:*const i8, 
        pub end:*const i8, 
        pub previous_allocation: *const i8, 
        pub next:*mut Arena, 
        pub defer_que:*mut c_void,
    }
    extern "C"{
        static temporary_allocator:Arena;
    }
    #[allow(unused)]
    extern "C"{
        pub fn malloc(_:usize)->*const c_void;
        pub fn arena_alloc(arena:*mut Arena, size:usize)->*mut c_void;
        pub fn DrawText(text: *const i8, posx:i32, posy:i32, font_size:i32,color:Color);
        pub fn create_entity(ent:*mut CEntity)->Optu32;
        pub fn destroy_entity(id:u32);
        pub fn get_entity(id:u32)->*mut CEntity;
        pub fn add_force(id:u32, force:Vector3);
        
        pub fn set_transform_comp(id:u32,trans:TransformComp)->bool;
        pub  fn get_transform_comp(id:u32)->*mut TransformComp;
        pub fn remove_transform_comp(id:u32)->bool;
        
        pub fn set_physics_comp(id:u32, phys:PhysicsComp)->bool;
        pub fn get_physics_comp(id:u32)->*mut PhysicsComp;
        pub fn remove_physics_comp(id:u32)->bool;
        
        pub fn set_mesh_comp(id:u32,mesh:MeshComp)->bool;
        pub fn get_mesh_comp(id:u32)->*mut MeshComp;
        pub fn remove_mesh_comp(id:u32)->bool;
        pub fn call_event(id:u32, event:*const extern fn (this:*mut CEntity, args:*const c_void),args:*const c_void);
    }
    pub unsafe fn tmp_alloc(size:usize)->*mut c_void{
        arena_alloc(&temporary_allocator as*const Arena as *mut Arena, size)
    }
}

pub fn no_op_drop(_:*const c_void){

}
pub fn create_entity<T:Entity>(entity:T)->Option<u32>{
    let v= unsafe{
        let ent = c_funcs::malloc(size_of::<T>())as *mut T;

        let ents = ent.as_mut().unwrap();
        *ents = entity;
        c_funcs::create_entity(ent as *mut CEntity)
    };
    if v.valid{
        Some(v.value)
    } else{
        None
    }
}
#[allow(unused)]
pub fn draw_text(text:&str, posx:i32, posy:i32, font_size:i32,color:Color){
    let txt = text.to_owned() + "\0";
    unsafe {
        c_funcs::DrawText(txt.as_ptr() as *const i8, posx, posy, font_size, color);
    }
}
pub fn destroy_entity(id:u32){
    unsafe{
        c_funcs::destroy_entity(id);
    }
}
pub fn get_entity<'a>(id:&'a u32)->Option<&'a mut CEntity>{
    unsafe{
        let out= c_funcs::get_entity(*id);
        if out.is_null(){
            None
        } else{
            out.as_mut() 
        }
    }
}
pub fn add_force(id:u32, force:Vector3){
    unsafe {
        c_funcs::add_force(id, force);
    }
}

pub fn set_transform_comp(id:u32,trans:TransformComp)->bool{
    unsafe{c_funcs::set_transform_comp(id, trans)}
}
pub  fn get_transform_comp<'a>(id:&'a u32)->Option<&'a mut TransformComp>{
    unsafe{
        let out= c_funcs::get_transform_comp(*id);
        if out.is_null(){
            None
        } else{
            out.as_mut() 
        }
    }
}
pub fn remove_transform_comp(id:u32)->bool{
    unsafe{
        c_funcs::remove_mesh_comp(id)
    }
}

pub fn set_physics_comp(id:u32, phys:PhysicsComp)->bool{
    unsafe {
        c_funcs::set_physics_comp(id, phys)
    }
}
pub fn get_physics_comp<'a>(id:&'a u32)->Option<&'a mut PhysicsComp>{
    unsafe{
        let out= c_funcs::get_physics_comp(*id);
        if out.is_null(){
            None
        } else{
            out.as_mut()
        }
    } 
}
pub fn remove_physics_comp(id:u32)->bool{
    unsafe {
        c_funcs::remove_physics_comp(id)
    }
}

pub fn set_mesh_comp(id:u32,mesh:MeshComp)->bool{
    unsafe {
        c_funcs::set_mesh_comp(id, mesh)
    }
}
pub fn get_mesh_comp<'a>(id:&'a u32)->Option<&'a mut MeshComp>{
    unsafe{
        let out= c_funcs::get_mesh_comp(*id);
        if out.is_null(){
            None
        } else{
            out.as_mut()
        }
    } 
}
pub fn remove_mesh_comp(id:u32)->bool{
    unsafe {
        c_funcs::remove_mesh_comp(id)
    }
}


pub fn call_event<T:Entity>(id:u32, func:Box<dyn Fn(&mut T)>){
    extern "C" fn thunk<T:Entity>(ent:*mut c_void,args:*const c_void){
        assert!(!ent.is_null());
        unsafe{
            let entity = (ent as *mut T).as_mut().unwrap();
            let func = std::ptr::read(args as *const Box<dyn Fn(&mut dyn Entity)>);
            func(entity);
        }  

    }
    unsafe{
        func((get_entity(&id).expect("msg") as *mut CEntity as * mut T).as_mut().expect("msg"));
        let arrrrg = c_funcs::tmp_alloc(size_of::<Box<dyn Fn(&mut dyn Entity)>>()) as *mut Box<dyn Fn(&mut T)>;
        memcpy(arrrrg as *mut c_void, func.as_ref() as *const dyn Fn(&mut T) as *mut c_void, 16);
        let _ =  ManuallyDrop::new(func);
        c_funcs::call_event(id, thunk::<T> as *const extern "C" fn (*mut CEntity, *const c_void), arrrrg as *const c_void);
    }
}