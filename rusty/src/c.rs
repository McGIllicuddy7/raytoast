use std::ffi::c_void;
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
unsafe impl Send for EntityVTable{}
unsafe impl Sync for EntityVTable{}
pub type OnTick = *const extern fn(*const c_void,f32);
pub type OnRender = *const extern fn(*const c_void);
pub type OnSetup = *const extern fn(*const c_void, u32);
pub type Destructor = *const extern fn (*const c_void);
pub trait Entity{
    fn on_tick(&self, _dt:f32){

    }
    fn on_render(&self){

    }
    fn on_setup(&self, _id:u32){

    }
    fn destructor(&self){

    }
} 
#[repr(C)]
#[derive(Clone, Copy)]
pub struct CEntity{
    pub vtable:*const EntityVTable, 
    pub self_id:u32,
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
pub struct Transform{
    pub translation:Vector3, 
    pub rotation:Vector4, 
    pub scale:Vector4,
}
#[repr(C)]
#[derive(Clone, Copy)]
pub struct TransformComp{
    pub transform:Transform
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
    #[allow(unused)]
    extern "C"{
        pub fn malloc(_:usize)->*const c_void;
        pub fn tmp_alloc(_:usize)->*const c_void;
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
        fn call_event(id:u32, event:*const extern fn (this:*const c_void, args:*const c_void),args:*const c_void);
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

pub fn call_event<T:Entity, U:Fn(&mut T)+Clone>(id:u32, event:&U){
    let bx = Box::new(event.clone());
    let event =bx.as_ref() as *const dyn Fn(&mut T);
    let func = (&event) as *const *const dyn Fn(&mut T); 
    let env = unsafe{func.offset(8)};
    todo!()
}