use raylib::{color, prelude::RaylibDraw};
use runtime::ecs::Entity;

pub mod runtime;
mod utils;
struct Ent{
    id:u32
}
impl Entity for Ent{
    fn on_init(&mut self, id:u32) {
        println!("initialized\n");
        self.id = id;
    }
    fn on_tick(&mut self, _delta_time:f32) {
    
    }
    fn on_render(&self, handle:&mut raylib::prelude::RaylibDrawHandle) {
        handle.draw_text("hey toast i love you", 400, 500, 24, color::Color::PINK);
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