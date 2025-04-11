use std::marker::PhantomData;
use std::sync::{RwLock, RwLockReadGuard};
pub struct Context<T: Send + Sync> {
    value: RwLock<Option<T>>,
}
pub struct ContextRef<'a, T: Send + Sync> {
    v: RwLockReadGuard<'a, T>,
}
impl<T: Send + Sync> std::default::Default for Context<T> {
    fn default() -> Self {
        Self::new()
    }
}
impl<T: Send + Sync> Context<T> {
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
    pub fn get_ref(&self) -> Option<ContextRef<T>> {
        let mut t = self.value.read().unwrap();
        if let Some(t) = t.as_ref().unwrap() {}
    }
}
