pub mod c;
pub mod math;
pub use c::*;
#[allow(unused)]
#[repr(C)]
#[derive(Clone, Copy)]
struct TestEntity {
    pub base_class: CEntity,
}
impl Entity for TestEntity {
    fn on_render(&self) {
        draw_text(
            "lol",
            500,
            500,
            48,
            Color {
                r: 255,
                g: 255,
                b: 255,
                a: 255,
            },
        );
    }
}
setup_vtable!(
    TestEntity,
    TEST_ENTITY_VTABLE,
    no_op_drop,
    test_entity_setup
);
impl TestEntity {
    pub fn new() -> Self {
        let base_class = CEntity {
            vtable: &TEST_ENTITY_VTABLE as *const EntityVTable,
            self_id: 0,
        };
        return Self { base_class };
    }
}

#[allow(unused)]
extern "C" {
    static entity_default_vtable: EntityVTable;
}
fn test_event(_ent: &mut TestEntity) {
    println!("hello world!");
}
#[no_mangle]
pub extern "C" fn create_test_rust_entity() -> *const CEntity {
    let ent = TestEntity::new();
    let out = create_entity(ent);
    if let Some(id) = out {
        call_event(id, Box::new(&test_event));
        let t = get_entity(&id);
        if let Some(out) = t {
            return out as *const CEntity;
        } else {
            assert!(false);
            return std::ptr::null();
        }
    }
    assert!(false);
    return std::ptr::null();
}
#[no_mangle]
pub extern "C" fn hello_from_rust(f: i32) -> *const u8 {
    for _ in 0..f {
        println!("hello from rust");
    }
    return "hello toast\0".as_ptr();
}
