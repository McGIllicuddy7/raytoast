
use std::{fmt::Debug, sync::Mutex};
use reflection::{Reflect, StaticReflect};
use utils::{register_types, Context, ContextRef, Reflect, StaticReflect};
pub mod utils;
static CONT: Context<[Mutex<usize>; 10]> = Context::new();
pub fn fill() -> ContextRef<'static, [Mutex<usize>; 10]> {
    CONT.store([const { Mutex::new(0) }; 10]);
    let t = CONT.get_ref();
    for i in 0..10 {
        let mut value = t[i].lock().unwrap();
        *value = i;
    }
    t
}
#[derive(StaticReflect,Reflect,Debug)]
pub struct Toast{
    pub name:String,
    pub a:usize
}


fn main(){
    register_types(&[as_registered!(Toast)]);
    let toast = Toast{name:"lol".to_string(), a:32};
    for f in toast.fields(){
        let a =  f.reflect;
        println!("{}:{:#?}",f.name,a);
    }
}
