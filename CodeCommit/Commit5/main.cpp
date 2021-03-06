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
Vector3f light_dir(1, 1, 1); // define light_dir
Vector3f eye_pos(1, 1, 3);


int main(int argc, char** argv)
{
    TGAImage image(width, height, TGAImage::RGB);
    Rasterizer rst;
    if (2 == argc)
    {
        model = new Model(argv[1]);
    }
    else
    {
        model = new Model("obj/african_head.obj");
    }
    GouraudShader shader;
    light_dir.normalize();//注意，这里light_dir实际上是(light_dir-{0,0,0})，仅代表光照方向，所以要化成单位向量。
    for (int i = 0; i < model->nfaces(); i++)
    {
        Vex vex;
        for (int j = 0; j < 3; j++)
        {
            Vector4f v = shader.vertex(i, j, vex);
            vex.screen_coords[j] = shader.get_viewport(v, width, height);
            vex.intensity[j] = std::max(0.f, vex.normal_coords[j] * light_dir);
        }
        Vector3f n = (vex.world_coords[1] - vex.world_coords[0]).cross(vex.world_coords[2] - vex.world_coords[0]);
        n.normalize();
        float backculling = n * light_dir;
        if (backculling > 0)
        {
            rst.triangle(vex, image, model);
        }
    }
    image.flip_vertically();
    image.write_tga_file("framebuffer1.tga");
    delete model;
    return 0;
}