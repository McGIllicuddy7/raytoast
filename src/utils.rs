use serde::{Deserialize, Serialize};
use std::any::{type_name, Any, TypeId};
use std::cell::Ref;
use std::marker::PhantomData;
use std::ops::{Deref, DerefMut};


use std::sync::{Mutex, RwLock, RwLockReadGuard, RwLockWriteGuard};
pub struct Context<T: Send + Sync + Serialize + for<'a> Deserialize<'a>> {
    value: RwLock<Option<T>>,
}
impl<T: Send + Sync + Serialize + for<'a> Deserialize<'a>> Context<T> {}
pub struct ContextRef<'a, T: Send + Sync> {
    v: RwLockReadGuard<'a, Option<T>>,
}
impl<T: Send + Sync> Deref for ContextRef<'_, T> {
    type Target = T;
    fn deref(&self) -> &Self::Target {
        self.v.as_ref().unwrap()
    }
}

pub struct ContextMut<'a, T: Send + Sync> {
    v: RwLockWriteGuard<'a, Option<T>>,
}
impl<T: Send + Sync> Deref for ContextMut<'_, T> {
    type Target = T;
    fn deref(&self) -> &Self::Target {
        self.v.as_ref().unwrap()
    }
}
impl<T: Send + Sync> DerefMut for ContextMut<'_, T> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        self.v.as_mut().unwrap()
    }
}
impl<T: Send + Sync + Serialize + for<'a> Deserialize<'a>> std::default::Default for Context<T> {
    fn default() -> Self {
        Self::new()
    }
}
impl<T: Send + Sync + Serialize + for<'a> Deserialize<'a>> Context<T> {
    pub const fn new() -> Self {
        Self {
            value: RwLock::new(None),
        }
    }
    pub const fn new_with(value: T) -> Self {
        Self {
            value: RwLock::new(Some(value)),
        }
    }

    pub fn get_ref(&self) -> ContextRef<T> {
        let t = self.value.read().unwrap();
        if t.as_ref().is_some() {
            ContextRef { v: t }
        } else {
            t.as_ref().unwrap();
            unreachable!()
        }
    }
    pub fn get_mut(&self) -> ContextMut<T> {
        let t = self.value.write().unwrap();
        if t.as_ref().is_some() {
            ContextMut { v: t }
        } else {
            t.as_ref().unwrap();
            unreachable!()
        }
    }
    pub fn get_ref_failable(&self) -> Option<ContextRef<T>> {
        let t = self.value.read().unwrap();
        if t.as_ref().is_some() {
            Some(ContextRef { v: t })
        } else {
            None
        }
    }
    pub fn get_mut_failable(&self) -> Option<ContextMut<T>> {
        let t = self.value.write().unwrap();
        if t.as_ref().is_some() {
            Some(ContextMut { v: t })
        } else {
            None
        }
    }
    pub fn store(&self, v: T) {
        let mut t = self.value.write().unwrap();
        *t = Some(v)
    }
    pub fn store_none(&self) {
        let mut t = self.value.write().unwrap();
        *t = None;
    }
    pub fn remove(&self) -> Option<T> {
        let mut t = self.value.write().unwrap();
        t.take()
    }
}

