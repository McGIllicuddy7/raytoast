use raylib::{color, prelude::RaylibDraw};

pub mod runtime;
mod utils;
fn main(){
    runtime::run(&||{println!("did a thing")}, &||{println!("f")}, &|handle|{handle.draw_text("hey toast i love you", 400, 500, 24, color::Color::PINK);});
}