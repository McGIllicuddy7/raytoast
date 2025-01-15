use crate::c_funcs;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct CVec<T> {
    pub items: *mut T,
    pub length: usize,
    pub capacity: usize,
    pub arena: *mut c_funcs::Arena,
}
unsafe impl<T> Send for CVec<T> {}
unsafe impl<T> Sync for CVec<T> {}
impl <T:Clone>Into<Vec<T>> for CVec<T>{
    fn into(self) -> Vec<T> {
        unsafe {std::ptr::slice_from_raw_parts(self.items, self.length).as_ref().map_or(Vec::new(), |f| {f.to_vec()})}
    }
}
