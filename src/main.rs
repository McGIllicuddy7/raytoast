use raylib::{color, prelude::RaylibDraw};
use runtime::ecs::Entity;

pub mod runtime;
mod utils;
struct Ent{

}
impl Entity for Ent{
    fn on_tick(&mut self, _delta_time:f32, _id:u32) {
        println!("enting");
    }
    fn on_render(&self, handle:&mut raylib::prelude::RaylibDrawHandle) {
        handle.draw_text("hey toast i love you", 400, 500, 24, color::Color::PINK);
    }
}
fn setup(){
    let ent = Box::new(Ent{});
    let _ = runtime::create_entity(ent).expect("msg");
}
fn main(){
    runtime::run(&setup, &||{}, &|_|{});
}