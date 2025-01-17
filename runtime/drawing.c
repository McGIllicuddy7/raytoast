#include "drawing.h"
#include "runtime.h"
make_lambda_capture(fn_void, void, draw_line_lambda, {DrawLine3D(captures->start_pos, captures->end_pos, captures->color);},{Vector3 start_pos; Vector3 end_pos; Color color;});

make_lambda_capture(fn_void, void, draw_sphere_lambda, {DrawSphere(captures->center, captures->radius, captures->color); printf("called draw\n");},{Vector3 center; float radius; Color color;});
void draw_line(Vector3 start, Vector3 end, Color color){
    fn_void func = tmp_lambda(draw_line_lambda, {start, end, color});
    draw_call(func);
}
void draw_sphere(Vector3 center_pos,float radius, Color color ){
    fn_void func = tmp_lambda(draw_sphere_lambda, {center_pos, radius, color});
    draw_call(func); 
}