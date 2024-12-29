use std::ffi::c_void;

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
struct EntityVTable {
    on_tick:*const fn(*const c_void,f32), 
    on_render:*const fn(*const c_void), 
    on_setup:*const fn(*const c_void, u32),
    destructor:*const fn(*const c_void),
}
unsafe impl Send for EntityVTable{}
unsafe impl Sync for EntityVTable{}
type OnTick = *const fn(*const c_void,f32);
type OnRender = *const fn(*const c_void);
type OnSetup = *const fn(*const c_void, u32);
type Destructor = *const fn (*const c_void);
#[derive(Copy, Clone)]
#[repr(C)]
struct Entity{
    pub vtable:*const EntityVTable, 
    pub self_id:u32,
} 
extern "C"{
    pub fn malloc(_:usize)->*const c_void;
}
struct TestEntity{
    pub base_class:Entity, 
}
impl TestEntity{
    pub fn on_tick(&mut self, dt:f32){
        println!("hello from rust");
    }
    pub fn on_render(&self){

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
extern fn create_test_rust_entity()->*const Entity{
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