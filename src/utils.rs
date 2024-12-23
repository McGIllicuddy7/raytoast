use std::{cell::RefCell, sync::Mutex};

use raylib::math::{Matrix, Quaternion, Transform};

#[allow(unused)]
pub struct ThreadLock<T>{
    cell:RefCell<T>,
    thread_id:Mutex<Option<std::thread::ThreadId>>,
}
unsafe impl <T> Send for ThreadLock<T>{}
unsafe impl <T> Sync for ThreadLock<T>{}
impl <T> ThreadLock<T>{
    #[allow(unused)]
    pub const fn new(value:T)->Self{
        Self { cell: RefCell::new(value), thread_id: Mutex::new(None) }
    }

    #[allow(unused)]
    pub fn get(&self)->Option<std::cell::Ref<'_, T>>{
        let mut t = self.thread_id.lock().expect("msg");
        match  t.as_ref() {
            None=>{      *t = Some(std::thread::Thread::id(&std::thread::current()));       if let Ok(out) = self.cell.try_borrow(){
                Some(out)
            } else{
                None
            }}
            Some(id)=>{
                if *id != std::thread::Thread::id(&std::thread::current()){
                    None
                } else if let Ok(out) = self.cell.try_borrow(){
                    Some(out)
                } else{
                    None
                }
            }
        }
    }

    #[allow(unused)]
    pub fn get_mut(&self)->Option<std::cell::RefMut<'_, T>>{
        let mut t = self.thread_id.lock().expect("msg");
        match  t.as_ref() {
            None=>{      *t = Some(std::thread::Thread::id(&std::thread::current()));       if let Ok(out) = self.cell.try_borrow_mut(){
                Some(out)
            } else{
                None
            }}
            Some(id)=>{
                if *id != std::thread::Thread::id(&std::thread::current()){
                    None
                } else if let Ok(out) = self.cell.try_borrow_mut(){
                    Some(out)
                } else{
                    None
                }
            }
        }
    }
}

#[allow(unused)]
pub struct Resource<T>{
    pub values:Vec<Option<T>>,
}
impl<T> Default for Resource<T> {
    fn default() -> Self {
        Self::new()
    }
}

impl <T>Resource<T>{
    #[allow(unused)]
    pub const fn new()->Self{
        Self { values: Vec::new() }
    }

    #[allow(unused)]
    pub fn create_new(&mut self,value:T)->Option<usize>{
        for i in 0..self.values.len(){
            if self.values[i].is_none(){
                (self.values)[i] = Some(value);
                return Some(i);
            }
        }
        self.values.push(Some(value));
        Some(self.values.len()-1)
    }

    #[allow(unused)]
    pub fn destroy(&mut self, id:usize)->bool{
        if id>=self.values.len(){
            return false;
        }
        self.values[id] = None;
        true
    }
    
    #[allow(unused)]
    pub fn get(&self, id:usize)->Option<&T>{
        if id>= self.values.len(){
            return None;
        }
        return self.values[id].as_ref();
    }

    #[allow(unused)]
    pub fn reserve(&mut self, count:usize){
        for _ in 0..count{
            self.values.push(None);
        }
    }
}

pub fn quat_to_mat(q:Quaternion)->Matrix
{
    let mut result = raylib::math::Matrix::identity();

    let a2 = 2.0*(q.x*q.x);
    let b2=2.0*(q.y*q.y);
    let c2=2.0*(q.z*q.z); //, d2=2*(q.w*q.w);

    let  ab = 2.0*(q.x*q.y);
    let ac=2.0*(q.x*q.z);
    let bc=2.0*(q.y*q.z);
    let ad = 2.0*(q.x*q.w);
    let  bd=2.0*(q.y*q.w);
    let cd=2.0*(q.z*q.w);

    result.m0 = 1.0 - b2 - c2;
    result.m1 = ab + cd;
    result.m2 = ac - bd;

    result.m4 = ab - cd;
    result.m5 = 1.0 - a2 - c2;
    result.m6 = bc + ad;

    result.m8 = ac + bd;
    result.m9 = bc - ad;
    result.m10 = 1.0 - a2 - b2;

    result
}

pub fn transform_to_matrix(trans:Transform)->Matrix{
    let loc = trans.translation;
    let rot = trans.rotation;
    let scale  = trans.scale;
    let locmat = raylib::math::Matrix::translate(loc.x, loc.y, loc.z);
    let rotmat = quat_to_mat(rot);
    let scalemat = raylib::math::Matrix::scale(scale.x, scale.y, scale.z);
    locmat*rotmat*scalemat
}

pub fn transform_to_ffimatrix(trans:Transform)->raylib::ffi::Matrix{
    unsafe {
        std::mem::transmute(transform_to_matrix(trans))
    }
}