#[derive(Debug,Clone)]
pub struct Field {
    pub name: String,
    pub offset: usize,
    pub value: Type,
}
#[derive(Debug,Clone)]
pub struct EnumVariant {
    pub name: String,
    pub internal_value: usize,
    pub value: Type,
}
#[derive(Debug,Clone)]
pub enum TypeData {
    Enum { variants: Vec<EnumVariant> },
    Struct { fields: Vec<Field> },
    Pointer{internal:Box<Type>},
    Slice{internal:Box<Type>},
    Vec{internal:Box<Type>}, 
    Map{key:Box<Type>,values:Box<Type>}, 
    Set{values:Box<Type>},
    String, 
    OpaqueType,
}
#[derive(Debug,Clone)]
pub struct Type {
    pub name: String,
    pub data: TypeData,
    pub size: usize,
    pub align:usize,
}
pub trait StaticReflect:Sized+std::fmt::Debug{
    fn static_reflect() -> Type {
        Type {
            name: type_name::<Self>().to_string(),
            data: TypeData::OpaqueType,
            size: size_of::<Self>(),
            align:align_of::<Self>(),
        }
    }
    fn to_any_ptr( ptr:&u8)->(&dyn Any, &dyn Reflect)
    where Self:'static+Reflect{
        let ptr = ptr as *const u8;
        let out = ptr as *const Self as *const dyn Any;
        let out2 = ptr as *const Self as *const dyn Reflect;
        unsafe{
            (out.as_ref().unwrap(), out2.as_ref().unwrap())
        }
    }
    fn to_bytes(ptr:&u8)->Option<Box<dyn Reflect>>{
        let t = <Self>::static_reflect();
        match t.data{
            TypeData::Enum { variants } => {},
            TypeData::Struct { fields } => todo!(),
            TypeData::Pointer { internal } => todo!(),
            TypeData::Slice { internal } => todo!(),
            TypeData::Vec { internal } => todo!(),
            TypeData::Map { key, values } => todo!(),
            TypeData::Set { values } => todo!(),
            TypeData::String => todo!(),
            TypeData::OpaqueType => todo!(),
        }
        None
    }
}
impl StaticReflect for i8{}
impl StaticReflect for i16{}
impl StaticReflect for i32{}
impl StaticReflect for i64{}
impl StaticReflect for i128{}
impl StaticReflect for u8{}
impl StaticReflect for u16{}
impl StaticReflect for u32{}
impl StaticReflect for u64{}
impl StaticReflect for u128{}
impl StaticReflect for bool{}
impl StaticReflect for usize{}
impl StaticReflect for isize{}
impl StaticReflect for f32{}
impl StaticReflect for f64{}
impl <T:StaticReflect> StaticReflect for *const T{
    fn static_reflect() -> Type {
        Type { name: type_name::<Self>().to_string(), data: TypeData::Pointer { internal: Box::new(T::static_reflect())} , size: size_of::<Self>(),            align:align_of::<Self>()}
    }
}
impl <T:StaticReflect> StaticReflect for *mut T{
    fn static_reflect() -> Type {
        Type { name: type_name::<Self>().to_string(), data: TypeData::Pointer { internal: Box::new(T::static_reflect())} , size: size_of::<Self>(),            align:align_of::<Self>()}
    }
}
impl <T:StaticReflect> StaticReflect for Vec<T>{
        fn static_reflect() -> Type {
            Type { name: type_name::<Self>().to_string(), data: TypeData::Vec{ internal: Box::new(T::static_reflect())} , size: size_of::<Self>(),            align:align_of::<Self>()}
        }
}
impl <T:StaticReflect> StaticReflect for &[T]{
    fn static_reflect() -> Type {
        Type { name: type_name::<Self>().to_string(), data: TypeData::Slice{ internal: Box::new(T::static_reflect())} , size: size_of::<Self>(),            align:align_of::<Self>()}
    }
}
impl <T:StaticReflect> StaticReflect for &T{
    fn static_reflect() -> Type {
        Type { name: type_name::<Self>().to_string(), data: TypeData::Slice{ internal: Box::new(T::static_reflect())} , size: size_of::<Self>(),            align:align_of::<Self>()}
    }
}
impl <T:StaticReflect> StaticReflect for std::collections::HashSet<T>{
    fn static_reflect() -> Type {
        Type { name: type_name::<Self>().to_string(), data: TypeData::Set{ values: Box::new(T::static_reflect())} , size: size_of::<Self>(),            align:align_of::<Self>()}
    }
}

