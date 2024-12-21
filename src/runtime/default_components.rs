use raylib::{color::Color, ffi::BoundingBox, math::{Transform, Vector3}};

#[derive(Clone, Copy)]
pub struct TransformComp{
    pub location:Transform, 
    pub relative_location:Transform, 
    pub parent_id:Option<usize>,
}

#[derive(Clone, Copy)]
//requires transform
pub struct PhysicsComp{
    pub velocity:Vector3, 
    pub bounds:BoundingBox,
    pub movable:bool,
}

#[derive(Clone, Copy)]
//requires transform
pub struct MeshComp{
    pub mesh:u32,
    pub mat:u32,
}

#[derive(Clone, Copy)]
//requires transform
pub struct LightComp{
    pub color:Color, 
}