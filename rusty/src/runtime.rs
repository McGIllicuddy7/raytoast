
use crate::cvec;
pub use crate::math::*;
pub use crate::c::*;
#[repr(C)]
#[derive(Clone, Copy)]
pub struct Transform {
    pub translation: Vector3,
    pub rotation: Vector4,
    pub scale: Vector4,
}
#[repr(C)]
#[derive(Clone, Copy)]
pub struct TransformComp {
    pub transform: Transform,
    pub parent: Optu32,
    pub children: cvec::CVec<u32>,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct ModelComp {
    pub shader_id: u32,
    pub model_id: u32,
}
#[repr(C)]
#[derive(Clone, Copy)]
pub struct BoundingBox {
    pub min: Vector3,
    pub max: Vector3,
}
#[repr(C)]
#[derive(Clone, Copy)]
pub struct PhysicsComp {
    pub bx: BoundingBox,
    pub velocity: Vector3,
    pub mass: f32,
    pub collided_this_frame: bool,
    pub movable: bool,
    pub can_bounce: bool,
}
