use std::{cell::RefCell, sync::Mutex};

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
    pub fn get<'a>(&'a self)->Option<std::cell::Ref<'a, T>>{
        let mut t = self.thread_id.lock().expect("msg");
        match  t.as_ref() {
            None=>{      *t = Some(std::thread::Thread::id(&std::thread::current()));       if let Ok(out) = self.cell.try_borrow(){
                return Some(out);
            } else{
                return None;
            }}
            Some(id)=>{
                if *id != std::thread::Thread::id(&std::thread::current()){
                    return None;
                } else{
                    if let Ok(out) = self.cell.try_borrow(){
                        return Some(out);
                    } else{
                        return None;
                    }
                }
            }
        }
    }

    #[allow(unused)]
    pub fn get_mut<'a>(&'a self)->Option<std::cell::RefMut<'a, T>>{
        let mut t = self.thread_id.lock().expect("msg");
        match  t.as_ref() {
            None=>{      *t = Some(std::thread::Thread::id(&std::thread::current())); return None;}
            Some(id)=>{
                if *id != std::thread::Thread::id(&std::thread::current()){
                    return None;
                } else{
                    if let Ok(out) = self.cell.try_borrow_mut(){
                        return Some(out);
                    } else{
                        return None;
                    }
                }
            }
        }
    }
}

#[allow(unused)]
pub struct Resource<T>{
    pub values:Vec<Option<T>>,
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
        return true;
    }
    
    #[allow(unused)]
    pub fn get<'a>(&'a self, id:usize)->Option<&'a T>{
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