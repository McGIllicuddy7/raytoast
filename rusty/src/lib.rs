mod c;


pub use c::*;
#[allow(unused)]
#[repr(C)]
#[derive(Clone, Copy)]
struct TestEntity{
    pub base_class:CEntity, 
}
impl Entity for TestEntity{

}



static TEST_ENTITY_VTABLE:EntityVTable = EntityVTable{on_tick:TestEntity::on_tick as OnTick,on_render:TestEntity::on_render as OnRender, on_setup:TestEntity::on_setup as OnSetup, destructor:no_op_drop as Destructor};
#[allow(unused)]
extern {
    static entity_default_vtable:EntityVTable;
}
#[no_mangle]
pub extern "C" fn create_test_rust_entity()->*const CEntity{
    unsafe{
        let ent = c_funcs::malloc(size_of::<TestEntity>())as *mut  CEntity;
        let ents = ent.as_mut().unwrap();
        
        ents.vtable = &TEST_ENTITY_VTABLE as *const EntityVTable;
        return ent;
    }
}
#[no_mangle]
pub extern "C" fn hello_from_rust(f:i32)->*const u8{
    for _ in 0..f{
        println!("hello from rust");
    }
    return "hello toast\0".as_ptr()
}