impl <T:StaticReflect,U:StaticReflect> StaticReflect for std::collections::HashMap<T, U>{
    fn static_reflect() -> Type {
        Type { name: type_name::<Self>().to_string(), data: TypeData::Map { key: Box::new(T::static_reflect()), values: Box::new(U::static_reflect())}, size: size_of::<Self>(),      align:align_of::<Self>()}
    }
}
impl StaticReflect for String{
    fn static_reflect() -> Type {
        Type { name: type_name::<Self>().to_string(), data:TypeData::String,  size: size_of::<Self>(),      align:align_of::<Self>()}
    }
}
pub struct FieldIter<'a>{
    rf:*const (),
    current:usize, 
    fields:Vec<Field>,
    __phantom_data:PhantomData<&'a()>
}
pub struct FieldIterValue<'a>{
    pub name:String, 
    pub type_name:String,
    pub value:&'a dyn Any,
    pub reflect:&'a dyn Reflect,
}
impl <'a>Iterator for FieldIter<'a>{
    type Item = FieldIterValue<'a>;
    fn next(&mut self) -> Option<Self::Item> {
        if self.current >= self.fields.len(){
            return None;
        }
        unsafe{
            let f = &self.fields[self.current];
            let ptr = (self.rf as *const u8).byte_add(f.offset);
            let (ptr,alias) =TYPE_REGISTERY.as_down_cast(&f.value.name, ptr.as_ref().unwrap()).unwrap();
            self.current += 1;
            Some(FieldIterValue{name:f.name.clone(), type_name:f.value.name.clone(), value:ptr, reflect:alias})
        }
    }
}
pub trait Reflect:std::fmt::Debug+{ 
    fn reflect(&self)->Type;
    fn field_info(&self)->Vec<Field>{
        let t =self.reflect();
        match t.data{
            TypeData::Struct { fields } => {fields},
            _=>{vec![]},
        }
    }
    fn fields(& self)->FieldIter<'_>{
        FieldIter { rf: self as *const Self as *const (), current: 0, fields:self.field_info()  ,__phantom_data:Default::default()}
    }
}
impl Reflect for i8{
    fn reflect(&self)->Type {
        Self::static_reflect()
    }
}
impl Reflect for i16{
    fn reflect(&self)->Type {
        Self::static_reflect()
    }
}
impl Reflect for i32{
    fn reflect(&self)->Type {
        Self::static_reflect()
    }
}
impl Reflect for i64{
    fn reflect(&self)->Type {
        Self::static_reflect()
    }
}
impl Reflect for i128{
    fn reflect(&self)->Type {
        Self::static_reflect()
    }
}
impl Reflect for u8{
    fn reflect(&self)->Type {
        Self::static_reflect()
    }
}
impl Reflect for u16{
    fn reflect(&self)->Type {
        Self::static_reflect()
    }
}
impl Reflect for u32{
    fn reflect(&self)->Type {
        Self::static_reflect()
    }
}
impl Reflect for u64{
    fn reflect(&self)->Type {
        Self::static_reflect()
    }
}
impl Reflect for u128{
    fn reflect(&self)->Type {
        Self::static_reflect()
    }
}
impl Reflect for bool{
    fn reflect(&self)->Type {
        Self::static_reflect()
    }
}
impl Reflect for usize{
    fn reflect(&self)->Type {
        Self::static_reflect()
    }
}
impl Reflect for isize{
    fn reflect(&self)->Type {
        Self::static_reflect()
    }
}
impl Reflect for f32{
    fn reflect(&self)->Type {
        Self::static_reflect()
    }
}
impl Reflect for f64{
    fn reflect(&self)->Type {
        Self::static_reflect()
    }
}
impl Reflect for String{
    fn reflect(&self)->Type{
        Self::static_reflect()
    }
}
impl <T:Reflect+StaticReflect> Reflect for Vec<T>{
    fn reflect(&self)->Type {
        Self::static_reflect()
    }
}
#[derive(Clone)]
pub struct RegisteredType{
    pub name:&'static str,
    pub to_any_ptr:&'static(dyn Send+Sync+ Fn(&u8)->(&dyn Any,&dyn Reflect)),
    pub from_bytes:&'static(dyn Send+Sync+Fn(&u8)->Option<Box<dyn Reflect>>),
}
pub struct TypeRegistery {
    pub base_types:Mutex<Option<Box<[RegisteredType]>>>,
    pub registered_types:Mutex<Vec<RegisteredType>>,
}
impl TypeRegistery{
    pub unsafe fn as_down_cast(&self,name:&str, ptr:*const u8)->Option<(&dyn Any, &dyn Reflect)>{
        let l_lock = self.base_types.lock().unwrap();
        let lock = l_lock.as_ref().unwrap();
        for i  in lock{
            //println!("{}", i.name);
            if i.name == name{
                unsafe{return Some((*i.to_any_ptr)(ptr.as_ref().unwrap()))}
            }
        }
        None
    }
}
#[macro_export]
macro_rules! as_registered {
    ($name:ty) => {
        $crate::utils::RegisteredType{name:std::any::type_name::<$name>(), to_any_ptr:&<$name>::to_any_ptr, from_bytes:&<$name>::to_bytes}
    };
}

pub static TYPE_REGISTERY:TypeRegistery = TypeRegistery{base_types:Mutex::new(None), registered_types:Mutex::new(Vec::new())};
 pub fn register_types(types:&[RegisteredType]){
    let base_types = [
        as_registered!(i8),
        as_registered!(i16),
        as_registered!(i32),
        as_registered!(i64),
        as_registered!(u8),
        as_registered!(u16),
        as_registered!(u32),
        as_registered!(u64),
        as_registered!(String),
        as_registered!(usize), 
        as_registered!(f32),
        as_registered!(f64),
    ];
    let mut base = base_types.as_ref().to_vec();
    for i in types{
        base.push(i.clone());
    }
    *TYPE_REGISTERY.base_types.lock().unwrap() = Some(base.into())
}