use raylib::{color::Color, ffi::BoundingBox, math::{Transform, Vector3}};

pub struct TransformComp{
    pub location:Transform, 
    pub relative_location:Transform, 
    pub parent_id:Option<usize>,
}

//requires transform
pub struct PhysicsComp{
    pub velocity:Vector3, 
    pub bounds:BoundingBox,
}

//requires transform
pub struct MeshComp{
    pub mesh:usize,
    pub mat:usize,
}
//requires transform
pub struct LightComp{
    pub color:Color, 
}