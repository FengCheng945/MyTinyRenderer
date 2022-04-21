#include <vector>
#include<iostream>
#include <cmath>
#include <limits>
#include "rasterizer.h"
#include "Vex.h"
#include "shader.h"
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255); //(R,G,B,A)
const TGAColor green = TGAColor(0, 255, 0, 255);
Model *model = NULL;
const int width  = 800;
const int height = 800;
Vector3f light_dir(3, 3, 2); // define light_dir
Vector3f eye_pos(1, 1, 3);

int main(int argc, char** argv)
{
    Rasterizer rst;
    if (2 == argc)
    {
        model = new Model(argv[1]);
    }
    else
    {
        model = new Model("obj/african_head.obj");
    }
    
    TGAImage depth(width, height, TGAImage::RGB);
    DepthShader depthshader;
    depthshader.set_view_matrix(light_dir);
    depthshader.set_projection_matrix(45, 1, 0.1, 50);
    depthshader.set_viewport_matrix(width, height);
    for (int i = 0; i < model->nfaces(); i++)
    {
        Vex vex;
        for (int j = 0; j < 3; j++)
        {
            Vector4f v = depthshader.vertex(i, j, vex);
            vex.screen_coords[j] = depthshader.get_viewport(v, width, height);
        }
        rst.triangle(vex, depth, model, depthshader);

    }
    depth.flip_vertically(); // to place the origin in the bottom left corner of the image
    depth.write_tga_file("depth.tga");

    light_dir.normalize();//注意，这里light_dir实际上是(light_dir-{0,0,0})，仅代表光照方向，所以要化成单位向量。
    TGAImage image(width, height, TGAImage::RGB);
    PhongShader shader;
    shader.shadowbuffer = depth.get_zbuffer();

    /*shader.set_model_matrix('Z', 45);*/
    shader.set_view_matrix(eye_pos);
    shader.set_projection_matrix(45, 1, 0.1, 50);
    shader.set_viewport_matrix(width, height);
    shader.set_shadowMatrix(depthshader.get_MS());
    for (int i = 0; i < model->nfaces(); i++)
    {
        Vex vex;
        for (int j = 0; j < 3; j++)
        {
            Vector4f v = shader.vertex(i, j, vex);
            vex.screen_coords[j] = shader.get_viewport(v, width, height);
        }
        vex.set_TBN();
        Vector3f n = (vex.world_coords[1] - vex.world_coords[0]).cross(vex.world_coords[2] - vex.world_coords[0]);
        n.normalize();
        float backculling = n * light_dir;
        if (backculling>0)
        {
            rst.triangle(vex, image, model, shader);
        }
    }
   
    image.flip_vertically();
    image.write_tga_file("framebuffer1.tga");
    delete model;
    return 0;
}