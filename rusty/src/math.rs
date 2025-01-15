use std::{
    mem::MaybeUninit,
    ops::{Add, AddAssign, Div, DivAssign, Mul, MulAssign, Sub, SubAssign},
};
pub trait Numeric:
    Add<Output = Self>
    + AddAssign
    + Div<Output = Self>
    + DivAssign
    + Mul<Output = Self>
    + MulAssign
    + Sub<Output = Self>
    + SubAssign
    + PartialEq
    + Clone
    + std::fmt::Debug
{
    fn zero() -> Self;
    fn unit() -> Self;
}

impl Numeric for f32 {
    fn unit() -> Self {
        return 1.0;
    }
    fn zero() -> Self {
        return 0.0;
    }
}
impl Numeric for f64 {
    fn unit() -> Self {
        return 1.0;
    }
    fn zero() -> Self {
        return 0.0;
    }
}
impl Numeric for u8 {
    fn unit() -> Self {
        return 1;
    }
    fn zero() -> Self {
        return 0;
    }
}
impl Numeric for u16 {
    fn unit() -> Self {
        return 1;
    }
    fn zero() -> Self {
        return 0;
    }
}
impl Numeric for u32 {
    fn unit() -> Self {
        return 1;
    }
    fn zero() -> Self {
        return 0;
    }
}
impl Numeric for u64 {
    fn unit() -> Self {
        return 1;
    }
    fn zero() -> Self {
        return 0;
    }
}
impl Numeric for u128 {
    fn unit() -> Self {
        return 1;
    }
    fn zero() -> Self {
        return 0;
    }
}
impl Numeric for i8 {
    fn unit() -> Self {
        return 1;
    }
    fn zero() -> Self {
        return 0;
    }
}
impl Numeric for i16 {
    fn unit() -> Self {
        return 1;
    }
    fn zero() -> Self {
        return 0;
    }
}
impl Numeric for i32 {
    fn unit() -> Self {
        return 1;
    }
    fn zero() -> Self {
        return 0;
    }
}
impl Numeric for i64 {
    fn unit() -> Self {
        return 1;
    }
    fn zero() -> Self {
        return 0;
    }
}
impl Numeric for i128 {
    fn unit() -> Self {
        return 1;
    }
    fn zero() -> Self {
        return 0;
    }
}
pub trait Vector:
    Add + AddAssign + Div<f32> + DivAssign<f32> + Mul<f32> + MulAssign<f32> + Sub + SubAssign + Sized
{
    fn dot(lhs: Self, rhs: Self) -> f32;
}
#[repr(C)]
#[derive(Clone, Copy, PartialEq)]
pub struct Vector2 {
    pub x: f32,
    pub y: f32,
}
#[repr(C)]
#[derive(Clone, Copy, PartialEq)]
pub struct Vector3 {
    pub x: f32,
    pub y: f32,
    pub z: f32,
}

#[repr(C)]
#[derive(Clone, Copy, PartialEq)]
pub struct Vector4 {
    pub x: f32,
    pub y: f32,
    pub z: f32,
    pub w: f32,
}
impl Add for Vector2 {
    type Output = Self;
    fn add(self, rhs: Self) -> Self {
        return Self {
            x: self.x + rhs.x,
            y: self.y + rhs.y,
        };
    }
}
impl AddAssign for Vector2 {
    fn add_assign(&mut self, rhs: Self) {
        *self = *self + rhs;
    }
}
impl Sub for Vector2 {
    type Output = Self;
    fn sub(self, rhs: Self) -> Self {
        return Self {
            x: self.x - rhs.x,
            y: self.y - rhs.y,
        };
    }
}
impl SubAssign for Vector2 {
    fn sub_assign(&mut self, rhs: Self) {
        *self = *self - rhs;
    }
}
impl Mul<f32> for Vector2 {
    type Output = Vector2;
    fn mul(self, rhs: f32) -> Self::Output {
        return Self {
            x: self.x * rhs,
            y: self.y * rhs,
        };
    }
}
impl MulAssign<f32> for Vector2 {
    fn mul_assign(&mut self, rhs: f32) {
        *self = *self * rhs;
    }
}
impl Div<f32> for Vector2 {
    type Output = Vector2;
    fn div(self, rhs: f32) -> Self::Output {
        return Self {
            x: self.x / rhs,
            y: self.y / rhs,
        };
    }
}
impl DivAssign<f32> for Vector2 {
    fn div_assign(&mut self, rhs: f32) {
        *self = *self / rhs;
    }
}

