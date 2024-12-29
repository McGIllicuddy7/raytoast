mod c;


pub use c::*;
struct TestEntity{
    pub base_class:CEntity, 
}
impl Entity for TestEntity{

}



static TestEntityVtable:EntityVTable = EntityVTable{on_tick:TestEntity::on_tick as OnTick,on_render:TestEntity::on_render as OnRender, on_setup:TestEntity::on_setup as OnSetup, destructor:no_op_drop as Destructor};
extern {
    static entity_default_vtable:EntityVTable;
}
#[no_mangle]
pub extern "C" fn create_test_rust_entity()->*const CEntity{
    unsafe{
        let ent = c_funcs::malloc(size_of::<TestEntity>())as *mut  CEntity;
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
