use std::ffi::{c_void, CStr, CString};
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
    on_tick:*const extern fn(*const c_void,f32), 
    on_render:*const extern fn(*const c_void), 
    on_setup:*const extern fn(*const c_void, u32),
    destructor:*const extern fn(*const c_void),
}
unsafe impl Send for EntityVTable{}
unsafe impl Sync for EntityVTable{}
type OnTick = *const extern fn(*const c_void,f32);
type OnRender = *const extern fn(*const c_void);
type OnSetup = *const extern fn(*const c_void, u32);
type Destructor = *const extern fn (*const c_void);
#[derive(Copy, Clone)]
#[repr(C)]
pub struct Entity{
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
extern "C"{
    pub fn malloc(_:usize)->*const c_void;
    pub fn DrawText(text: *const i8, posx:i32, posy:i32, font_size:i32,color:Color);
}
struct TestEntity{
    pub base_class:Entity, 
}
impl TestEntity{
    pub fn on_tick(&mut self, dt:f32){
        println!("hello from rust");
    }
    pub fn on_render(&self){
        unsafe{
            DrawText(CString::new("hello from rust").expect("msg").as_ptr() , 500, 500, 12,Color { r: 255, g: 255, b: 255, a: 255 });
        }
    }
    pub fn on_setup(&mut self,id:u32){
        self.base_class.self_id = id;
    }

}
pub fn no_op_drop(_:*const c_void){

}
static TestEntityVtable:EntityVTable = EntityVTable{on_tick:TestEntity::on_tick as OnTick,on_render:TestEntity::on_render as OnRender, on_setup:TestEntity::on_setup as OnSetup, destructor:no_op_drop as Destructor};
extern {
    static entity_default_vtable:EntityVTable;
}
#[no_mangle]
pub extern "C" fn create_test_rust_entity()->*const Entity{
    unsafe{
        let ent = malloc(size_of::<TestEntity>())as *mut  Entity;
        let ents = ent.as_mut().unwrap();
        ents.vtable = &TestEntityVtable as *const EntityVTable;
        return ent;
    }
}
#[no_mangle]
pub extern "C" fn hello_from_rust(f:i32)->*const u8{
    for i in 0..f{
        println!("hello from rust");
    }
    return "hello toast\0".as_ptr()
}