impl Add for Vector3 {
    type Output = Self;
    fn add(self, rhs: Self) -> Self {
        return Self {
            x: self.x + rhs.x,
            y: self.y + rhs.y,
            z: self.z + rhs.z,
        };
    }
}
impl AddAssign for Vector3 {
    fn add_assign(&mut self, rhs: Self) {
        *self = *self + rhs;
    }
}
impl Sub for Vector3 {
    type Output = Self;
    fn sub(self, rhs: Self) -> Self {
        return Self {
            x: self.x - rhs.x,
            y: self.y - rhs.y,
            z: self.z - rhs.z,
        };
    }
}
impl SubAssign for Vector3 {
    fn sub_assign(&mut self, rhs: Self) {
        *self = *self - rhs;
    }
}
impl Mul<f32> for Vector3 {
    type Output = Vector3;
    fn mul(self, rhs: f32) -> Self::Output {
        return Self {
            x: self.x * rhs,
            y: self.y * rhs,
            z: self.z * rhs,
        };
    }
}
impl MulAssign<f32> for Vector3 {
    fn mul_assign(&mut self, rhs: f32) {
        *self = *self * rhs;
    }
}
impl Div<f32> for Vector3 {
    type Output = Vector3;
    fn div(self, rhs: f32) -> Self::Output {
        return Self {
            x: self.x / rhs,
            y: self.y / rhs,
            z: self.z / rhs,
        };
    }
}
impl DivAssign<f32> for Vector3 {
    fn div_assign(&mut self, rhs: f32) {
        *self = *self / rhs;
    }
}

impl Add for Vector4 {
    type Output = Self;
    fn add(self, rhs: Self) -> Self {
        return Self {
            x: self.x + rhs.x,
            y: self.y + rhs.y,
            z: self.z + rhs.z,
            w: self.w + rhs.w,
        };
    }
}
impl AddAssign for Vector4 {
    fn add_assign(&mut self, rhs: Self) {
        *self = *self + rhs;
    }
}
impl Sub for Vector4 {
    type Output = Self;
    fn sub(self, rhs: Self) -> Self {
        return Self {
            x: self.x - rhs.x,
            y: self.y - rhs.y,
            z: self.z - rhs.z,
            w: self.w - rhs.w,
        };
    }
}
impl SubAssign for Vector4 {
    fn sub_assign(&mut self, rhs: Self) {
        *self = *self - rhs;
    }
}
impl Mul<f32> for Vector4 {
    type Output = Vector4;
    fn mul(self, rhs: f32) -> Self::Output {
        return Self {
            x: self.x * rhs,
            y: self.y * rhs,
            z: self.z * rhs,
            w: self.w * rhs,
        };
    }
}
impl MulAssign<f32> for Vector4 {
    fn mul_assign(&mut self, rhs: f32) {
        *self = *self * rhs;
    }
}
impl Div<f32> for Vector4 {
    type Output = Vector4;
    fn div(self, rhs: f32) -> Self::Output {
        return Self {
            x: self.x / rhs,
            y: self.y / rhs,
            z: self.z / rhs,
            w: self.w / rhs,
        };
    }
}
impl DivAssign<f32> for Vector4 {
    fn div_assign(&mut self, rhs: f32) {
        *self = *self / rhs;
    }
}

impl Vector for Vector2 {
    fn dot(lhs: Self, rhs: Self) -> f32 {
        lhs.x * rhs.x + lhs.y * rhs.y
    }
}

impl Vector for Vector3 {
    fn dot(lhs: Self, rhs: Self) -> f32 {
        lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z
    }
}
impl Vector for Vector4 {
    fn dot(lhs: Self, rhs: Self) -> f32 {
        lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w
    }
}
pub fn dot<T: Vector>(x: T, y: T) -> f32 {
    return T::dot(x, y);
}
pub fn cross(a: Vector3, b: Vector3) -> Vector3 {
    let s1: f32 = a.y * b.z - a.z * b.y;
    let s2: f32 = a.z * b.x - a.x * b.z;
    let s3: f32 = a.x * b.y - a.y * b.x;
    Vector3 {
        x: s1,
        y: s2,
        z: s3,
    }
}
#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Matrix<const HEIGHT: usize, const WIDTH: usize = HEIGHT, T: Numeric = f32> {
    pub values: [[T; WIDTH]; HEIGHT],
}
impl<const HEIGHT: usize, const WIDTH: usize, T: Numeric> Add for Matrix<HEIGHT, WIDTH, T> {
    type Output = Self;

    fn add(self, rhs: Self) -> Self::Output {
        let mut out = self;
        for y in 0..HEIGHT {
            for x in 0..WIDTH {
                out.values[y][x] += rhs.values[y][x].clone();
            }
        }
        out
    }
}
impl<const HEIGHT: usize, const WIDTH: usize, T: Numeric> AddAssign for Matrix<HEIGHT, WIDTH, T> {
    fn add_assign(&mut self, rhs: Self) {
        *self = self.clone() + rhs.clone();
    }
}
impl<const HEIGHT: usize, const WIDTH: usize, T: Numeric> Sub for Matrix<HEIGHT, WIDTH, T> {
    type Output = Self;
    fn sub(self, rhs: Self) -> Self::Output {
        let mut out = self;
        for y in 0..HEIGHT {
            for x in 0..WIDTH {
                out.values[y][x] -= rhs.values[y][x].clone();
            }
        }
        out
    }
}
impl<const HEIGHT: usize, const WIDTH: usize, T: Numeric> SubAssign for Matrix<HEIGHT, WIDTH, T> {
    fn sub_assign(&mut self, rhs: Self) {
        *self = self.clone() - rhs;
    }
}

