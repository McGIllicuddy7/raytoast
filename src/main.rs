use raylib::{color::{self, Color}, math::{Vector3, Vector4}, prelude::RaylibDraw};
use runtime::{default_components::{MeshComp, TransformComp}, ecs::Entity, get_base_shader, run};

pub mod runtime;
mod utils;
struct Ent{
    id:u32
}
impl Entity for Ent{
    fn on_init(&mut self, id:u32) {
        println!("initialized\n");
        self.id = id;
        let trans = raylib::prelude::Transform{ translation:Vector3::new(0.0, 0.0, 0.0,), 
            rotation:Vector4::new(0.0, 0.0, 0.0, 1.0),
            scale:Vector3::new(1.0, 1.0, 1.0,),

        };
        let trans_comp = TransformComp{
            location:trans, relative_location:trans, parent_id:None,
        };
        runtime::set_transform_comp(id,trans_comp);
        let mesh = MeshComp{mesh: runtime::get_cube_mesh(), mat:get_base_shader()};
        runtime::set_mesh_comp(id,mesh );
    }
    fn on_render(&self, handle:&mut raylib::prelude::RaylibDrawHandle) {
        handle.draw_text("howdy nerds",440, 500, 24, Color::PINK);
    }
    fn on_tick(&mut self, _delta_time:f32) {
    
    }
    fn get_id(&self)->Option<u32> {
        Some(self.id)
    }
}
fn setup(){
    let ent = Box::new(Ent{id:0});
    let _ = runtime::create_entity(ent).expect("msg");

}
fn main(){
    runtime::run(&setup, &||{}, &|_|{});
}