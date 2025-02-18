#pragma once 
#include "raylib.h"
#include "raymath.h"
void draw_line(Vector3 start, Vector3 end, Color color);
void draw_sphere(Vector3 center_pos,float radius, Color color);
void draw_text(const char * txt, int x, int y, int height, Color color);
void draw_box(BoundingBox bx, Color color);