impl<const HEIGHT: usize, const WIDTH: usize, T: Numeric> Mul<T> for Matrix<HEIGHT, WIDTH, T> {
    type Output = Self;

    fn mul(self, rhs: T) -> Self::Output {
        let mut out = self;
        for y in 0..HEIGHT {
            for x in 0..WIDTH {
                out.values[y][x] *= rhs.clone();
            }
        }
        return out;
    }
}
/*matrix5x5i matrix5x5iMlt(matrix5x5i a, matrix5x5i b){
    matrix5x5i out = {};
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            out.data[i][j] = 0;
            for (int k = 0; k < 5; k++) {
                out.data[i][j] += a.data[i][k] * b.data[k][j];
            }
        }
    }
    return out;
}*/
//A width == B height
impl<const HEIGHT: usize, const WIDTH: usize, T: Numeric, const OTHER_WIDTH: usize, const OTHER_HEIGHT:usize>
    Mul<Matrix<OTHER_HEIGHT, OTHER_WIDTH, T>> for Matrix<HEIGHT, WIDTH, T>
{
    type Output = Matrix<HEIGHT, HEIGHT, T>;

    fn mul(self, rhs: Matrix<OTHER_HEIGHT, OTHER_WIDTH, T>) -> Self::Output {
        assert_eq!(WIDTH, OTHER_HEIGHT);
        let mut out: Matrix<HEIGHT, HEIGHT, T> = Matrix::zero();
        for i in 0..HEIGHT {
            for j in 0..OTHER_WIDTH {
                for k in 0..HEIGHT{
                    out.values[i][j] += self.values[i][k].clone() * rhs.values[k][j].clone();
                }
            }
        }
        out
    }
}

impl<const HEIGHT: usize, const WIDTH: usize, T: Numeric, U: Into<T> + Clone>
    From<[[U; WIDTH]; HEIGHT]> for Matrix<HEIGHT, WIDTH, T>
{
    fn from(value: [[U; WIDTH]; HEIGHT]) -> Self {
        let mut out: Matrix<HEIGHT, WIDTH, T> = unsafe { MaybeUninit::uninit().assume_init() };
        for i in 0..HEIGHT {
            for j in 0..WIDTH {
                out.values[i][j] = value[i][j].clone().into();
            }
        }
        out
    }
}

impl<const HEIGHT: usize, const WIDTH: usize, T: Numeric> Matrix<HEIGHT, WIDTH, T> {
    pub fn zero() -> Self {
        let mut tmp: Matrix<HEIGHT, WIDTH, T> = unsafe { MaybeUninit::uninit().assume_init() };
        for i in 0..HEIGHT {
            for j in 0..WIDTH {
                tmp.values[i][j] = T::zero();
            }
        }
        tmp
    }
    pub fn identity() -> Self {
        let mut out = Self::zero();
        for i in 0..HEIGHT {
            for j in 0..WIDTH {
                if i == j {
                    out.values[i][j] = T::unit();
                }
            }
        }
        out
    }
}

impl From<Vector2> for Matrix<1, 2, f32>{
    fn from(value: Vector2) -> Self {
        let out:Matrix<1,2,f32> = [[value.x, value.y]].into();
        out
    }   
}
impl From<Vector3> for Matrix<1, 3, f32>{
    fn from(value: Vector3) -> Self {
        let out:Matrix<1,3,f32> = [[value.x, value.y, value.z]].into();
        out
    }   
}
impl From<Vector4> for Matrix<1, 4, f32>{
    fn from(value: Vector4) -> Self {
        let out:Matrix<1,4,f32> = [[value.x, value.y, value.z, value.w]].into();
        out
    }   
}
impl Into<Vector2> for Matrix<1,2, f32>{
    fn into(self) -> Vector2 {
        return Vector2 { x: self.values[0][0], y: self.values[0][1] }
    }
}
impl Into<Vector3> for Matrix<1,3, f32>{
    fn into(self) -> Vector3 {
        return Vector3 { x: self.values[0][0], y: self.values[0][1],z: self.values[0][2] }
    }
}
impl Into<Vector4> for Matrix<1, 4, f32>{
    fn into(self) -> Vector4 {
        return Vector4 { x: self.values[0][0], y: self.values[0][1],z: self.values[0][2] , w:self.values[0][3]} 
    }
}
impl <const HEIGHT:usize,const WIDTH:usize, T:Numeric >std::ops::Index<usize> for Matrix<HEIGHT, WIDTH, T>{
    type Output = [T];
    fn index(&self, index: usize) -> &Self::Output {
        &self.values[index]
    }
}
impl <const HEIGHT:usize,const WIDTH:usize, T:Numeric >std::ops::IndexMut<usize> for Matrix<HEIGHT, WIDTH, T>{
    fn index_mut(&mut self, index: usize) ->  &mut Self::Output {
        &mut self.values[index]
    }
} 



#[test]
fn test1() {
    let _a: Matrix<3> = Matrix::identity() * 2.0;
    let b: Matrix<3> = [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0], [7.0, 8.0, 9.0]].into();
    println!("{:#?}", b * b);
}

