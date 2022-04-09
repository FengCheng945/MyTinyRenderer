#include <vector>
#include<iostream>
#include <cmath>
#include <limits>
#include "rasterizer.h"
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255); //(R,G,B,A)
const TGAColor green = TGAColor(0, 255, 0, 255);
Model *model = NULL;
const int width  = 800;
const int height = 800;

int main(int argc, char** argv) {
    TGAImage image(width, height, TGAImage::RGB);
    Rasterizer rst;
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }
    Vector3f light_dir(0, 0, 1); // define light_dir
    Vector3f eye_pos(0, 0, 3);
    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        Vector3f screen_coords[3];
        Vector3f world_coords[3];
        Vector2i texture[3];
        for (int j = 0; j < 3; j++)
        {
            Vector3f vex = model->vert(face[j]);
            /*screen_coords[j] = Vector3f((vex.x + 1.) * width / 2., (vex.y + 1.) * height / 2., vex.z);*/
            Matrix4f M_model = rst.get_model_matrix('Z', 45);
            Matrix4f M_view = rst.get_view_matrix(eye_pos);
            Matrix4f M_pro = rst.get_projection_matrix(45, 1, 0.1, 50);
            Vector4f v(vex.x, vex.y, vex.z, 1.f);
            v =  M_pro * M_view * M_model * v;
            v = v/v.w;
            screen_coords[j] = rst.get_viewport(v, width, height);
            world_coords[j] = vex;
            texture[j] = model->uv(i,j);
        }
        Vector3f n = (world_coords[1] - world_coords[0]).cross(world_coords[2] - world_coords[0]); 
        n.normalize();
        float intensity = n * light_dir;
        if (intensity > 0) 
        {
            Vector3f vertex[3] = { screen_coords[0], screen_coords[1], screen_coords[2] };
            rst.triangle(vertex, texture, image, model, intensity);
        }
    }
    image.flip_vertically(); 
    image.write_tga_file("framebuffer1.tga");
    delete model;
    return 0;
}